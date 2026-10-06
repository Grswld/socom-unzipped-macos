// macOS fork, the VU1 worker Part 3 Task 3.1: the lock-free SPSC ring with a preallocated arena (runtime/vu1_spsc_ring.h).
#include "MiniTest.h"
#include "runtime/vu1_spsc_ring.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

using namespace vu1work;

namespace
{
    uint64_t checksum(const uint8_t *p, uint32_t n)
    {
        uint64_t h = 1469598103934665603ull;
        for (uint32_t i = 0; i < n; ++i)
            h = (h ^ p[i]) * 1099511628211ull;
        return h;
    }
    void fill(std::vector<uint8_t> &buf, uint32_t seed, uint32_t n)
    {
        buf.resize(n);
        for (uint32_t i = 0; i < n; ++i)
            buf[i] = static_cast<uint8_t>((seed * 2654435761u + i * 40503u) >> 13);
    }
}

void register_vu1_spsc_ring_tests()
{
    MiniTest::Case("Vu1SpscRing", [](TestCase &tc)
    {
        tc.Run("1,000,000 items cross threads in order with intact payloads", [](TestCase &t)
        {
            SpscRing ring(1u << 12, 1u << 20);
            constexpr uint32_t kItems = 1000000;
            std::atomic<uint64_t> bad{0}, seen{0};
            std::thread consumer([&] {
                ItemView v;
                uint64_t seq;
                uint64_t expect = 1;
                std::vector<uint8_t> ref;
                while (ring.pop(v, seq))
                {
                    if (seq != expect++ || v.a != uint32_t(seq - 1))
                        ++bad;
                    fill(ref, v.a, v.size);
                    if (v.size && std::memcmp(ref.data(), v.data, v.size) != 0)
                        ++bad;
                    ++seen;
                    ring.release(seq, false);
                }
            });
            std::vector<uint8_t> payload;
            for (uint32_t i = 0; i < kItems; ++i)
            {
                const uint32_t n = (i * 7919u) % 600u;   // 0..599 bytes, many sizes, many wraps
                fill(payload, i, n);
                ring.push(Kind::Vif1Data, i, 0, payload.data(), n);
                if (i % 37 == 0)
                    ring.publish();
            }
            ring.publish();
            ring.waitDone(ring.lastPushed());
            ring.close();
            consumer.join();
            t.Equals(seen.load(), uint64_t(kItems), "every item");
            t.Equals(bad.load(), uint64_t(0), "in order, payloads intact");
        });

        tc.Run("pushed items are invisible until published", [](TestCase &t)
        {
            SpscRing ring(64, 4096);
            const uint8_t b[4] = {1, 2, 3, 4};
            ring.push(Kind::GsReg, 1, 0, b, 4);
            ItemView v;
            uint64_t seq;
            t.IsFalse(ring.tryPop(v, seq), "not yet");
            ring.publish();
            t.IsTrue(ring.tryPop(v, seq), "now");
            t.IsTrue(v.size == 4 && v.data[3] == 4, "the payload");
            ring.release(seq, false);
        });

        tc.Run("waitDone returns only after release of that item; frames ahead counts frame marks", [](TestCase &t)
        {
            SpscRing ring(64, 4096);
            ring.push(Kind::FrameMark, 0, 0, nullptr, 0);
            ring.push(Kind::FrameMark, 0, 0, nullptr, 0);
            const uint64_t last = ring.push(Kind::GsReg, 0, 0, nullptr, 0);
            ring.publish();
            t.Equals(ring.framesAhead(), 2u, "two frames in flight");
            std::atomic<bool> released{false};
            std::thread consumer([&] {
                ItemView v;
                uint64_t seq;
                for (int i = 0; i < 3 && ring.pop(v, seq); ++i)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    if (seq == last)
                        released = true;
                    ring.release(seq, v.kind == Kind::FrameMark);
                }
            });
            ring.waitFramesAhead(0);
            t.Equals(ring.framesAhead(), 0u, "both frames applied");
            ring.waitDone(last);
            t.IsTrue(released.load(), "the last item released before waitDone returned");
            consumer.join();
            ring.close();
        });

        tc.Run("a full ring and a full arena block the producer, then it continues", [](TestCase &t)
        {
            SpscRing ring(8, 1024);
            std::vector<uint8_t> big(300, 7);
            std::atomic<int> pushed{0};
            std::thread producer([&] {
                for (int i = 0; i < 40; ++i)
                {
                    ring.push(Kind::Vif1Data, uint32_t(i), 0, big.data(), uint32_t(big.size()));
                    ring.publish();
                    ++pushed;
                }
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            t.IsTrue(pushed.load() < 40, "blocked while nothing is consumed");
            ItemView v;
            uint64_t seq;
            int got = 0;
            while (got < 40 && ring.pop(v, seq))
            {
                if (v.size != 300 || v.data[299] != 7)
                    break;
                ring.release(seq, false);
                ++got;
            }
            producer.join();
            t.Equals(got, 40, "all forty, intact");
            ring.close();
        });

        tc.Run("a payload bigger than half the arena still goes through", [](TestCase &t)
        {
            SpscRing ring(16, 4096);
            std::vector<uint8_t> huge;
            fill(huge, 99, 10000);
            ring.push(Kind::Vif1Data, 0, 0, huge.data(), uint32_t(huge.size()));
            ring.publish();
            ItemView v;
            uint64_t seq;
            t.IsTrue(ring.tryPop(v, seq), "popped");
            t.IsTrue(v.size == 10000 && checksum(v.data, v.size) == checksum(huge.data(), 10000), "intact");
            ring.release(seq, false);
        });

        tc.Run("no lost wake-up: bursts with sleeps between them all arrive", [](TestCase &t)
        {
            SpscRing ring(256, 1u << 16);
            std::atomic<uint64_t> seen{0};
            std::thread consumer([&] {
                ItemView v;
                uint64_t seq;
                while (ring.pop(v, seq))
                {
                    ++seen;
                    ring.release(seq, false);
                }
            });
            uint64_t sent = 0;
            for (int burst = 0; burst < 300; ++burst)
            {
                for (int i = 0; i < burst % 7; ++i)
                {
                    ring.push(Kind::GsReg, 0, 0, nullptr, 0);
                    ++sent;
                }
                ring.publish();
                if (burst % 3 == 0)
                    std::this_thread::sleep_for(std::chrono::microseconds(200));   // let the worker fall asleep
            }
            ring.waitDone(ring.lastPushed());
            t.Equals(seen.load(), sent, "every item, none stranded behind a sleeping worker");
            ring.close();
            consumer.join();
        });

        tc.Run("close wakes a sleeping consumer and a waiting producer", [](TestCase &t)
        {
            SpscRing ring(8, 1024);
            std::thread consumer([&] {
                ItemView v;
                uint64_t seq;
                while (ring.pop(v, seq))
                    ring.release(seq, false);
            });
            ring.push(Kind::FrameMark, 0, 0, nullptr, 0);   // never published: the consumer stays asleep
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ring.close();
            consumer.join();
            ring.waitFramesAhead(0);
            ring.waitDone(ring.lastPushed());
            t.IsTrue(true, "no hang");
        });

        // The run-3 freeze of 2026-10-05 (PS2X_VU1_QUEUE_FRAMES=0): the game thread waits for 0 frames ahead after
        // every frame mark, so the mark is the last item and no later release can rescue a missed wake-up.
        tc.Run("no lost wake-up: waitFramesAhead(0) after every frame mark, 300,000 times", [](TestCase &t)
        {
            SpscRing ring(1024, 1 << 20);
            std::thread consumer([&] {
                ItemView v;
                uint64_t seq;
                while (ring.pop(v, seq))
                {
                    if (v.kind == Kind::FrameMark)   // long enough that the producer reaches its sleep path
                    {
                        const auto until = std::chrono::steady_clock::now() + std::chrono::microseconds(3);
                        while (std::chrono::steady_clock::now() < until)
                        {
                        }
                    }
                    ring.release(seq, v.kind == Kind::FrameMark);
                }
            });
            std::atomic<uint64_t> rounds{0};
            std::atomic<bool> producerDone{false};
            std::thread producer([&] {
                for (int i = 0; i < 300000; ++i)
                {
                    ring.push(Kind::GsReg, 0, 0, nullptr, 0);
                    ring.push(Kind::FrameMark, 0, 0, nullptr, 0);
                    ring.waitFramesAhead(0);
                    rounds.fetch_add(1, std::memory_order_relaxed);
                }
                producerDone = true;
            });
            bool hung = false;
            uint64_t last = 0;
            auto lastMove = std::chrono::steady_clock::now();
            while (!producerDone)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                const uint64_t r = rounds.load();
                if (r != last)
                {
                    last = r;
                    lastMove = std::chrono::steady_clock::now();
                }
                else if (std::chrono::steady_clock::now() - lastMove > std::chrono::seconds(2))
                {
                    hung = true;
                    break;
                }
            }
            ring.close();   // frees a stuck producer (and the consumer) either way
            producer.join();
            consumer.join();
            t.IsTrue(!hung, "the producer never stalls (it stalled after " + std::to_string(last) + " rounds)");
        });
    });
}
