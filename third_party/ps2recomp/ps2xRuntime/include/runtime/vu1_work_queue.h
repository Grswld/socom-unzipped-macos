#pragma once
// macOS fork, the VU1 worker (docs/superpowers/specs/2026-10-04-vu1-worker-design.md 3.2; plan Part 2 Task 1).
// One ordered FIFO between the game thread (the only producer) and the VU1 worker (the only consumer): every item is a
// piece of GS-bound work the game thread used to do inline, applied by the worker in exactly the order it was posted.
// Sequence numbers let the producer wait for an item to be applied (a drain); FrameMark items bound how many game frames
// the worker may still be working on (backpressure). serialize/deserialize are the recording format (Task 4).
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>

namespace vu1work
{
    enum class Kind : uint8_t
    {
        Vif1Data,            // bytes: VIF1 DMA data; b: the EE's vu0_fbrst at the kick
        GifPath3,            // bytes: one PATH3 (GIF DMA) chunk
        ArbiterDrain,        // the GIF arbiter's drain after a transfer batch
        Vif1Reg,             // a: a VIF1 register or FIFO address; b: the 32-bit value
        GsPriv32,            // a: a GS privileged register address; b: the value
        GsPriv64,
        GsReg,               // a: a GS register number; b: the value (GS::writeRegister)
        ClearFb,             // a: the context; b: the RGBA (GS::clearFramebufferContext)
        GuestFrameBoundary,  // the GL backend's VBlank frame mark
        FrameMark,           // a game frame (sceGsSyncV): accounting only
    };

    struct WorkItem
    {
        Kind kind = Kind::FrameMark;
        uint32_t a = 0;
        uint64_t b = 0;
        std::vector<uint8_t> bytes;
    };

    class WorkQueue
    {
    public:
        uint64_t push(WorkItem &&item)
        {
            uint64_t seq;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                seq = ++m_nextSeq;
                if (item.kind == Kind::FrameMark)
                    ++m_pushedFrames;
                m_queue.emplace_back(seq, std::move(item));
            }
            m_items.notify_one();
            return seq;
        }

        bool pop(WorkItem &out, uint64_t &seq)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_items.wait(lock, [&] { return !m_queue.empty() || m_closed; });
            if (m_queue.empty())
                return false;
            seq = m_queue.front().first;
            out = std::move(m_queue.front().second);
            m_queue.pop_front();
            return true;
        }

        void markDone(uint64_t seq, bool frameMark)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (seq > m_doneSeq)
                    m_doneSeq = seq;
                if (frameMark)
                    ++m_doneFrames;
            }
            m_progress.notify_all();
        }

        void waitDone(uint64_t seq)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_progress.wait(lock, [&] { return m_doneSeq >= seq || m_closed; });
        }

        uint64_t lastPushed() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_nextSeq;
        }

        void waitFramesAhead(unsigned limit)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_progress.wait(lock, [&] { return m_pushedFrames - m_doneFrames <= limit || m_closed; });
        }

        unsigned framesAhead() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return static_cast<unsigned>(m_pushedFrames - m_doneFrames);
        }

        void close()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_closed = true;
            }
            m_items.notify_all();
            m_progress.notify_all();
        }

    private:
        mutable std::mutex m_mutex;
        std::condition_variable m_items;      // the consumer waits here
        std::condition_variable m_progress;   // the producer's waitDone / waitFramesAhead
        std::deque<std::pair<uint64_t, WorkItem>> m_queue;
        uint64_t m_nextSeq = 0, m_doneSeq = 0, m_pushedFrames = 0, m_doneFrames = 0;
        bool m_closed = false;
    };

    // The recording format, little-endian: u8 kind, u32 a, u64 b, u32 n, n bytes.
    inline void serialize(const WorkItem &item, std::vector<uint8_t> &out)
    {
        const uint32_t n = static_cast<uint32_t>(item.bytes.size());
        const size_t at = out.size();
        out.resize(at + 1 + 4 + 8 + 4 + n);
        uint8_t *p = out.data() + at;
        *p++ = static_cast<uint8_t>(item.kind);
        std::memcpy(p, &item.a, 4); p += 4;
        std::memcpy(p, &item.b, 8); p += 8;
        std::memcpy(p, &n, 4); p += 4;
        if (n)
            std::memcpy(p, item.bytes.data(), n);
    }

    inline bool deserialize(const uint8_t *&p, const uint8_t *end, WorkItem &item)
    {
        if (end - p < 17)
            return false;
        const uint8_t kind = p[0];
        if (kind > static_cast<uint8_t>(Kind::FrameMark))
            return false;
        uint32_t a, n;
        uint64_t b;
        std::memcpy(&a, p + 1, 4);
        std::memcpy(&b, p + 5, 8);
        std::memcpy(&n, p + 13, 4);
        if (static_cast<uint64_t>(end - p) < 17u + n)
            return false;
        item.kind = static_cast<Kind>(kind);
        item.a = a;
        item.b = b;
        item.bytes.assign(p + 17, p + 17 + n);
        p += 17 + n;
        return true;
    }
}
