#pragma once
// macOS fork, the VU1 worker: the executor's interface (vu1_domain_apply.cpp).
#include "runtime/vu1_work_queue.h"

#include <cstdint>

class PS2Memory;
class GS;
class GifArbiter;

namespace vu1domain
{
    struct ApplyTarget
    {
        PS2Memory *memory = nullptr;
        GS *gs = nullptr;
        GifArbiter *arbiter = nullptr;
    };
    void applyItem(const vu1work::WorkItem &item, void *ctx);   // ctx: ApplyTarget*
    uint64_t currentItemFbrst();

    // The stubs' direct GS calls (GS.cpp, Support.h): posted while the worker is on, called through otherwise.
    void gsWriteRegister(GS &gs, uint8_t reg, uint64_t value);
    bool gsClearFramebufferContext(GS &gs, uint32_t context, uint32_t rgba);
    // EeScheduler's VBlank frame mark: true when it was posted (the caller then skips its own call and its wait).
    bool postGuestFrameBoundary();
    // A GS privileged display register (PMODE, SMODE2, DISPFB, DISPLAY, BGCOLOR) written by a stub: in place with the
    // worker off (today's behaviour), posted in order with the GS work while it is on -- the display must not switch
    // to a buffer the worker has not finished drawing (Task 7: flicker and colour distortion with the worker on).
    void gsPrivWrite64(uint64_t &field, uint32_t address, uint64_t value);   // on the worker: the EE's vu0_fbrst captured with the Vif1Data item being applied
}
