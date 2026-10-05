// macOS fork, the VU1 worker Part 2 Task 2: the domain (runtime/vu1_domain.h) driven with a recording ApplyFn.
#include "MiniTest.h"
#include "runtime/ps2_guest_clock.h"
#include "runtime/vu1_domain.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <mutex>
#include <thread>
#include <vector>

using namespace vu1work;

namespace
{
    struct Recorder
    {
        std::mutex mutex;
        std::vector<std::pair<Kind, uint32_t>> applied;
        std::thread::id thread;
        bool onWorkerInside = false;
        int sleepMsOnFrameMark = 0;
        int sleepMsOnEach = 0;
    };
    Recorder g_rec;

    void recordApply(const ItemView &item, void *)
    {
        if (g_rec.sleepMsOnEach)
            std::this_thread::sleep_for(std::chrono::milliseconds(g_rec.sleepMsOnEach));
        if (item.kind == Kind::FrameMark && g_rec.sleepMsOnFrameMark)
            std::this_thread::sleep_for(std::chrono::milliseconds(g_rec.sleepMsOnFrameMark));
        std::lock_guard<std::mutex> lock(g_rec.mutex);
        g_rec.applied.emplace_back(item.kind, item.a);
        g_rec.thread = std::this_thread::get_id();
        g_rec.onWorkerInside = vu1domain::onWorker();
    }

    void resetRecorder()
    {
        std::lock_guard<std::mutex> lock(g_rec.mutex);
        g_rec.applied.clear();
        g_rec.onWorkerInside = false;
        g_rec.sleepMsOnFrameMark = 0;
        g_rec.sleepMsOnEach = 0;
    }

    WorkItem item(Kind k, uint32_t a = 0)
    {
        WorkItem it;
        it.kind = k;
        it.a = a;
        return it;
    }

    vu1domain::Config on(unsigned frames = 1)
    {
        vu1domain::Config c;
        c.enabled = true;
        c.queueFrames = frames;
        return c;
    }
}

void register_vu1_domain_tests()
{
    MiniTest::Case("Vu1Domain", [](TestCase &tc)
    {
        tc.Run("off: no worker, shouldPost is false", [](TestCase &t)
        {
            resetRecorder();
            vu1domain::start(vu1domain::Config{}, recordApply, nullptr);
            t.IsFalse(vu1domain::enabled(), "not enabled");
            t.IsFalse(vu1domain::shouldPost(), "nothing is posted");
            vu1domain::stop();
        });

        tc.Run("on: items apply on another thread, in order", [](TestCase &t)
        {
            resetRecorder();
            vu1domain::start(on(), recordApply, nullptr);
            t.IsTrue(vu1domain::shouldPost(), "posting");
            for (uint32_t i = 0; i < 500; ++i)
                vu1domain::post(item(Kind::GsReg, i));
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            std::lock_guard<std::mutex> lock(g_rec.mutex);
            bool ordered = g_rec.applied.size() == 500;
            for (uint32_t i = 0; ordered && i < 500; ++i)
                ordered = g_rec.applied[i].second == i;
            t.IsTrue(ordered, "500 in order");
            t.IsFalse(g_rec.thread == std::this_thread::get_id(), "applied on the worker");
            vu1domain::stop();
        });

        tc.Run("onWorker is true inside apply and false outside", [](TestCase &t)
        {
            resetRecorder();
            vu1domain::start(on(), recordApply, nullptr);
            vu1domain::post(item(Kind::GsReg));
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            t.IsTrue(g_rec.onWorkerInside, "inside");
            t.IsFalse(vu1domain::onWorker(), "outside");
            vu1domain::stop();
        });

        tc.Run("drain returns after every posted item applied, and counts its kind", [](TestCase &t)
        {
            resetRecorder();
            g_rec.sleepMsOnEach = 2;
            vu1domain::start(on(), recordApply, nullptr);
            const uint64_t before = vu1domain::drainCount(vu1overlap::SyncKind::GifReg);
            for (int i = 0; i < 10; ++i)
                vu1domain::post(item(Kind::GsReg));
            vu1domain::drain(vu1overlap::SyncKind::GifReg);
            {
                std::lock_guard<std::mutex> lock(g_rec.mutex);
                t.Equals(g_rec.applied.size(), size_t(10), "all ten applied");
            }
            t.Equals(vu1domain::drainCount(vu1overlap::SyncKind::GifReg), before + 1, "one GIF drain counted");
            vu1domain::stop();
        });

        tc.Run("frameBoundary with queueFrames 0 returns only after the worker applied that frame", [](TestCase &t)
        {
            resetRecorder();
            g_rec.sleepMsOnFrameMark = 10;
            vu1domain::start(on(0), recordApply, nullptr);
            vu1domain::post(item(Kind::GsReg, 1));
            vu1domain::frameBoundary();
            {
                std::lock_guard<std::mutex> lock(g_rec.mutex);
                t.Equals(g_rec.applied.size(), size_t(2), "the item and the frame mark both applied");
            }
            vu1domain::stop();
        });

        tc.Run("frameBoundary with queueFrames 1 lets one frame run ahead", [](TestCase &t)
        {
            resetRecorder();
            g_rec.sleepMsOnFrameMark = 30;
            vu1domain::start(on(1), recordApply, nullptr);
            const auto t0 = std::chrono::steady_clock::now();
            vu1domain::frameBoundary();   // frame 1 in flight: returns at once
            const auto t1 = std::chrono::steady_clock::now();
            vu1domain::frameBoundary();   // frame 2: waits for frame 1
            const auto t2 = std::chrono::steady_clock::now();
            t.IsTrue(t1 - t0 < std::chrono::milliseconds(15), "the first boundary does not wait");
            t.IsTrue(t2 - t1 >= std::chrono::milliseconds(15), "the second waits for the first frame");
            vu1domain::stop();
        });

        tc.Run("stop drains before it returns", [](TestCase &t)
        {
            resetRecorder();
            g_rec.sleepMsOnEach = 1;
            vu1domain::start(on(), recordApply, nullptr);
            for (int i = 0; i < 20; ++i)
                vu1domain::post(item(Kind::GsReg));
            vu1domain::stop();
            std::lock_guard<std::mutex> lock(g_rec.mutex);
            t.Equals(g_rec.applied.size(), size_t(20), "every item applied");
            t.IsFalse(vu1domain::enabled(), "stopped");
        });

        tc.Run("items apply in order across a VIF1 stall (data, STC, PATH3)", [](TestCase &t)
        {
            resetRecorder();
            vu1domain::start(on(), recordApply, nullptr);
            vu1domain::post(item(Kind::Vif1Data, 1));
            vu1domain::post(item(Kind::Vif1Reg, 0x10003C10u));   // FBRST (STC)
            vu1domain::post(item(Kind::GifPath3, 3));
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            std::lock_guard<std::mutex> lock(g_rec.mutex);
            t.IsTrue(g_rec.applied.size() == 3 && g_rec.applied[0].first == Kind::Vif1Data &&
                         g_rec.applied[1].first == Kind::Vif1Reg && g_rec.applied[2].first == Kind::GifPath3,
                     "data, STC, PATH3");
            vu1domain::stop();
        });

        tc.Run("the recorder writes what was posted", [](TestCase &t)
        {
            resetRecorder();
            const std::string path = (std::filesystem::temp_directory_path() / "vu1q_test.vq").string();
            std::filesystem::remove(path);
            vu1domain::Config c = on();
            c.recordPath = path.c_str();
            vu1domain::start(c, recordApply, nullptr);
            WorkItem d = item(Kind::Vif1Data, 7);
            d.bytes = {9, 8, 7};
            vu1domain::post(std::move(d));
            vu1domain::post(item(Kind::FrameMark));
            vu1domain::stop();
            std::ifstream in(path, std::ios::binary);
            std::vector<uint8_t> buf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            const uint8_t *p = buf.data();
            WorkItem a, b;
            t.IsTrue(deserialize(p, buf.data() + buf.size(), a) && deserialize(p, buf.data() + buf.size(), b), "two records");
            t.IsTrue(a.kind == Kind::Vif1Data && a.a == 7 && a.bytes == std::vector<uint8_t>{9, 8, 7}, "the data item");
            t.IsTrue(b.kind == Kind::FrameMark, "the frame mark");
            std::filesystem::remove(path);
        });

        tc.Run("knobs: off by default, queue frames bounded 0-4", [](TestCase &t)
        {
            const auto none = vu1domain::configFrom([](const char *) -> const char * { return nullptr; });
            t.IsFalse(none.enabled, "off by default");
            t.Equals(none.queueFrames, 1u, "one frame by default");
            const auto set = vu1domain::configFrom([](const char *n) -> const char * {
                if (std::string(n) == "PS2X_VU1_THREAD") return "1";
                if (std::string(n) == "PS2X_VU1_QUEUE_FRAMES") return "9";
                return nullptr;
            });
            t.IsTrue(set.enabled, "on");
            t.Equals(set.queueFrames, 1u, "out of range: the default");
        });
    });

    MiniTest::Case("Vu1DomainStats", [](TestCase &tc)
    {
        tc.Run("frame-time percentiles (nearest rank)", [](TestCase &t)
        {
            vu1domain::FrameTimes ft;
            for (int i = 1; i <= 100; ++i)
                ft.add(double(i));
            t.Equals(ft.percentile(50.0), 50.0, "p50");
            t.Equals(ft.percentile(95.0), 95.0, "p95");
            t.Equals(ft.percentile(99.0), 99.0, "p99");
            t.Equals(vu1domain::FrameTimes{}.percentile(99.0), 0.0, "empty");
        });

        tc.Run("a drain's wait is subtracted from guest time", [](TestCase &t)
        {
            resetRecorder();
            g_rec.sleepMsOnEach = 30;
            vu1domain::start(on(), recordApply, nullptr);
            vu1domain::post(item(Kind::GsReg));
            const int64_t before = ps2GuestClockExcludedNs().load();
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            const int64_t added = ps2GuestClockExcludedNs().load() - before;
            vu1domain::stop();
            t.IsTrue(added >= 20'000'000, "at least 20 ms excluded, got " + std::to_string(added));
        });

        tc.Run("frame times are kept with the worker off too (the A/B baseline)", [](TestCase &t)
        {
            vu1domain::Config c;
            c.stats = true;
            vu1domain::start(c, recordApply, nullptr);
            vu1domain::resetFrameStats();
            for (int i = 0; i < 4; ++i)
            {
                vu1domain::frameBoundary();
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            t.Equals(vu1domain::frameStatsCount(), size_t(3), "three intervals between four boundaries");
            vu1domain::stop();
        });

        tc.Run("a frame mark's post-to-applied latency is measured", [](TestCase &t)
        {
            resetRecorder();
            g_rec.sleepMsOnFrameMark = 25;
            vu1domain::Config c = on(0);
            c.stats = true;
            vu1domain::start(c, recordApply, nullptr);
            vu1domain::resetFrameStats();
            vu1domain::frameBoundary();
            vu1domain::stop();
            t.IsTrue(vu1domain::frameLatencyMaxMs() >= 20.0, "latency at least the frame mark's 25 ms");
        });
    });

    MiniTest::Case("Vu1DomainHooks", [](TestCase &tc)
    {
        tc.Run("stop clears the drain hook and the fbrst probe (no hook outlives its runtime)", [](TestCase &t)
        {
            static int s_hookCalls = 0;
            s_hookCalls = 0;
            vu1domain::setDrainHook([](void *) { ++s_hookCalls; }, nullptr);
            vu1domain::setFbrstProbe([](void *) -> uint64_t { return 0x400u; }, nullptr);
            vu1domain::stop();   // the worker is off: stop must still drop them
            t.Equals(vu1domain::eeFbrst(), uint64_t(0), "probe cleared");
            resetRecorder();
            vu1domain::start(on(), recordApply, nullptr);
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            vu1domain::stop();
            t.Equals(s_hookCalls, 0, "the old hook is never called");
        });
    });

    MiniTest::Case("Vu1DomainInline", [](TestCase &tc)
    {
        tc.Run("inline (the bisect mode): post applies at once, on the calling thread, as the worker would", [](TestCase &t)
        {
            resetRecorder();
            vu1domain::Config c = on();
            c.inlineApply = true;
            vu1domain::start(c, recordApply, nullptr);
            t.IsTrue(vu1domain::shouldPost(), "the guards post");
            vu1domain::post(item(Kind::GsReg, 42));
            {
                std::lock_guard<std::mutex> lock(g_rec.mutex);
                t.Equals(g_rec.applied.size(), size_t(1), "applied before post returned");
                t.IsTrue(g_rec.thread == std::this_thread::get_id(), "on the calling thread");
                t.IsTrue(g_rec.onWorkerInside, "onWorker inside apply, so guards execute");
            }
            t.IsFalse(vu1domain::onWorker(), "and false again after");
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);   // a no-op
            vu1domain::frameBoundary();                        // applies its frame mark inline
            vu1domain::stop();
            std::lock_guard<std::mutex> lock(g_rec.mutex);
            t.Equals(g_rec.applied.size(), size_t(2), "the frame mark applied too");
        });
    });

    MiniTest::Case("Vu1DomainTail", [](TestCase &tc)
    {
        tc.Run("slow frames are tagged worker, readback, both or game; fast frames are not tagged", [](TestCase &t)
        {
            using vu1domain::SlowTag;
            t.Equals(int(vu1domain::classifySlow(33.0, 0, 0)), int(SlowTag::Fast), "33.0 ms is not slow");
            t.Equals(int(vu1domain::classifySlow(40.0, 2'000'000, 0)), int(SlowTag::Worker), "a backpressure wait");
            t.Equals(int(vu1domain::classifySlow(40.0, 100'000, 1)), int(SlowTag::Readback), "a readback, no real wait");
            t.Equals(int(vu1domain::classifySlow(40.0, 2'000'000, 3)), int(SlowTag::Both), "both");
            t.Equals(int(vu1domain::classifySlow(40.0, 0, 0)), int(SlowTag::Game), "neither");
        });

        tc.Run("input-to-display latency: the completed frame's first pad read to the next present", [](TestCase &t)
        {
            vu1domain::Config c;
            c.stats = true;
            vu1domain::start(c, recordApply, nullptr);   // worker off: the frame completes at sceGsSyncV
            vu1domain::resetFrameStats();
            vu1domain::notePadRead();
            std::this_thread::sleep_for(std::chrono::milliseconds(12));
            vu1domain::notePadRead();   // a second read in the same frame does not move the frame's input time
            vu1domain::frameBoundary();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            vu1domain::notePresent();
            vu1domain::notePresent();   // the same completed frame presented again: no second sample
            const std::vector<double> lat = vu1domain::latencySamples();
            vu1domain::stop();
            t.Equals(lat.size(), size_t(1), "one sample");
            t.IsTrue(!lat.empty() && lat[0] >= 15.0 && lat[0] < 80.0, "about 17 ms, from the first read");
        });

        tc.Run("with the worker on, the frame completes when the worker applies its frame mark", [](TestCase &t)
        {
            resetRecorder();
            g_rec.sleepMsOnFrameMark = 20;
            vu1domain::Config c = on(1);
            c.stats = true;
            vu1domain::start(c, recordApply, nullptr);
            vu1domain::resetFrameStats();
            vu1domain::notePadRead();
            vu1domain::frameBoundary();          // the mark is posted; the worker takes 20 ms with it
            vu1domain::notePresent();            // nothing completed yet: no sample
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
            vu1domain::notePresent();
            const std::vector<double> lat = vu1domain::latencySamples();
            vu1domain::stop();
            t.Equals(lat.size(), size_t(1), "one sample, after the worker applied the mark");
            t.IsTrue(!lat.empty() && lat[0] >= 20.0, "at least the worker's 20 ms");
        });
    });

    MiniTest::Case("Vu1DomainLatencyStale", [](TestCase &tc)
    {
        tc.Run("a frame with no pad read gives no latency sample (no stale time from 64 frames before)", [](TestCase &t)
        {
            vu1domain::Config c;
            c.stats = true;
            vu1domain::start(c, recordApply, nullptr);
            vu1domain::resetFrameStats();
            vu1domain::notePadRead();
            for (int i = 0; i < 64; ++i)
                vu1domain::frameBoundary();   // frame 0 had a read; frames 1..63 none
            vu1domain::frameBoundary();       // frame 64 (same slot as frame 0) completes, without a read
            vu1domain::notePresent();
            const size_t n = vu1domain::latencySamples().size();
            vu1domain::stop();
            t.Equals(n, size_t(0), "no sample for a frame without a pad read");
        });
    });

    MiniTest::Case("Vu1DomainFrameLog", [](TestCase &tc)
    {
        tc.Run("the frame log has one F row per game frame and one L row per latency sample", [](TestCase &t)
        {
            const std::string path = "vu1_frame_log_test.csv";
            std::remove(path.c_str());
            vu1domain::Config c;
            c.stats = true;
            c.frameLogPath = path;
            vu1domain::start(c, recordApply, nullptr);
            vu1domain::resetFrameStats();
            for (int i = 0; i < 3; ++i)
            {
                vu1domain::notePadRead();
                vu1domain::frameBoundary();
                vu1domain::notePresent();
            }
            vu1domain::stop();   // closes the file
            std::ifstream in(path);
            std::string line;
            int f = 0, l = 0;
            bool header = false;
            while (std::getline(in, line))
            {
                if (line.rfind("#", 0) == 0)
                    header = true;
                else if (line.rfind("F,", 0) == 0)
                    ++f;
                else if (line.rfind("L,", 0) == 0)
                    ++l;
            }
            std::remove(path.c_str());
            t.IsTrue(header, "a header line");
            t.Equals(f, 2, "frames 2 and 3 have an interval (the first boundary only starts the clock)");
            t.Equals(l, 3, "three presents of three completed frames");
        });
    });
}
