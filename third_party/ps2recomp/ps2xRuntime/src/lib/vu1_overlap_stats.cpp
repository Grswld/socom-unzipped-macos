// macOS fork, the VU1 worker's Task 0: the process-wide overlap window behind PS2X_VU1_OVERLAP_TRACE.
#include "runtime/vu1_overlap_stats.h"
#include "ps2x/knobs.h"

#include <chrono>
#include <cstdio>

namespace vu1overlap
{
    namespace
    {
        Window g_window;
        uint64_t g_windowStartNs = 0;
        ProbeFn g_probe = nullptr;
        void *g_probeCtx = nullptr;
    }

    bool on()
    {
        static const bool s_on = ps2x::knob("PS2X_VU1_OVERLAP_TRACE") != nullptr;
        return s_on;
    }
    void setProbe(ProbeFn fn, void *ctx)
    {
        g_probe = fn;
        g_probeCtx = ctx;
    }
    Probe probe() { return g_probe ? g_probe(g_probeCtx) : Probe{}; }
    uint64_t nowNs()
    {
        return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::steady_clock::now().time_since_epoch()).count());
    }
    void addVif1(uint64_t ns, const Probe &p)
    {
        if (!on())
            return;
        g_window.addVif1(ns, p);
        const uint64_t now = nowNs();
        if (g_windowStartNs == 0)
            g_windowStartNs = now;
        if (now - g_windowStartNs >= 1'000'000'000ull)
        {
            std::fprintf(stderr, "%s\n", g_window.report(double(now - g_windowStartNs) / 1e9).c_str());
            g_window = Window{};
            g_windowStartNs = now;
        }
    }
    void ibit()
    {
        if (on())
            g_window.noteIbit(nowNs());
    }
    void stc()
    {
        if (on())
            g_window.noteStc(nowNs());
    }
}
