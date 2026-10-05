#pragma once
// macOS fork, the VU1 worker (docs/superpowers/specs/2026-10-04-vu1-worker-design.md 3.1-3.4; plan Part 2 Task 2).
// While PS2X_VU1_THREAD=1 a worker thread on a performance core owns VIF1, VU1, the GIF arbiter and the GS front end.
// The game thread posts the work it used to do inline (vu1work::WorkItem), drains before the few reads that need the
// worker's state (research/84: about four a frame), and bounds how far it runs ahead at every frame boundary
// (sceGsSyncV). Every wait is subtracted from guest time as VU1's own time was. Off, nothing here runs.
#include "runtime/vu1_overlap_stats.h"
#include "runtime/vu1_work_queue.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vu1domain
{
    struct Config
    {
        bool enabled = false;          // PS2X_VU1_THREAD
        unsigned queueFrames = 1;      // PS2X_VU1_QUEUE_FRAMES: game frames the worker may still hold at a boundary (0-4)
        const char *recordPath = nullptr;   // PS2X_VU1_QUEUE_RECORD: every posted item, in order (Task 4)
        bool stats = false;            // PS2X_VU1_WORKER_STATS
    };
    Config configFrom(const char *(*knob)(const char *));

    using ApplyFn = void (*)(const vu1work::WorkItem &item, void *ctx);

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

    void post(vu1work::WorkItem &&item);
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
    void resetFrameStats();
    size_t frameStatsCount();
    double frameLatencyMaxMs();   // FrameMark post to applied, since the last reset

    // The VU1 D/T stop bits (vu0_vpu_stat 0x0600) of the last program the worker ran: never written into the live EE
    // context from the worker; the runtime's drain hook merges them on the game thread after every drain and frame
    // boundary.
    void noteVpuStop(uint32_t bits);
    uint32_t pendingVpuStop();
    using DrainHookFn = void (*)(void *ctx);
    void setDrainHook(DrainHookFn fn, void *ctx);

    // The EE's vu0_fbrst (VU1 D/T enables) at a VIF1 kick, captured into the Vif1Data item so the worker never reads
    // the live EE context. The runtime installs the probe; without one it is 0.
    using FbrstFn = uint64_t (*)(void *ctx);
    void setFbrstProbe(FbrstFn fn, void *ctx);
    uint64_t eeFbrst();
}
