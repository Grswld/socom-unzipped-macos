// macOS fork, the VU1 worker: the domain (runtime/vu1_domain.h).
#include "runtime/vu1_domain.h"

#include "runtime/host_thread_qos.h"
#include "runtime/ps2_guest_clock.h"
#include "ps2x/knobs.h"

#include <algorithm>
#include <cfenv>
#include <chrono>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
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
            std::unique_ptr<vu1work::SpscRing> queue;
            std::thread worker;
            std::FILE *record = nullptr;
            std::vector<uint8_t> recordBuf;
            std::atomic<uint64_t> drains[int(vu1overlap::SyncKind::Count)] = {};
            // Stats (game thread, except the two atomics the worker writes).
            FrameTimes frames;
            int64_t lastBoundaryNs = 0, windowStartNs = 0;
            uint64_t items = 0, windowDrains[int(vu1overlap::SyncKind::Count)] = {};
            int64_t drainWaitNs = 0, backpressureWaitNs = 0;
            unsigned aheadMax = 0;
            std::atomic<int64_t> latencyMaxNs{0}, latencyTotalNs{0};
            std::atomic<uint64_t> latencyCount{0};
            std::atomic<uint32_t> vpuStop{0};
            std::atomic<uint64_t> dtStops{0};
            void (*statsExtra)() = nullptr;
            int batchDepth = 0;   // game thread only
            // Task 3.2 / 3.3
            uint64_t frameIndex = 0;                         // game thread: the frame being built
            std::atomic<int64_t> padTime[64] = {};           // first pad read of frame i, at [i % 64]
            uint64_t padFrame = UINT64_MAX;                  // game thread: the frame padTime was set for
            std::atomic<uint64_t> completedFrame{UINT64_MAX};
            uint64_t lastPresented = UINT64_MAX;             // main thread
            std::mutex latencyMutex;
            std::vector<double> latency;
            int64_t prevBoundaryBpNs = 0;                    // the backpressure wait made at the previous boundary
            uint64_t readbacksAtBoundary = 0;
            unsigned slow[5] = {};
            unsigned aheadAfterMax = 0;
            double withReadbackMs = 0, withoutReadbackMs = 0;   // the readback tag's evidence: mean frame time each way
            unsigned withReadback = 0, withoutReadback = 0;
            DrainHookFn drainHook = nullptr;
            void *drainHookCtx = nullptr;
        };
        State &state()
        {
            static State s;
            return s;
        }

        // The game thread's FPU rounding (ps2_runtime.cpp: toward zero, PCSX2's "Chop", unless PS2X_EE_ROUND=nearest),
        // applied to the worker: the GS front end's float math ran under it inline and must run under it here.
        void setGuestRounding()
        {
            const char *round = ps2x::knob("PS2X_EE_ROUND");
            if (!round || std::strcmp(round, "nearest") != 0)
                std::fesetround(FE_TOWARDZERO);
        }

        int64_t nowNs()
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                       std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        double latencyPercentile(double p);

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
            setGuestRounding();   // the GS front end and VU1 round as they did on the game thread
            vu1work::ItemView item;
            uint64_t seq = 0;
            while (s.queue->pop(item, seq))
            {
                s.apply(item, s.ctx);
                if (item.kind == vu1work::Kind::FrameMark)
                    s.completedFrame.store(item.a, std::memory_order_release);
                if (item.kind == vu1work::Kind::FrameMark && item.b != 0)
                {
                    const int64_t lat = nowNs() - static_cast<int64_t>(item.b);
                    s.latencyTotalNs.fetch_add(lat, std::memory_order_relaxed);
                    s.latencyCount.fetch_add(1, std::memory_order_relaxed);
                    int64_t prev = s.latencyMaxNs.load(std::memory_order_relaxed);
                    while (lat > prev && !s.latencyMaxNs.compare_exchange_weak(prev, lat, std::memory_order_relaxed))
                    {
                    }
                }
                s.queue->release(seq, item.kind == vu1work::Kind::FrameMark);
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
        if (const char *v = knob("PS2X_VU1_THREAD_INLINE"))
            c.inlineApply = std::strcmp(v, "0") != 0 && *v;
        return c;
    }

    void start(const Config &cfg, ApplyFn apply, void *ctx)
    {
        State &s = state();
        if (enabled())
            return;
        s.cfg = cfg;   // kept with the worker off too: the frame stats are the A/B baseline
        if (!cfg.enabled || !apply)
            return;
        s.apply = apply;
        s.ctx = ctx;
        s.queue = std::make_unique<vu1work::SpscRing>();
        if (cfg.recordPath && *cfg.recordPath)
            s.record = std::fopen(cfg.recordPath, "wb");
        if (!cfg.inlineApply)
            s.worker = std::thread(workerLoop);
        detail::enabledFlag().store(true, std::memory_order_release);
        std::fprintf(stderr, "[vu1-worker] on: queue frames %u%s%s\n", cfg.queueFrames, s.record ? ", recording" : "",
                     cfg.inlineApply ? ", INLINE (items applied at post on the game thread)" : "");
    }

    void stop()
    {
        State &s = state();
        // Hooks and probes point into the runtime that installed them: none outlives a stop (the crash of
        // 2026-10-05: a test runtime's drain hook called after the runtime was gone).
        s.drainHook = nullptr;
        s.drainHookCtx = nullptr;
        setFbrstProbe(nullptr, nullptr);
        if (!enabled())
        {
            s.cfg = Config{};
            return;
        }
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
        s.cfg = Config{};
    }

    void post(vu1work::Kind kind, uint32_t a, uint64_t b, const uint8_t *data, uint32_t size)
    {
        State &s = state();
        const vu1work::ItemView view{kind, a, b, data, size};
        if (s.record)
        {
            vu1work::serialize(view, s.recordBuf);
            if (s.recordBuf.size() >= (8u << 20))
            {
                std::fwrite(s.recordBuf.data(), 1, s.recordBuf.size(), s.record);
                s.recordBuf.clear();
            }
        }
        ++s.items;
        if (s.cfg.inlineApply)
        {
            detail::onWorkerFlag() = true;   // the guards execute, exactly as on the worker
            s.apply(view, s.ctx);
            detail::onWorkerFlag() = false;
            return;
        }
        s.queue->push(kind, a, b, data, size);
        if (s.batchDepth == 0 || s.queue->unpublished() >= 64u || s.queue->consumerSleeping())
            s.queue->publish();
    }

    BatchScope::BatchScope() { ++state().batchDepth; }
    BatchScope::~BatchScope()
    {
        if (--state().batchDepth == 0)
            flush();
    }

    void post(vu1work::WorkItem &&item)
    {
        post(item.kind, item.a, item.b, item.bytes.empty() ? nullptr : item.bytes.data(),
             static_cast<uint32_t>(item.bytes.size()));
    }

    void flush()
    {
        State &s = state();
        if (s.queue && !s.cfg.inlineApply)
            s.queue->publish();
    }

    void drain(vu1overlap::SyncKind why)
    {
        State &s = state();
        if (s.cfg.inlineApply)
        {
            s.drains[int(why)].fetch_add(1, std::memory_order_relaxed);
            ++s.windowDrains[int(why)];
            return;
        }
        const int64_t t0 = nowNs();
        s.queue->waitDone(s.queue->lastPushed());
        const int64_t waited = nowNs() - t0;
        excludeFromGuestClock(waited);
        s.drainWaitNs += waited;
        s.drains[int(why)].fetch_add(1, std::memory_order_relaxed);
        ++s.windowDrains[int(why)];
        if (s.drainHook)
            s.drainHook(s.drainHookCtx);
    }

    void frameBoundary()
    {
        State &s = state();
        const int64_t now = nowNs();
        if (s.cfg.stats)
        {
            const uint64_t readbacks = gsReadbackCount().load(std::memory_order_relaxed);
            if (s.lastBoundaryNs != 0)
            {
                const double interval = double(now - s.lastBoundaryNs) / 1e6;
                s.frames.add(interval);
                ++s.slow[int(classifySlow(interval, s.prevBoundaryBpNs, readbacks - s.readbacksAtBoundary))];
                if (readbacks != s.readbacksAtBoundary)
                {
                    s.withReadbackMs += interval;
                    ++s.withReadback;
                }
                else
                {
                    s.withoutReadbackMs += interval;
                    ++s.withoutReadback;
                }
            }
            s.readbacksAtBoundary = readbacks;
            s.lastBoundaryNs = now;
            if (s.windowStartNs == 0)
                s.windowStartNs = now;
            if (now - s.windowStartNs >= 5'000'000'000ll && !s.frames.ms.empty())
            {
                const double secs = double(now - s.windowStartNs) / 1e9;
                const double n = double(s.frames.ms.size());
                const uint64_t lc = s.latencyCount.exchange(0);
                const int64_t lt = s.latencyTotalNs.exchange(0);
                const int64_t lm = s.latencyMaxNs.exchange(0);
                using K = vu1overlap::SyncKind;
                std::fprintf(stderr,
                             "[vu1-worker] on=%d frames=%zu frame_ms p50=%.1f p95=%.1f p99=%.1f items=%.0f/s drains "
                             "vif1=%.2f dmactl=%.2f gif=%.2f other=%.2f /frame drain_wait=%.1f backpressure_wait=%.1f ms/s "
                             "ahead_max=%u after_wait=%u latency mean=%.1f max=%.1f ms dt_stops=%llu slow worker=%u readback=%u "
                             "both=%u game=%u input_lat p50=%.1f p95=%.1f ms frame_mean readback=%.1f (%u) none=%.1f (%u)\n",
                             enabled() ? 1 : 0, s.frames.ms.size(), s.frames.percentile(50), s.frames.percentile(95),
                             s.frames.percentile(99), double(s.items) / secs, double(s.windowDrains[int(K::Vif1Reg)]) / n,
                             double(s.windowDrains[int(K::DmaCtl)]) / n, double(s.windowDrains[int(K::GifReg)]) / n,
                             double(s.windowDrains[int(K::GsPriv)] + s.windowDrains[int(K::Vu1Mem)]) / n,
                             double(s.drainWaitNs) / 1e6 / secs, double(s.backpressureWaitNs) / 1e6 / secs, s.aheadMax,
                             s.aheadAfterMax, lc ? double(lt) / double(lc) / 1e6 : 0.0, double(lm) / 1e6,
                             static_cast<unsigned long long>(s.dtStops.load()), s.slow[1], s.slow[2], s.slow[3], s.slow[4],
                             latencyPercentile(50.0), latencyPercentile(95.0),
                             s.withReadback ? s.withReadbackMs / s.withReadback : 0.0, s.withReadback,
                             s.withoutReadback ? s.withoutReadbackMs / s.withoutReadback : 0.0, s.withoutReadback);
                s.withReadbackMs = s.withoutReadbackMs = 0;
                s.withReadback = s.withoutReadback = 0;
                std::fill(std::begin(s.slow), std::end(s.slow), 0u);
                s.aheadAfterMax = 0;
                {
                    std::lock_guard<std::mutex> lock(s.latencyMutex);
                    s.latency.clear();
                }
                if (s.statsExtra)
                    s.statsExtra();
                s.frames.clear();
                s.windowStartNs = now;
                s.items = 0;
                std::fill(std::begin(s.windowDrains), std::end(s.windowDrains), 0u);
                s.drainWaitNs = s.backpressureWaitNs = 0;
                s.aheadMax = 0;
            }
        }
        const uint64_t frame = s.frameIndex++;
        s.padTime[s.frameIndex % 64].store(0, std::memory_order_release);   // a frame with no pad read gives no sample
        if (!shouldPost())
        {
            s.completedFrame.store(frame, std::memory_order_release);   // worker off: complete at sceGsSyncV
            s.prevBoundaryBpNs = 0;
            return;
        }
        // a: the frame index (the worker marks it complete); b: the post time (the mark's own latency)
        post(vu1work::Kind::FrameMark, static_cast<uint32_t>(frame), static_cast<uint64_t>(now), nullptr, 0);
        if (s.cfg.inlineApply)
            return;
        s.aheadMax = std::max(s.aheadMax, s.queue->framesAhead());
        const int64_t t0 = nowNs();
        s.queue->waitFramesAhead(s.cfg.queueFrames);
        const int64_t waited = nowNs() - t0;
        excludeFromGuestClock(waited);
        s.backpressureWaitNs += waited;
        s.prevBoundaryBpNs = waited;
        s.aheadAfterMax = std::max(s.aheadAfterMax, s.queue->framesAhead());   // the bound applies here
        if (s.drainHook)
            s.drainHook(s.drainHookCtx);
    }

    std::atomic<uint64_t> &gsReadbackCount()
    {
        static std::atomic<uint64_t> s_count{0};
        return s_count;
    }

    void notePadRead()
    {
        State &s = state();
        if (!s.cfg.stats || s.padFrame == s.frameIndex)
            return;
        s.padFrame = s.frameIndex;
        s.padTime[s.frameIndex % 64].store(nowNs(), std::memory_order_release);
    }

    void notePresent()
    {
        State &s = state();
        if (!s.cfg.stats)
            return;
        const uint64_t done = s.completedFrame.load(std::memory_order_acquire);
        if (done == UINT64_MAX || done == s.lastPresented)
            return;
        s.lastPresented = done;
        const int64_t pad = s.padTime[done % 64].load(std::memory_order_acquire);
        if (pad == 0)
            return;
        std::lock_guard<std::mutex> lock(s.latencyMutex);
        s.latency.push_back(double(nowNs() - pad) / 1e6);
    }

    std::vector<double> latencySamples()
    {
        State &s = state();
        std::lock_guard<std::mutex> lock(s.latencyMutex);
        return s.latency;
    }

    void resetFrameStats()
    {
        State &s = state();
        s.frames.clear();
        s.lastBoundaryNs = 0;
        s.frameIndex = 0;
        s.padFrame = UINT64_MAX;
        for (auto &p : s.padTime)
            p.store(0);
        s.completedFrame.store(UINT64_MAX);
        s.lastPresented = UINT64_MAX;
        {
            std::lock_guard<std::mutex> lock(s.latencyMutex);
            s.latency.clear();
        }
        s.latencyMaxNs = 0;
        s.latencyTotalNs = 0;
        s.latencyCount = 0;
    }
    size_t frameStatsCount() { return state().frames.ms.size(); }
    double frameLatencyMaxMs() { return double(state().latencyMaxNs.load()) / 1e6; }

    void noteVpuStop(uint32_t bits)
    {
        state().vpuStop.store(bits, std::memory_order_relaxed);
        if (bits)
            state().dtStops.fetch_add(1, std::memory_order_relaxed);
    }
    uint32_t pendingVpuStop() { return state().vpuStop.load(std::memory_order_relaxed); }
    void setDrainHook(DrainHookFn fn, void *ctx)
    {
        state().drainHook = fn;
        state().drainHookCtx = ctx;
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
    void setStatsExtra(void (*fn)()) { state().statsExtra = fn; }

    namespace
    {
        double latencyPercentile(double p)
        {
            State &s = state();
            FrameTimes ft;
            {
                std::lock_guard<std::mutex> lock(s.latencyMutex);
                ft.ms = s.latency;
            }
            return ft.percentile(p);
        }
    }
}
