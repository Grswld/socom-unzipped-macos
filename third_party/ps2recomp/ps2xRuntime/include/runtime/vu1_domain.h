#pragma once
// macOS fork, the VU1 worker (docs/superpowers/specs/2026-10-04-vu1-worker-design.md 3.1-3.4; plan Part 2 Task 2).
// While PS2X_VU1_THREAD=1 a worker thread on a performance core owns VIF1, VU1, the GIF arbiter and the GS front end.
// The game thread posts the work it used to do inline (vu1work::WorkItem), drains before the few reads that need the
// worker's state (research/84: about four a frame), and bounds how far it runs ahead at every frame boundary
// (sceGsSyncV). Every wait is subtracted from guest time as VU1's own time was. Off, nothing here runs.
#include "runtime/vu1_overlap_stats.h"
#include "runtime/vu1_spsc_ring.h"
#include "runtime/vu1_work_queue.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace vu1domain
{
    struct Config
    {
        bool enabled = false;          // PS2X_VU1_THREAD
        unsigned queueFrames = 1;      // PS2X_VU1_QUEUE_FRAMES: game frames the worker may still hold at a boundary (0-4)
        const char *recordPath = nullptr;   // PS2X_VU1_QUEUE_RECORD: every posted item, in order (Task 4)
        bool stats = false;            // PS2X_VU1_WORKER_STATS
        // PS2X_VU1_FRAME_LOG (needs the stats): a CSV, one "F" row per game frame (interval, backpressure wait,
        // readbacks, cumulative VU1 programs while PS2X_VU_STATS is on) and one "L" row per input-latency sample.
        std::string frameLogPath;
        bool inlineApply = false;      // PS2X_VU1_THREAD_INLINE: the same items, applied at post on the game thread (a bisect)
    };
    Config configFrom(const char *(*knob)(const char *));

    using ApplyFn = void (*)(const vu1work::ItemView &item, void *ctx);   // the payload is valid during the call

    void start(const Config &cfg, ApplyFn apply, void *ctx);   // spawns the worker when enabled; idempotent per stop
    void stop();                                               // drains, closes, joins; idempotent

    namespace detail
    {
        std::atomic<bool> &enabledFlag();
        bool &onWorkerFlag();   // thread_local
    }
    inline bool enabled() { return detail::enabledFlag().load(std::memory_order_acquire); }
    inline bool onWorker() { return detail::onWorkerFlag(); }
    inline bool shouldPost() { return enabled() && !onWorker(); }

    // Copy one item into the ring (payload once, into the preallocated arena); published every 64 items, at once when the
    // worker sleeps, and at flush(). The WorkItem form is for sites with no payload and for tests.
    void post(vu1work::Kind kind, uint32_t a, uint64_t b, const uint8_t *data, uint32_t size);
    void post(vu1work::WorkItem &&item);
    void flush();   // publish everything posted

    // Items posted inside a batch scope are published every 64 and when the scope ends; outside one, at once. A
    // transfer batch (processPendingTransfers) is one scope: the worker sees a kick's items together, and nothing posted
    // elsewhere (an STC, a register write) can wait unpublished behind a worker that has gone to sleep.
    struct BatchScope
    {
        BatchScope();
        ~BatchScope();
        BatchScope(const BatchScope &) = delete;
        BatchScope &operator=(const BatchScope &) = delete;
    };
    void drain(vu1overlap::SyncKind why);   // waits for every posted item; counted by kind; excluded from guest time
    void frameBoundary();                   // posts a FrameMark, then waits while more than queueFrames are in flight

    uint64_t drainCount(vu1overlap::SyncKind why);

    // Game-frame times (host ms between sceGsSyncV calls): the spec's p95/p99 gate. Nearest-rank percentiles.
    struct FrameTimes
    {
        std::vector<double> ms;
        void add(double v) { ms.push_back(v); }
        void clear() { ms.clear(); }
        double percentile(double p) const
        {
            if (ms.empty())
                return 0.0;
            std::vector<double> sorted(ms);
            std::sort(sorted.begin(), sorted.end());
            const size_t rank = static_cast<size_t>(std::ceil(p / 100.0 * double(sorted.size())));
            return sorted[std::min(sorted.size(), std::max<size_t>(rank, 1u)) - 1u];
        }
    };
    // Part 3 Task 3.3: a game frame over 33.4 ms (the locked-30 budget) is tagged by what it waited on: a backpressure
    // wait on the worker over 0.5 ms (Worker), a GL render-target / VRAM readback in it (Readback), both, or neither (Game).
    enum class SlowTag : uint8_t { Fast, Worker, Readback, Both, Game };
    inline SlowTag classifySlow(double intervalMs, int64_t backpressureNs, uint64_t readbacks)
    {
        if (intervalMs <= 33.4)
            return SlowTag::Fast;
        const bool worker = backpressureNs > 500'000, readback = readbacks > 0;
        return worker && readback ? SlowTag::Both : worker ? SlowTag::Worker : readback ? SlowTag::Readback : SlowTag::Game;
    }
    std::atomic<uint64_t> &gsReadbackCount();   // the GL backend counts its glReadPixels readbacks here

    // Part 3 Task 3.2: input-to-display latency. The first pad read of each game frame is kept by frame index; a frame
    // completes at sceGsSyncV (worker off) or when the worker applies its frame mark (worker on); at every host present
    // the newest completed frame not yet presented gives one sample: now - its first pad read.
    void notePadRead();                     // game thread (scePad2Read)
    // sceGsSyncV's exit, on the VBlank grid: an "E,frame,t_ms,vsync_tick,caller_ra,game_dt_ms,t0_entry,t0_exit" row. A game
    // frame is ONE OR TWO sceGsSyncV calls (SOCOM II's zVid_Swap: a limiter wait, then the frame's own), so frame times
    // come from the exits of the caller's final call, and VBlanks from their ticks. game_dt_ms: what the game measured
    // for its frame on T0 (the caller passes it; NaN when unknown).
    // t0Entry/t0Exit: EE timer 0's COUNT at the call and at the resume (the game paces on T0 % 525).
    void noteSyncVExit(uint64_t vsyncTick, uint32_t callerRa, float gameDtMs, uint32_t t0Entry = 0, uint32_t t0Exit = 0);
    bool frameLogOn();
    void notePresent();                     // main thread, after EndDrawing
    std::vector<double> latencySamples();   // ms, since resetFrameStats

    void resetFrameStats();
    size_t frameStatsCount();
    double frameLatencyMaxMs();   // FrameMark post to applied, since the last reset

    // The VU1 D/T stop bits (vu0_vpu_stat 0x0600) of the last program the worker ran: never written into the live EE
    // context from the worker; the runtime's drain hook merges them on the game thread after every drain and frame
    // boundary.
    void noteVpuStop(uint32_t bits);
    uint32_t pendingVpuStop();
    void setStatsExtra(void (*fn)());   // a diagnostic line printed after the stats line
    using DrainHookFn = void (*)(void *ctx);
    void setDrainHook(DrainHookFn fn, void *ctx);

    // The EE's vu0_fbrst (VU1 D/T enables) at a VIF1 kick, captured into the Vif1Data item so the worker never reads
    // the live EE context. The runtime installs the probe; without one it is 0.
    using FbrstFn = uint64_t (*)(void *ctx);
    void setFbrstProbe(FbrstFn fn, void *ctx);
    uint64_t eeFbrst();
}
