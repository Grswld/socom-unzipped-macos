// macOS fork, the VU1 worker's correctness gate (plan Part 2 Task 4): replay a recorded VU1-worker queue
// (PS2X_VU1_QUEUE_RECORD) through a headless PS2Memory + GS (the CPU raster backend) + GIF arbiter + VU1 interpreter,
// either inline or through the real worker (--threaded), and print the VRAM hash at every frame mark. The two runs
// must print the same lines: the queue machinery may not change what the GS draws.
//
//   vu1_queue_replay <file.vq> [--threaded] [--frames N]
#include "runtime/gs/gs_cpu_backend.h"
#include "runtime/gs/gs_frontend.h"
#include "runtime/gs/ps2_gif_arbiter.h"
#include "runtime/ps2_memory.h"
#include "runtime/ps2_vu1.h"
#include "runtime/vu1_domain.h"
#include "runtime/vu1_domain_apply.h"
#include "ps2x/knobs.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace
{
    uint64_t fnv1a64(const uint8_t *p, size_t n)
    {
        uint64_t h = 1469598103934665603ull;
        for (size_t i = 0; i < n; ++i)
            h = (h ^ p[i]) * 1099511628211ull;
        return h;
    }

    struct Replay
    {
        PS2Memory memory;
        GS gs;
        GifArbiter arbiter{[this](const uint8_t *d, uint32_t n) { gs.processGIFPacket(d, n); }};
        VU1Interpreter vu{VU1Interpreter::Unit::VU1};
        vu1domain::ApplyTarget target;
        std::vector<uint64_t> frameHashes;
        size_t maxFrames = SIZE_MAX;
    };
    Replay *g_replay = nullptr;

    // The executor plus the frame hash: applied wherever the item is applied (inline here, or on the worker).
    void applyAndHash(const vu1work::WorkItem &item, void *ctx)
    {
        vu1domain::applyItem(item, ctx);
        if (item.kind == vu1work::Kind::FrameMark && g_replay->frameHashes.size() < g_replay->maxFrames)
            g_replay->frameHashes.push_back(fnv1a64(g_replay->memory.getGSVRAM(), PS2_GS_VRAM_SIZE));
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: vu1_queue_replay <file.vq> [--threaded] [--frames N]\n");
        return 2;
    }
    ps2x::knobs::setDevMode(true);
    bool threaded = false;
    size_t maxFrames = SIZE_MAX;
    for (int i = 2; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--threaded") == 0)
            threaded = true;
        else if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc)
            maxFrames = std::strtoull(argv[++i], nullptr, 10);
    }
    std::ifstream in(argv[1], std::ios::binary);
    if (!in)
    {
        std::fprintf(stderr, "cannot read %s\n", argv[1]);
        return 2;
    }
    std::vector<uint8_t> buf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    auto replay = std::make_unique<Replay>();
    g_replay = replay.get();
    replay->maxFrames = maxFrames;
    if (!replay->memory.initialize())
    {
        std::fprintf(stderr, "PS2Memory::initialize failed\n");
        return 1;
    }
    Replay &r = *replay;
    r.gs.init(r.memory.getGSVRAM(), static_cast<uint32_t>(PS2_GS_VRAM_SIZE), &r.memory.gs());
    r.gs.setRasterBackend(std::make_unique<GSCpuBackend>());
    r.memory.setGifArbiter(&r.arbiter);
    // The runtime's MSCAL / MSCNT wiring (ps2_runtime.cpp), with the D/T enables from the item's captured vu0_fbrst.
    r.memory.setVu1MscalCallback([&r](uint32_t startPC, uint32_t top, uint32_t itop) {
        const uint64_t fbrst = vu1domain::currentItemFbrst();
        r.vu.state().dBitEnabled = (fbrst & (1u << 10)) != 0u;
        r.vu.state().tBitEnabled = (fbrst & (1u << 11)) != 0u;
        r.vu.executeProgram(r.memory.getVU1Code(), PS2_VU1_CODE_SIZE, r.memory.getVU1Data(), PS2_VU1_DATA_SIZE, r.gs,
                            &r.memory, startPC, top, itop);
    });
    r.memory.setVu1MscntCallback([&r](uint32_t top, uint32_t itop) {
        const uint64_t fbrst = vu1domain::currentItemFbrst();
        r.vu.state().dBitEnabled = (fbrst & (1u << 10)) != 0u;
        r.vu.state().tBitEnabled = (fbrst & (1u << 11)) != 0u;
        r.vu.continueProgram(r.memory.getVU1Code(), PS2_VU1_CODE_SIZE, r.memory.getVU1Data(), PS2_VU1_DATA_SIZE, r.gs,
                             &r.memory, top, itop);
    });
    r.target = vu1domain::ApplyTarget{&r.memory, &r.gs, &r.arbiter};

    if (threaded)
    {
        vu1domain::Config cfg;
        cfg.enabled = true;
        cfg.queueFrames = 4;
        vu1domain::start(cfg, applyAndHash, &r.target);
    }
    const uint8_t *p = buf.data(), *end = buf.data() + buf.size();
    uint64_t items = 0;
    vu1work::WorkItem item;
    while (p < end && vu1work::deserialize(p, end, item))
    {
        ++items;
        if (threaded)
            vu1domain::post(std::move(item));
        else
            applyAndHash(item, &r.target);
        item = vu1work::WorkItem{};
    }
    if (threaded)
        vu1domain::stop();
    if (p != end)
        std::fprintf(stderr, "warning: %zu trailing bytes not a whole record\n", static_cast<size_t>(end - p));
    std::fprintf(stderr, "%llu items, %zu frames, %s\n", static_cast<unsigned long long>(items), r.frameHashes.size(),
                 threaded ? "threaded" : "inline");
    for (size_t i = 0; i < r.frameHashes.size(); ++i)
        std::printf("frame %zu vram %016llx\n", i, static_cast<unsigned long long>(r.frameHashes[i]));
    return 0;
}
