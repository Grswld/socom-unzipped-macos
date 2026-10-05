// macOS fork, VU1 worker Part 3 (the readback cost): GSCpuBackend::WriteVramRow -- one lock for a row of pixels --
// must leave VRAM byte for byte as the per-pixel WriteVram it replaces in the GL backend's readback.
#include "MiniTest.h"
#include "runtime/gs/gs_cpu_backend.h"
#include "runtime/gs/gs_types.h"

#include <cstring>
#include <random>
#include <vector>

void register_gs_cpu_span_write_tests()
{
    MiniTest::Case("GsCpuSpanWrite", [](TestCase &tc)
    {
        tc.Run("a row write equals per-pixel writes for every render-target format", [](TestCase &t)
        {
            constexpr uint32_t kVram = 4u * 1024u * 1024u;
            std::vector<uint8_t> a(kVram, 0x5A), b(kVram, 0x5A);
            GSCpuBackend pa, pb;
            pa.Initialize(a.data(), kVram);
            pb.Initialize(b.data(), kVram);
            const uint32_t psms[] = {GS_PSM_CT32, GS_PSM_CT24, GS_PSM_CT16, GS_PSM_CT16S,
                                     GS_PSM_Z32, GS_PSM_Z24, GS_PSM_Z16, GS_PSM_Z16S};
            std::mt19937 rng(7u);
            int rows = 0;
            for (uint32_t psm : psms)
                for (int k = 0; k < 40; ++k)
                {
                    const uint32_t base = (rng() % 256u) << 5, bw = 1u + rng() % 10u;
                    const uint32_t y = rng() % 448u, x0 = rng() % 64u, n = 1u + rng() % (bw * 64u - x0);
                    std::vector<uint32_t> px(n);
                    for (auto &p : px)
                        p = rng();
                    for (uint32_t i = 0; i < n; ++i)
                        pa.WriteVram(psm, base, bw, x0 + i, y, px[i]);
                    pb.WriteVramRow(psm, base, bw, x0, y, px.data(), n);
                    ++rows;
                }
            t.IsTrue(rows == 320, "320 rows written");
            t.IsTrue(std::memcmp(a.data(), b.data(), kVram) == 0, "VRAM identical");
        });
    });
}
