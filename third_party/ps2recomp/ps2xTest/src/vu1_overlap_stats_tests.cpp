// macOS fork, the VU1 worker's Task 0: the overlap window (runtime/vu1_overlap_stats.h).
#include "MiniTest.h"
#include "runtime/vu1_overlap_stats.h"

using namespace vu1overlap;

void register_vu1_overlap_stats_tests()
{
    MiniTest::Case("Vu1OverlapWindow", [](TestCase &tc)
    {
        tc.Run("VIF1 time counts as overlappable only when another guest thread was ready", [](TestCase &t)
        {
            Window w;
            w.addVif1(4'000'000, Probe{2, 7});
            w.addVif1(1'000'000, Probe{0, 7});
            w.addVif1(3'000'000, Probe{1, 9});
            t.Equals(w.vif1Ns, uint64_t(8'000'000), "all VIF1 time");
            t.Equals(w.overlappableNs, uint64_t(7'000'000), "the two with a ready thread");
            t.Equals(w.kicks, uint64_t(3), "kicks");
            t.Equals(w.kicksByTid.at(7), uint64_t(2), "thread 7 kicked twice");
            t.Equals(w.kicksByTid.at(9), uint64_t(1), "thread 9 once");
        });

        tc.Run("a probe-less window still counts", [](TestCase &t)
        {
            Window w;
            w.addVif1(2'000'000, Probe{});
            t.Equals(w.vif1Ns, uint64_t(2'000'000), "counted");
            t.Equals(w.overlappableNs, uint64_t(0), "unknown is not overlappable");
            t.IsTrue(w.kicksByTid.empty(), "no thread recorded");
        });

        tc.Run("i-bit to STC latency: mean and max", [](TestCase &t)
        {
            Window w;
            w.noteIbit(100); w.noteStc(1'100);
            w.noteIbit(5'000); w.noteStc(8'000);
            t.Equals(w.ibits, uint64_t(2), "two i-bits");
            t.Equals(w.stcLatencyNsTotal, uint64_t(4'000), "1000 + 3000");
            t.Equals(w.stcLatencyNsMax, uint64_t(3'000), "max");
            t.Equals(w.stcLatencySamples, uint64_t(2), "two samples");
        });

        tc.Run("an STC without an i-bit records no latency", [](TestCase &t)
        {
            Window w;
            w.noteStc(500);
            t.Equals(w.stcs, uint64_t(1), "counted");
            t.Equals(w.stcLatencySamples, uint64_t(0), "no sample");
        });

        tc.Run("the latest i-bit starts the latency", [](TestCase &t)
        {
            Window w;
            w.noteIbit(100); w.noteIbit(900); w.noteStc(1'000);
            t.Equals(w.stcLatencyNsTotal, uint64_t(100), "from 900, not 100");
        });

        tc.Run("the report line, and a zero-length window", [](TestCase &t)
        {
            Window w;
            w.addVif1(500'000'000, Probe{1, 3});
            w.noteIbit(0); w.noteStc(2'000'000);
            const std::string line = w.report(1.0);
            t.IsTrue(line.find("vif1=500 ms/s") != std::string::npos, line);
            t.IsTrue(line.find("overlappable=500 ms/s (100%)") != std::string::npos, line);
            t.IsTrue(line.find("ibits=1.0/s") != std::string::npos, line);
            t.IsTrue(line.find("stc_latency mean=2.00 max=2.00 ms") != std::string::npos, line);
            t.IsTrue(line.find("by_tid=3:1") != std::string::npos, line);
            t.IsTrue(Window{}.report(0.0).find("vif1=0 ms/s") != std::string::npos, "zero-length window");
        });
    });
}
