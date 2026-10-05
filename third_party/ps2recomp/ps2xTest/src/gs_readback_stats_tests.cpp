// macOS fork, VU1 worker Part 3: the readback attribution behind the slow-frame "readback" tag.
#include "MiniTest.h"
#include "runtime/gs/gs_readback_stats.h"

#include <string>

void register_gs_readback_stats_tests()
{
    MiniTest::Case("GsReadbackStats", [](TestCase &tc)
    {
        tc.Run("downloads are counted and timed by site, blocked waits by thread; the line resets them", [](TestCase &t)
        {
            using namespace gsreadback;
            (void)takeLine(5.0);   // start clean
            noteDownload(Site::Grow, 1'000'000);
            noteDownload(Site::TexShadow, 2'000'000);
            noteDownload(Site::TexShadow, 2'000'000);
            noteDownload(Site::Command, 3'000'000);
            noteBlocked(Waiter::Worker, 4'000'000);
            noteBlocked(Waiter::Other, 6'000'000);
            noteAsyncRequest();
            const std::string line = takeLine(1.0);
            t.IsTrue(line.find("grow=1/1.0ms") != std::string::npos, "grow: 1 download, 1.0 ms");
            t.IsTrue(line.find("tex=2/4.0ms") != std::string::npos, "texture shadow: 2 downloads, 4.0 ms");
            t.IsTrue(line.find("cmd=1/3.0ms") != std::string::npos, "readback command: 1, 3.0 ms");
            t.IsTrue(line.find("blocked worker=1/4.0ms") != std::string::npos, "the worker blocked once, 4 ms");
            t.IsTrue(line.find("other=1/6.0ms") != std::string::npos, "another thread blocked once, 6 ms");
            t.IsTrue(line.find("async=1") != std::string::npos, "one async request");
            const std::string next = takeLine(1.0);
            t.IsTrue(next.find("grow=0/0.0ms") != std::string::npos, "reset after the line");
        });
    });
}
