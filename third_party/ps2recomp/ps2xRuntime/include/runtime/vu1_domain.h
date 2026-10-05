#pragma once
// macOS fork, the VU1 worker (docs/superpowers/specs/2026-10-04-vu1-worker-design.md 3.1-3.4; plan Part 2 Task 2).
// While PS2X_VU1_THREAD=1 a worker thread on a performance core owns VIF1, VU1, the GIF arbiter and the GS front end.
// The game thread posts the work it used to do inline (vu1work::WorkItem), drains before the few reads that need the
// worker's state (research/84: about four a frame), and bounds how far it runs ahead at every frame boundary
// (sceGsSyncV). Every wait is subtracted from guest time as VU1's own time was. Off, nothing here runs.
#include "runtime/vu1_overlap_stats.h"
#include "runtime/vu1_work_queue.h"

#include <atomic>
#include <cstdint>

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
}
