#pragma once
// macOS fork, the VU1 worker Part 3: what the slow-frame "readback" tag is made of. The GL backend's render-target
// downloads (glReadPixels) are counted and timed by the site that asked for them, and every caller that waits for one
// (syncDirtyPagesForRead: a local-to-host or local-to-local transfer, a VRAM read) is timed by the thread it blocks.
// Read and reset once per [vu1-worker] stats window (PS2X_VU1_WORKER_STATS); a few relaxed atomics otherwise.
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <string>

namespace gsreadback
{
    enum class Site : int { Grow, TexShadow, Command, Count };   // growRenderTarget, a texture over a stale RT, CmdType::Readback
    enum class Waiter : int { Worker, Other, Count };            // the VU1 worker; any other thread (the game thread)

    namespace detail
    {
        struct Pair
        {
            std::atomic<uint64_t> n{0}, ns{0};
        };
        inline Pair g_site[int(Site::Count)];
        inline Pair g_blocked[int(Waiter::Count)];
        inline std::atomic<uint64_t> g_async{0};

        inline void take(Pair &p, uint64_t &n, double &ms)
        {
            n = p.n.exchange(0, std::memory_order_relaxed);
            ms = double(p.ns.exchange(0, std::memory_order_relaxed)) / 1e6;
        }
    }

    inline void noteDownload(Site s, int64_t ns)
    {
        detail::g_site[int(s)].n.fetch_add(1, std::memory_order_relaxed);
        detail::g_site[int(s)].ns.fetch_add(uint64_t(ns), std::memory_order_relaxed);
    }
    inline void noteBlocked(Waiter w, int64_t ns)
    {
        detail::g_blocked[int(w)].n.fetch_add(1, std::memory_order_relaxed);
        detail::g_blocked[int(w)].ns.fetch_add(uint64_t(ns), std::memory_order_relaxed);
    }
    inline void noteAsyncRequest() { detail::g_async.fetch_add(1, std::memory_order_relaxed); }

    // The window's totals divided by `secs` (counts and ms per second), then reset.
    inline std::string takeLine(double secs)
    {
        uint64_t n[5];
        double ms[5];
        detail::take(detail::g_site[0], n[0], ms[0]);
        detail::take(detail::g_site[1], n[1], ms[1]);
        detail::take(detail::g_site[2], n[2], ms[2]);
        detail::take(detail::g_blocked[0], n[3], ms[3]);
        detail::take(detail::g_blocked[1], n[4], ms[4]);
        const uint64_t async = detail::g_async.exchange(0, std::memory_order_relaxed);
        if (secs <= 0)
            secs = 1;
        char buf[320];
        std::snprintf(buf, sizeof buf,
                      "readbacks/s grow=%llu/%.1fms tex=%llu/%.1fms cmd=%llu/%.1fms blocked worker=%llu/%.1fms "
                      "other=%llu/%.1fms async=%llu",
                      (unsigned long long)(n[0] / secs), ms[0] / secs, (unsigned long long)(n[1] / secs), ms[1] / secs,
                      (unsigned long long)(n[2] / secs), ms[2] / secs, (unsigned long long)(n[3] / secs), ms[3] / secs,
                      (unsigned long long)(n[4] / secs), ms[4] / secs, (unsigned long long)(async / secs));
        return buf;
    }
}
