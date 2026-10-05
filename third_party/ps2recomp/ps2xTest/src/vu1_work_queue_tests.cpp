// macOS fork, the VU1 worker Part 2 Task 1: the work queue (runtime/vu1_work_queue.h).
#include "MiniTest.h"
#include "runtime/vu1_work_queue.h"
#include <atomic>
#include <chrono>
#include <vector>
#include <thread>
using namespace vu1work;

void register_vu1_work_queue_tests()
{
    MiniTest::Case("Vu1WorkQueue", [](TestCase &tc)
    {
        tc.Run("items come out in the order they went in, across threads", [](TestCase &t)
        {
            WorkQueue q;
            std::vector<uint32_t> seen;
            std::thread consumer([&] {
                WorkItem it; uint64_t seq;
                while (q.pop(it, seq)) { seen.push_back(it.a); q.markDone(seq, it.kind == Kind::FrameMark); }
            });
            for (uint32_t i = 0; i < 10000; ++i) { WorkItem it; it.kind = Kind::GsReg; it.a = i; q.push(std::move(it)); }
            q.waitDone(q.lastPushed());
            q.close();
            consumer.join();
            bool ordered = seen.size() == 10000;
            for (uint32_t i = 0; ordered && i < seen.size(); ++i) ordered = seen[i] == i;
            t.IsTrue(ordered, "10000 items in order");
        });

        tc.Run("waitDone returns only after the consumer applied that item", [](TestCase &t)
        {
            WorkQueue q;
            std::atomic<bool> applied{false};
            std::thread consumer([&] {
                WorkItem it; uint64_t seq;
                q.pop(it, seq);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                applied = true;
                q.markDone(seq, false);
            });
            WorkItem it; it.kind = Kind::GsReg;
            const uint64_t seq = q.push(std::move(it));
            q.waitDone(seq);
            t.IsTrue(applied.load(), "applied before waitDone returned");
            q.close(); consumer.join();
        });

        tc.Run("frames ahead: the producer blocks past the limit and resumes when a frame is done", [](TestCase &t)
        {
            WorkQueue q;
            for (int f = 0; f < 3; ++f) { WorkItem it; it.kind = Kind::FrameMark; q.push(std::move(it)); }
            t.Equals(q.framesAhead(), 3u, "three frames in flight");
            std::atomic<bool> released{false};
            std::thread producer([&] { q.waitFramesAhead(1); released = true; });
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            t.IsFalse(released.load(), "blocked at 3 > 1");
            WorkItem it; uint64_t seq;
            q.pop(it, seq); q.markDone(seq, true);
            q.pop(it, seq); q.markDone(seq, true);
            producer.join();
            t.IsTrue(released.load(), "released at 1");
            q.close();
        });

        tc.Run("close wakes a blocked consumer and a blocked producer", [](TestCase &t)
        {
            WorkQueue q;
            std::atomic<bool> exited{false};
            std::thread consumer([&] { WorkItem it; uint64_t seq; while (q.pop(it, seq)) q.markDone(seq, it.kind == Kind::FrameMark); exited = true; });
            WorkItem it; it.kind = Kind::FrameMark;
            q.push(std::move(it)); q.push(WorkItem{}); 
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            q.close();
            consumer.join();
            q.waitFramesAhead(0);   // returns: closed
            q.waitDone(q.lastPushed());
            t.IsTrue(exited.load(), "the consumer left its loop once closed");
        });

        tc.Run("an item survives serialisation byte for byte", [](TestCase &t)
        {
            WorkItem in; in.kind = Kind::Vif1Data; in.a = 0x10009000u; in.b = 0x123456789ABCDEFull;
            in.bytes = {1, 2, 3, 0xFF};
            std::vector<uint8_t> buf; serialize(in, buf);
            const uint8_t *p = buf.data();
            WorkItem out;
            t.IsTrue(deserialize(p, buf.data() + buf.size(), out), "read back");
            t.IsTrue(out.kind == in.kind && out.a == in.a && out.b == in.b && out.bytes == in.bytes, "identical");
            t.IsTrue(p == buf.data() + buf.size(), "consumed exactly");
            const uint8_t *q2 = buf.data();
            t.IsFalse(deserialize(q2, buf.data() + 5, out), "a truncated record is refused");
        });
    });
}
