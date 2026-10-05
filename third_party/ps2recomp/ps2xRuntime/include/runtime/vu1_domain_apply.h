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
    uint64_t currentItemFbrst();   // on the worker: the EE's vu0_fbrst captured with the Vif1Data item being applied
}
