// macOS fork, the VU1 worker: the domain (runtime/vu1_domain.h).
#include "runtime/vu1_domain.h"

#include "runtime/host_thread_qos.h"
#include "runtime/ps2_guest_clock.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace vu1domain
{
    namespace detail
    {
        std::atomic<bool> &enabledFlag()
        {
            static std::atomic<bool> s_enabled{false};
            return s_enabled;
        }
        bool &onWorkerFlag()
        {
            thread_local bool t_onWorker = false;
            return t_onWorker;
        }
    }

    namespace
    {
        struct State
        {
            Config cfg;
            ApplyFn apply = nullptr;
            void *ctx = nullptr;
            std::unique_ptr<vu1work::WorkQueue> queue;
            std::thread worker;
            std::FILE *record = nullptr;
            std::vector<uint8_t> recordBuf;
            std::atomic<uint64_t> drains[int(vu1overlap::SyncKind::Count)] = {};
        };
        State &state()
        {
            static State s;
            return s;
        }

        int64_t nowNs()
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                       std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        // A wait on the worker is not guest time (spec 3.4): VU1's own time was subtracted when it ran inline.
        void excludeFromGuestClock(int64_t ns)
        {
            if (ns > 0)
                ps2GuestClockExcludedNs().fetch_add(ns, std::memory_order_relaxed);
        }

        void workerLoop()
        {
            State &s = state();
            detail::onWorkerFlag() = true;
            hostThreadSetInteractive();
            vu1work::WorkItem item;
            uint64_t seq = 0;
            while (s.queue->pop(item, seq))
            {
                s.apply(item, s.ctx);
                s.queue->markDone(seq, item.kind == vu1work::Kind::FrameMark);
            }
        }
    }

    Config configFrom(const char *(*knob)(const char *))
    {
        Config c;
        if (const char *v = knob("PS2X_VU1_THREAD"))
            c.enabled = std::strcmp(v, "0") != 0 && std::strcmp(v, "false") != 0 && std::strcmp(v, "off") != 0 && *v;
        if (const char *v = knob("PS2X_VU1_QUEUE_FRAMES"))
        {
            char *end = nullptr;
            const long n = std::strtol(v, &end, 10);
            if (end != v && *end == '\0' && n >= 0 && n <= 4)
                c.queueFrames = static_cast<unsigned>(n);
        }
        c.recordPath = knob("PS2X_VU1_QUEUE_RECORD");
        c.stats = knob("PS2X_VU1_WORKER_STATS") != nullptr;
        return c;
    }

    void start(const Config &cfg, ApplyFn apply, void *ctx)
    {
        State &s = state();
        if (enabled() || !cfg.enabled || !apply)
            return;
        s.cfg = cfg;
        s.apply = apply;
        s.ctx = ctx;
        s.queue = std::make_unique<vu1work::WorkQueue>();
        if (cfg.recordPath && *cfg.recordPath)
            s.record = std::fopen(cfg.recordPath, "wb");
        s.worker = std::thread(workerLoop);
        detail::enabledFlag().store(true, std::memory_order_release);
        std::fprintf(stderr, "[vu1-worker] on: queue frames %u%s\n", cfg.queueFrames, s.record ? ", recording" : "");
    }

    void stop()
    {
        State &s = state();
        if (!enabled())
            return;
        s.queue->waitDone(s.queue->lastPushed());
        detail::enabledFlag().store(false, std::memory_order_release);
        s.queue->close();
        if (s.worker.joinable())
            s.worker.join();
        if (s.record)
        {
            if (!s.recordBuf.empty())
                std::fwrite(s.recordBuf.data(), 1, s.recordBuf.size(), s.record);
            std::fclose(s.record);
            s.record = nullptr;
            s.recordBuf.clear();
        }
        s.queue.reset();
    }

    void post(vu1work::WorkItem &&item)
    {
        State &s = state();
        if (s.record)
        {
            vu1work::serialize(item, s.recordBuf);
            if (s.recordBuf.size() >= (8u << 20))
            {
                std::fwrite(s.recordBuf.data(), 1, s.recordBuf.size(), s.record);
                s.recordBuf.clear();
            }
        }
        s.queue->push(std::move(item));
    }

    void drain(vu1overlap::SyncKind why)
    {
        State &s = state();
        const int64_t t0 = nowNs();
        s.queue->waitDone(s.queue->lastPushed());
        excludeFromGuestClock(nowNs() - t0);
        s.drains[int(why)].fetch_add(1, std::memory_order_relaxed);
    }

    void frameBoundary()
    {
        if (!shouldPost())
            return;
        State &s = state();
        vu1work::WorkItem mark;
        mark.kind = vu1work::Kind::FrameMark;
        post(std::move(mark));
        const int64_t t0 = nowNs();
        s.queue->waitFramesAhead(s.cfg.queueFrames);
        excludeFromGuestClock(nowNs() - t0);
    }

    uint64_t drainCount(vu1overlap::SyncKind why) { return state().drains[int(why)].load(std::memory_order_relaxed); }

    namespace
    {
        FbrstFn g_fbrstFn = nullptr;
        void *g_fbrstCtx = nullptr;
    }
    void setFbrstProbe(FbrstFn fn, void *ctx)
    {
        g_fbrstFn = fn;
        g_fbrstCtx = ctx;
    }
    uint64_t eeFbrst() { return g_fbrstFn ? g_fbrstFn(g_fbrstCtx) : 0u; }
}
