// macOS fork, the VU1 worker Part 2 Task 3: the game-thread guards post GS-bound work, which the executor applies on
// the worker, against a real PS2Memory (the VIF1 DMA DIRECT shapes of ps2_memory_tests.cpp).
#include "MiniTest.h"
#include "runtime/gs/gs_frontend.h"
#include "runtime/ps2_memory.h"
#include "runtime/vu1_domain.h"
#include "runtime/vu1_domain_apply.h"

#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

namespace
{
    constexpr uint32_t kVif1Ch = 0x10009000u;

    uint32_t vifCmd(uint8_t opcode, uint8_t num, uint16_t imm)
    {
        return (static_cast<uint32_t>(opcode) << 24) | (static_cast<uint32_t>(num) << 16) | imm;
    }
    uint64_t dmaTag(uint16_t qwc, uint8_t id, uint32_t addr)
    {
        return static_cast<uint64_t>(qwc) | (static_cast<uint64_t>(id & 7u) << 28) | (static_cast<uint64_t>(addr) << 32);
    }

    struct Capture
    {
        std::mutex mutex;
        std::vector<std::vector<uint8_t>> packets;
        std::vector<std::thread::id> threads;
    };

    // A normal-mode VIF1 DMA of one DIRECT QW at kSrc: the GIF callback sees 16 bytes 0x11.. .
    void kickDirect(PS2Memory &mem, uint32_t src)
    {
        uint8_t *rdram = mem.getRDRAM();
        std::memset(rdram + src, 0, 32u);
        const uint32_t cmd = vifCmd(0x50u, 0u, 1u);
        std::memcpy(rdram + src, &cmd, sizeof(cmd));
        for (uint32_t i = 0; i < 16u; ++i)
            rdram[src + 4u + i] = static_cast<uint8_t>(0x11u + i);
        mem.writeIORegister(kVif1Ch + 0x10u, src);
        mem.writeIORegister(kVif1Ch + 0x20u, 2u);
        mem.writeIORegister(kVif1Ch + 0x00u, 0x100u);
    }

    vu1domain::Config on()
    {
        vu1domain::Config c;
        c.enabled = true;
        return c;
    }

    bool isDirectPayload(const std::vector<uint8_t> &p, uint8_t first)
    {
        if (p.size() != 16u)
            return false;
        for (uint32_t i = 0; i < 16u; ++i)
            if (p[i] != static_cast<uint8_t>(first + i))
                return false;
        return true;
    }
}

void register_vu1_domain_guard_tests()
{
    MiniTest::Case("Vu1DomainGuards", [](TestCase &tc)
    {
        tc.Run("a posted normal-mode VIF1 kick owns its bytes, and the DMA completes at once", [](TestCase &t)
        {
            PS2Memory mem;
            t.IsTrue(mem.initialize(), "initialize");
            Capture cap;
            mem.setGifPacketCallback([&](const uint8_t *d, uint32_t n) {
                std::lock_guard<std::mutex> lock(cap.mutex);
                cap.packets.emplace_back(d, d + n);
                cap.threads.push_back(std::this_thread::get_id());
            });
            vu1domain::ApplyTarget target{&mem, nullptr, nullptr};
            vu1domain::start(on(), vu1domain::applyItem, &target);
            kickDirect(mem, 0x24000u);
            t.IsTrue((mem.readIORegister(kVif1Ch + 0x00u) & 0x100u) == 0u, "STR clear right after the kick");
            std::memset(mem.getRDRAM() + 0x24000u, 0xEE, 32u);   // the game frees and reuses the packet
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            vu1domain::stop();
            t.Equals(cap.packets.size(), size_t(1), "one GIF packet");
            t.IsTrue(!cap.packets.empty() && isDirectPayload(cap.packets[0], 0x11u), "the bytes as they were at the kick");
            t.IsTrue(!cap.threads.empty() && cap.threads[0] != std::this_thread::get_id(), "delivered on the worker");
        });

        tc.Run("a posted chain kick carries its chain data", [](TestCase &t)
        {
            PS2Memory mem;
            t.IsTrue(mem.initialize(), "initialize");
            Capture cap;
            mem.setGifPacketCallback([&](const uint8_t *d, uint32_t n) {
                std::lock_guard<std::mutex> lock(cap.mutex);
                cap.packets.emplace_back(d, d + n);
                cap.threads.push_back(std::this_thread::get_id());
            });
            constexpr uint32_t kTag = 0x25000u;
            uint8_t *rdram = mem.getRDRAM();
            std::memset(rdram + kTag, 0, 32u);
            const uint64_t endTag = dmaTag(1u, 7u, 0u);
            std::memcpy(rdram + kTag, &endTag, sizeof(endTag));
            const uint32_t directCmd = vifCmd(0x50u, 0u, 1u);
            std::memcpy(rdram + kTag + 12u, &directCmd, sizeof(directCmd));
            for (uint32_t i = 0; i < 16u; ++i)
                rdram[kTag + 16u + i] = static_cast<uint8_t>(0x70u + i);
            vu1domain::ApplyTarget target{&mem, nullptr, nullptr};
            vu1domain::start(on(), vu1domain::applyItem, &target);
            mem.writeIORegister(kVif1Ch + 0x30u, kTag);
            mem.writeIORegister(kVif1Ch + 0x00u, 0x144u);
            std::memset(rdram + kTag, 0, 32u);
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            vu1domain::stop();
            t.Equals(cap.packets.size(), size_t(1), "one GIF packet");
            t.IsTrue(!cap.packets.empty() && isDirectPayload(cap.packets[0], 0x70u), "the chain's payload");
            t.IsTrue(!cap.threads.empty() && cap.threads[0] != std::this_thread::get_id(), "delivered on the worker");
        });

        tc.Run("on and off: the same GIF packet and the same DMA registers", [](TestCase &t)
        {
            auto run = [](bool worker, std::vector<std::vector<uint8_t>> &packets, uint32_t &chcr, uint32_t &qwc) {
                PS2Memory mem;
                mem.initialize();
                Capture cap;
                mem.setGifPacketCallback([&](const uint8_t *d, uint32_t n) {
                    std::lock_guard<std::mutex> lock(cap.mutex);
                    cap.packets.emplace_back(d, d + n);
                });
                vu1domain::ApplyTarget target{&mem, nullptr, nullptr};
                if (worker)
                    vu1domain::start(on(), vu1domain::applyItem, &target);
                kickDirect(mem, 0x24000u);
                if (worker)
                {
                    vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
                    vu1domain::stop();
                }
                packets = cap.packets;
                chcr = mem.readIORegister(kVif1Ch + 0x00u);
                qwc = mem.readIORegister(kVif1Ch + 0x20u);
            };
            std::vector<std::vector<uint8_t>> off, onP;
            uint32_t chcrOff = 0, qwcOff = 0, chcrOn = 0, qwcOn = 0;
            run(false, off, chcrOff, qwcOff);
            run(true, onP, chcrOn, qwcOn);
            t.IsTrue(off == onP, "identical GIF packets");
            t.Equals(chcrOn, chcrOff, "identical CHCR");
            t.Equals(qwcOn, qwcOff, "identical QWC");
        });

        tc.Run("a VIF1 register write is posted, and applied it lands", [](TestCase &t)
        {
            static std::vector<vu1work::WorkItem> s_seen;
            s_seen.clear();
            PS2Memory mem;
            t.IsTrue(mem.initialize(), "initialize");
            vu1domain::ApplyTarget target{&mem, nullptr, nullptr};
            vu1domain::start(on(), [](const vu1work::WorkItem &it, void *ctx) {
                s_seen.push_back(it);
                vu1domain::applyItem(it, ctx);
            }, &target);
            mem.writeIORegister(0x10003C30u, 0x1234u);   // VIF1 MARK
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            vu1domain::stop();
            t.IsTrue(s_seen.size() == 1 && s_seen[0].kind == vu1work::Kind::Vif1Reg && s_seen[0].a == 0x10003C30u &&
                         s_seen[0].b == 0x1234u, "posted as one Vif1Reg item");
            t.Equals(mem.readIORegister(0x10003C30u) & 0xFFFFu, 0x1234u, "MARK written");
        });
    });

    MiniTest::Case("Vu1DomainStubHelpers", [](TestCase &tc)
    {
        tc.Run("GS register writes, clears and the VBlank frame mark are posted while the worker is on", [](TestCase &t)
        {
            static std::vector<vu1work::WorkItem> s_seen;
            static std::mutex s_mutex;
            s_seen.clear();
            vu1domain::start(on(), [](const vu1work::WorkItem &it, void *) {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_seen.push_back(it);
            }, nullptr);
            GS gs;   // never touched while posting
            vu1domain::gsWriteRegister(gs, 0x47u, 0x30000ull);
            t.IsTrue(vu1domain::gsClearFramebufferContext(gs, 1u, 0x80402010u), "a posted clear reports success");
            t.IsTrue(vu1domain::postGuestFrameBoundary(), "the frame mark was posted");
            vu1domain::drain(vu1overlap::SyncKind::Vif1Reg);
            vu1domain::stop();
            std::lock_guard<std::mutex> lock(s_mutex);
            t.IsTrue(s_seen.size() == 3, "three items");
            t.IsTrue(s_seen.size() == 3 && s_seen[0].kind == vu1work::Kind::GsReg && s_seen[0].a == 0x47u && s_seen[0].b == 0x30000ull, "GsReg");
            t.IsTrue(s_seen.size() == 3 && s_seen[1].kind == vu1work::Kind::ClearFb && s_seen[1].a == 1u && s_seen[1].b == 0x80402010ull, "ClearFb");
            t.IsTrue(s_seen.size() == 3 && s_seen[2].kind == vu1work::Kind::GuestFrameBoundary, "GuestFrameBoundary");
        });

        tc.Run("off: the frame mark is not posted (the caller runs today's path)", [](TestCase &t)
        {
            t.IsFalse(vu1domain::postGuestFrameBoundary(), "not posted");
        });
    });

    MiniTest::Case("Vu1DomainDrains", [](TestCase &tc)
    {
        // A slow executor: every item takes 20 ms, so a read that does not drain sees the old state.
        static vu1domain::ApplyTarget *s_target = nullptr;
        auto slowApply = [](const vu1work::WorkItem &it, void *ctx) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            vu1domain::applyItem(it, ctx);
        };

        tc.Run("a VIF1 register read drains first", [=](TestCase &t)
        {
            PS2Memory mem;
            t.IsTrue(mem.initialize(), "initialize");
            vu1domain::ApplyTarget target{&mem, nullptr, nullptr};
            s_target = &target;
            vu1domain::start(on(), slowApply, &target);
            const uint64_t before = vu1domain::drainCount(vu1overlap::SyncKind::Vif1Reg);
            mem.writeIORegister(0x10003C30u, 0x4321u);   // VIF1 MARK, posted
            const uint32_t mark = mem.readIORegister(0x10003C30u) & 0xFFFFu;
            vu1domain::stop();
            t.Equals(mark, 0x4321u, "the read saw the posted write");
            t.Equals(vu1domain::drainCount(vu1overlap::SyncKind::Vif1Reg), before + 1, "one VIF1 drain");
        });

        tc.Run("a DMA-1 register read does not drain", [=](TestCase &t)
        {
            PS2Memory mem;
            t.IsTrue(mem.initialize(), "initialize");
            vu1domain::ApplyTarget target{&mem, nullptr, nullptr};
            vu1domain::start(on(), slowApply, &target);
            mem.writeIORegister(0x10003C30u, 0x1111u);   // pending on the slow worker
            const uint64_t drains = vu1domain::drainCount(vu1overlap::SyncKind::Dma1Reg);
            (void)mem.readIORegister(0x10009000u);       // D1 CHCR
            t.Equals(vu1domain::drainCount(vu1overlap::SyncKind::Dma1Reg), drains, "no DMA-1 drain");
            vu1domain::stop();
        });

        tc.Run("a D_STAT read and a GIF register read drain", [=](TestCase &t)
        {
            PS2Memory mem;
            t.IsTrue(mem.initialize(), "initialize");
            vu1domain::ApplyTarget target{&mem, nullptr, nullptr};
            vu1domain::start(on(), slowApply, &target);
            const uint64_t d0 = vu1domain::drainCount(vu1overlap::SyncKind::DmaCtl);
            const uint64_t g0 = vu1domain::drainCount(vu1overlap::SyncKind::GifReg);
            (void)mem.readIORegister(0x1000E010u);   // D_STAT
            (void)mem.readIORegister(0x10003020u);   // GIF STAT
            vu1domain::stop();
            t.Equals(vu1domain::drainCount(vu1overlap::SyncKind::DmaCtl), d0 + 1, "D_STAT drained");
            t.Equals(vu1domain::drainCount(vu1overlap::SyncKind::GifReg), g0 + 1, "GIF drained");
        });
    });
}
