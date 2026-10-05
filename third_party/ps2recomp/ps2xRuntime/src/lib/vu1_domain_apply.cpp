// macOS fork, the VU1 worker: the executor. One posted item -> the exact public PS2Memory / GS / GifArbiter call the
// game thread used to make inline (plan Part 2 Task 2). Runs on the worker, where vu1domain::shouldPost() is false,
// so the guarded entry points execute instead of posting again.
#include "runtime/vu1_domain_apply.h"

#include "runtime/gs/gs_frontend.h"
#include "runtime/gs/ps2_gif_arbiter.h"
#include "runtime/ps2_memory.h"
#include "runtime/vu1_domain.h"

#include <atomic>

bool ps2xVif1IsStalled();   // ps2_vif1_interpreter.cpp

namespace vu1domain
{
    thread_local uint64_t t_currentItemFbrst = 0;
    // Diagnostic (Task 7 bisect): items other than the i-bit handler's own (PATH3, the arbiter drain, VIF1 register
    // writes) applied while VIF1 is stalled at an i-bit. In the console's order there are none.
    std::atomic<uint64_t> g_ibitForeignItems{0}, g_ibitStalledItems{0};
    uint64_t ibitForeignItems() { return g_ibitForeignItems.load(); }
    uint64_t ibitStalledItems() { return g_ibitStalledItems.load(); }

    uint64_t currentItemFbrst() { return t_currentItemFbrst; }

    void applyItem(const vu1work::WorkItem &item, void *ctx)
    {
        auto *target = static_cast<ApplyTarget *>(ctx);
        if (ps2xVif1IsStalled())
        {
            g_ibitStalledItems.fetch_add(1, std::memory_order_relaxed);
            if (item.kind != vu1work::Kind::GifPath3 && item.kind != vu1work::Kind::ArbiterDrain &&
                item.kind != vu1work::Kind::Vif1Reg)
                g_ibitForeignItems.fetch_add(1, std::memory_order_relaxed);
        }
        const uint32_t n = static_cast<uint32_t>(item.bytes.size());
        switch (item.kind)
        {
        case vu1work::Kind::Vif1Data:
            t_currentItemFbrst = item.b;
            target->memory->processVIF1Data(item.bytes.data(), n);
            break;
        case vu1work::Kind::GifPath3:
            target->memory->submitGifPacket(GifPathId::Path3, item.bytes.data(), n, false);
            break;
        case vu1work::Kind::ArbiterDrain:
            if (target->arbiter)
                target->arbiter->drain();
            break;
        case vu1work::Kind::Vif1Reg:
            target->memory->writeIORegister(item.a, static_cast<uint32_t>(item.b));
            break;
        case vu1work::Kind::GsPriv32:
            target->memory->write32(item.a, static_cast<uint32_t>(item.b));
            break;
        case vu1work::Kind::GsPriv64:
            target->memory->write64(item.a, item.b);
            break;
        case vu1work::Kind::GsReg:
            target->gs->writeRegister(static_cast<uint8_t>(item.a), item.b);
            break;
        case vu1work::Kind::ClearFb:
            target->gs->clearFramebufferContext(item.a, static_cast<uint32_t>(item.b));
            break;
        case vu1work::Kind::GuestFrameBoundary:
            target->gs->guestFrameBoundary();
            break;
        case vu1work::Kind::FrameMark:
            break;
        }
    }

    void gsWriteRegister(GS &gs, uint8_t reg, uint64_t value)
    {
        if (!shouldPost())
        {
            gs.writeRegister(reg, value);
            return;
        }
        vu1work::WorkItem it;
        it.kind = vu1work::Kind::GsReg;
        it.a = reg;
        it.b = value;
        post(std::move(it));
    }

    bool gsClearFramebufferContext(GS &gs, uint32_t context, uint32_t rgba)
    {
        if (!shouldPost())
            return gs.clearFramebufferContext(context, rgba);
        vu1work::WorkItem it;
        it.kind = vu1work::Kind::ClearFb;
        it.a = context;
        it.b = rgba;
        post(std::move(it));
        return true;
    }

    bool postGuestFrameBoundary()
    {
        if (!shouldPost())
            return false;
        vu1work::WorkItem it;
        it.kind = vu1work::Kind::GuestFrameBoundary;
        post(std::move(it));
        return true;
    }

    void gsPrivWrite64(uint64_t &field, uint32_t address, uint64_t value)
    {
        if (!shouldPost())
        {
            field = value;
            return;
        }
        vu1work::WorkItem it;
        it.kind = vu1work::Kind::GsPriv64;
        it.a = address;
        it.b = value;
        post(std::move(it));
    }
}
