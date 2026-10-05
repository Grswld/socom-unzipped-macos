#pragma once
// macOS fork, the VU1 worker Part 3 Task 3.1: the queue as a lock-free single-producer / single-consumer ring with a
// preallocated payload arena. The game thread is the only producer, the VU1 worker the only consumer. A mission posts
// ~160,000 items a second; the WorkQueue (one mutex, one notify and one heap copy per item) cost both threads, so here:
//   - no lock and no heap allocation per item: fixed slots, and each payload copied once into a byte arena that wraps
//     (a payload over half the arena gets a heap block of its own, owned by its slot);
//   - items are invisible until publish(); the domain publishes every 64 items, at batch ends, or at once when the
//     worker is asleep, and notifies only then;
//   - the worker sleeps on the published index (std::atomic::wait) behind a sleeping flag, the producer on the released
//     index behind a waiting flag; the flag and the index are seq_cst on both sides, so no wake-up is lost;
//   - the worker reads a payload in place (ItemView) and releases slot and arena strictly in order.
#include "runtime/vu1_work_queue.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

namespace vu1work
{
    struct ItemView
    {
        Kind kind = Kind::FrameMark;
        uint32_t a = 0;
        uint64_t b = 0;
        const uint8_t *data = nullptr;
        uint32_t size = 0;
    };

    inline ItemView viewOf(const WorkItem &item)
    {
        return ItemView{item.kind, item.a, item.b, item.bytes.empty() ? nullptr : item.bytes.data(),
                        static_cast<uint32_t>(item.bytes.size())};
    }

    class SpscRing
    {
    public:
        explicit SpscRing(size_t slots = size_t(1) << 16, size_t arenaBytes = size_t(64) << 20)
            : m_slots(roundPow2(slots)), m_mask(m_slots.size() - 1), m_arena(new uint8_t[arenaBytes]),
              m_arenaCap(arenaBytes)
        {
        }

        // ---- producer ----------------------------------------------------------------------------------------
        uint64_t push(Kind kind, uint32_t a, uint64_t b, const uint8_t *data, uint32_t size)
        {
            const uint64_t seq = m_writeSeq + 1;
            waitProducer([&] { return seq - m_releasedSeq.load(std::memory_order_acquire) <= m_slots.size(); });
            Slot &s = m_slots[(seq - 1) & m_mask];
            s.kind = kind;
            s.a = a;
            s.b = b;
            s.size = size;
            s.heap.reset();
            if (size == 0u || data == nullptr)
            {
                s.data = nullptr;
                s.size = 0;
                s.arenaEnd = m_arenaHead;
            }
            else if (size > m_arenaCap / 2u)
            {
                s.heap.reset(new uint8_t[size]);
                std::memcpy(s.heap.get(), data, size);
                s.data = s.heap.get();
                s.arenaEnd = m_arenaHead;
            }
            else
            {
                uint64_t start = m_arenaHead;
                const uint64_t pos = start % m_arenaCap;
                if (pos + size > m_arenaCap)
                    start += m_arenaCap - pos;   // the tail is skipped; the payload starts at offset 0
                const uint64_t end = start + size;
                waitProducer([&] { return end - m_arenaTail.load(std::memory_order_acquire) <= m_arenaCap; });
                uint8_t *dst = m_arena.get() + (start % m_arenaCap);
                std::memcpy(dst, data, size);
                s.data = dst;
                s.arenaEnd = end;
                m_arenaHead = end;
            }
            if (kind == Kind::FrameMark)
                ++m_pushedFrames;
            m_writeSeq = seq;
            return seq;
        }

        void publish()
        {
            if (m_published.load(std::memory_order_relaxed) == m_writeSeq)
                return;
            m_published.store(m_writeSeq, std::memory_order_seq_cst);
            if (m_consumerSleeping.load(std::memory_order_seq_cst))
                wake(m_consumerEpoch);
        }

        bool consumerSleeping() const { return m_consumerSleeping.load(std::memory_order_relaxed); }
        uint64_t lastPushed() const { return m_writeSeq; }
        uint64_t unpublished() const { return m_writeSeq - m_published.load(std::memory_order_relaxed); }

        void waitDone(uint64_t seq)
        {
            publish();
            waitProducer([&] { return m_releasedSeq.load(std::memory_order_acquire) >= seq; });
        }

        void waitFramesAhead(unsigned limit)
        {
            publish();
            waitProducer([&] { return framesAhead() <= limit; });
        }

        unsigned framesAhead() const
        {
            return static_cast<unsigned>(m_pushedFrames - m_doneFrames.load(std::memory_order_acquire));
        }

        // ---- consumer ----------------------------------------------------------------------------------------
        bool tryPop(ItemView &out, uint64_t &seq)
        {
            const uint64_t next = m_readSeq + 1;
            if (m_published.load(std::memory_order_acquire) < next)
                return false;
            const Slot &s = m_slots[(next - 1) & m_mask];
            out = ItemView{s.kind, s.a, s.b, s.data, s.size};
            seq = next;
            m_readSeq = next;
            return true;
        }

        bool pop(ItemView &out, uint64_t &seq)
        {
            for (;;)
            {
                if (tryPop(out, seq))
                    return true;
                if (m_closed.load(std::memory_order_acquire))
                    return tryPop(out, seq);
                for (int i = 0; i < 128 && m_published.load(std::memory_order_acquire) == m_readSeq; ++i)
                    cpuRelax();
                if (m_published.load(std::memory_order_acquire) != m_readSeq)
                    continue;
                // The epoch is read before the last look: a publish or close after that look bumps it, so the wait
                // below returns (std::atomic::wait returns only on a changed value -- a bare notify is not enough).
                const uint32_t epoch = m_consumerEpoch.load(std::memory_order_seq_cst);
                m_consumerSleeping.store(true, std::memory_order_seq_cst);
                if (m_published.load(std::memory_order_seq_cst) == m_readSeq && !m_closed.load(std::memory_order_seq_cst))
                    m_consumerEpoch.wait(epoch, std::memory_order_seq_cst);
                m_consumerSleeping.store(false, std::memory_order_relaxed);
            }
        }

        void release(uint64_t seq, bool frameMark)
        {
            const Slot &s = m_slots[(seq - 1) & m_mask];
            if (s.arenaEnd > m_arenaTail.load(std::memory_order_relaxed))
                m_arenaTail.store(s.arenaEnd, std::memory_order_release);
            if (frameMark)
                m_doneFrames.fetch_add(1, std::memory_order_release);
            m_releasedSeq.store(seq, std::memory_order_seq_cst);
            if (m_producerWaiting.load(std::memory_order_seq_cst))
                wake(m_producerEpoch);
        }

        void close()
        {
            m_closed.store(true, std::memory_order_seq_cst);
            wake(m_consumerEpoch);
            wake(m_producerEpoch);
        }

    private:
        struct Slot
        {
            Kind kind = Kind::FrameMark;
            uint32_t a = 0;
            uint64_t b = 0;
            const uint8_t *data = nullptr;
            uint32_t size = 0;
            uint64_t arenaEnd = 0;   // absolute arena offset this payload ends at (released in order)
            std::unique_ptr<uint8_t[]> heap;
        };

        static size_t roundPow2(size_t n)
        {
            size_t p = 1;
            while (p < n)
                p <<= 1;
            return p;
        }

        static void wake(std::atomic<uint32_t> &epoch)
        {
            epoch.fetch_add(1, std::memory_order_seq_cst);
            epoch.notify_all();
        }

        static void cpuRelax()
        {
#if defined(__aarch64__) || defined(_M_ARM64)
            __asm__ __volatile__("yield");
#elif defined(__x86_64__) || defined(_M_X64)
            _mm_pause();
#endif
        }

        // The producer's only wait: publishes first (the worker must see what it is waited on), then sleeps on the
        // released index until `ready` or close.
        template <typename Ready>
        void waitProducer(Ready ready)
        {
            if (ready() || m_closed.load(std::memory_order_acquire))
                return;
            publish();
            for (int i = 0; i < 256; ++i)
            {
                if (ready() || m_closed.load(std::memory_order_acquire))
                    return;
                cpuRelax();
            }
            for (;;)
            {
                const uint32_t epoch = m_producerEpoch.load(std::memory_order_seq_cst);
                m_producerWaiting.store(true, std::memory_order_seq_cst);
                if (ready() || m_closed.load(std::memory_order_seq_cst))
                    break;
                m_producerEpoch.wait(epoch, std::memory_order_seq_cst);
            }
            m_producerWaiting.store(false, std::memory_order_relaxed);
        }

        std::vector<Slot> m_slots;
        const size_t m_mask;
        std::unique_ptr<uint8_t[]> m_arena;
        const size_t m_arenaCap;

        // producer-only
        uint64_t m_writeSeq = 0;
        uint64_t m_arenaHead = 0;
        uint64_t m_pushedFrames = 0;
        // consumer-only
        uint64_t m_readSeq = 0;
        // shared
        alignas(64) std::atomic<uint64_t> m_published{0};
        alignas(64) std::atomic<uint64_t> m_releasedSeq{0};
        alignas(64) std::atomic<uint64_t> m_arenaTail{0};
        alignas(64) std::atomic<uint64_t> m_doneFrames{0};
        alignas(64) std::atomic<bool> m_consumerSleeping{false};
        alignas(64) std::atomic<bool> m_producerWaiting{false};
        alignas(64) std::atomic<uint32_t> m_consumerEpoch{0};
        alignas(64) std::atomic<uint32_t> m_producerEpoch{0};
        std::atomic<bool> m_closed{false};
    };
}
