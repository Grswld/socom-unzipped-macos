// macOS fork, the VU1 worker: the VIF1 i-bit scanner (runtime/vif1_scanner.h) against the real interpreter
// (PS2Memory::processVIF1Data). The scanner must decode exactly the i-bits the interpreter decodes, at the same calls,
// and agree on the stall, for every stream and every way of cutting it into chunks.
#include "MiniTest.h"
#include "runtime/ps2_memory.h"
#include "runtime/vif1_scanner.h"

#include <cstdint>
#include <cstring>
#include <random>
#include <vector>

bool ps2xVif1IsStalled();   // ps2_vif1_interpreter.cpp

namespace
{
    void put32(std::vector<uint8_t> &s, uint32_t v)
    {
        const size_t at = s.size();
        s.resize(at + 4);
        std::memcpy(s.data() + at, &v, 4);
    }
    uint32_t code(uint8_t op, uint8_t num, uint16_t imm, bool irq)
    {
        return (irq ? 0x80000000u : 0u) | (uint32_t(op & 0x7F) << 24) | (uint32_t(num) << 16) | imm;
    }

    // One random VIF1 stream of `cmds` commands. cycle tracks STCYCL so the UNPACK data lengths are right for the
    // shapes it emits (the scanner and the interpreter must agree on the wrong ones too: lengths come from the code).
    std::vector<uint8_t> randomStream(std::mt19937 &rng, int cmds)
    {
        std::vector<uint8_t> s;
        auto r = [&](uint32_t n) { return static_cast<uint32_t>(rng() % n); };
        uint32_t cl = 1, wl = 1;
        for (int i = 0; i < cmds; ++i)
        {
            const bool irq = r(10) == 0;
            switch (r(11))
            {
            case 0: put32(s, code(0x00, 0, 0, irq)); break;                                           // NOP
            case 1: { cl = 1 + r(4); wl = 1 + r(4); put32(s, code(0x01, 0, uint16_t(cl | (wl << 8)), irq)); break; } // STCYCL
            case 2: put32(s, code(0x14, 0, uint16_t(r(64)), irq)); break;                             // MSCAL (no callback: no-op)
            case 3: put32(s, code(0x20, 0, 0, irq)); put32(s, rng()); break;                          // STMASK
            case 4: put32(s, code(0x30, 0, 0, irq)); for (int k = 0; k < 4; ++k) put32(s, rng()); break; // STROW
            case 5: { const uint32_t n = 1 + r(4); put32(s, code(0x4A, uint8_t(n), 0, irq)); for (uint32_t k = 0; k < n * 2; ++k) put32(s, rng()); break; } // MPG
            case 6: { // DIRECT with one QW of A+D GIF tag + payload, sometimes an IMAGE tag whose data runs past it
                const bool image = r(3) == 0;
                const uint32_t qw = 1 + r(3);
                put32(s, code(0x50, 0, uint16_t(qw), irq));
                // pad the stream to 16-byte alignment is not required by the interpreter: emit raw QWs
                uint64_t tag = image ? (uint64_t(2) << 58) | (1 + r(6)) : (uint64_t(1) << 60) | (uint64_t(0xE) << 0) | 1u;
                put32(s, uint32_t(tag)); put32(s, uint32_t(tag >> 32)); put32(s, 0x0Eu); put32(s, 0u);
                for (uint32_t k = 1; k < qw; ++k) for (int w = 0; w < 4; ++w) put32(s, rng());
                if (image) for (uint32_t k = 0; k < 6; ++k) for (int w = 0; w < 4; ++w) put32(s, rng()); // image QWs
                break;
            }
            default: { // UNPACK V4-32 / V3-16 / S-8 with a small num
                const uint8_t shapes[] = {0x6C, 0x69, 0x62, 0x6D};
                const uint8_t op = shapes[r(4)];
                const uint32_t num = 1 + r(8);
                put32(s, code(op, uint8_t(num), uint16_t(r(512)), irq));
                const uint32_t vn = (op >> 2) & 3, vl = op & 3;
                const uint32_t bits = (vl == 3 && vn == 3) ? 16 : (vn + 1) * (vl == 0 ? 32 : vl == 1 ? 16 : vl == 2 ? 8 : (vn == 3 ? 4 : 16));
                const uint32_t bpv = (bits + 7) / 8;
                uint32_t src = num;
                if (cl < wl) { src = (num / wl) * cl + std::min(num % wl, cl); }
                const uint32_t bytes = (src * bpv + 3) & ~3u;
                for (uint32_t k = 0; k < bytes / 4; ++k) put32(s, rng());
                break;
            }
            }
        }
        return s;
    }
}

void register_vif1_scanner_tests()
{
    MiniTest::Case("Vif1Scanner", [](TestCase &tc)
    {
        tc.Run("a hand-built stream: one i-bit stalls, the rest waits for STC, the second fires at STC", [](TestCase &t)
        {
            std::vector<uint8_t> s;
            put32(s, code(0x00, 0, 0, true));    // NOP with the i-bit
            put32(s, code(0x00, 0, 0, false));
            put32(s, code(0x00, 0, 0, true));    // a second i-bit, held behind the stall
            Vif1Scanner sc;
            t.Equals(sc.scan(s.data(), uint32_t(s.size())), 1u, "one i-bit decoded");
            t.IsTrue(sc.stalled(), "stalled with the rest held");
            t.Equals(sc.stc(), 1u, "the held i-bit fires at STC");
            t.IsTrue(sc.stalled(), "and stalls again");
            t.Equals(sc.stc(), 0u, "nothing left");
            t.IsFalse(sc.stalled(), "running");
        });

        tc.Run("1,000 random streams in random chunks agree with the interpreter call by call", [](TestCase &t)
        {
            std::mt19937 rng(20261005u);
            int mismatches = 0;
            std::string first;
            for (int caseNo = 0; caseNo < 1000 && mismatches == 0; ++caseNo)
            {
                PS2Memory mem;
                mem.initialize();
                (void)mem.consumePendingIntcCauses();
                Vif1Scanner sc;
                // the interpreter's stall state is a process global: start each case from a VIF1 reset
                mem.writeIORegister(0x10003C10u, 0x1u);
                sc.reset();
                const std::vector<uint8_t> stream = randomStream(rng, 1 + int(rng() % 40));
                size_t at = 0;
                int step = 0;
                while (at < stream.size() && mismatches == 0)
                {
                    const size_t len = std::min<size_t>(stream.size() - at, 4 * (1 + rng() % 24));
                    mem.processVIF1Data(stream.data() + at, uint32_t(len));
                    const unsigned scanned = sc.scan(stream.data() + at, uint32_t(len));
                    at += len;
                    auto check = [&](const char *what) {
                        size_t intc = 0;
                        for (uint32_t c : mem.consumePendingIntcCauses()) intc += (c == 5u);
                        if (intc != scanned || ps2xVif1IsStalled() != sc.stalled())
                        {
                            ++mismatches;
                            first = "case " + std::to_string(caseNo) + " step " + std::to_string(step) + " (" + what +
                                    "): interpreter intc=" + std::to_string(intc) + " stalled=" +
                                    std::to_string(ps2xVif1IsStalled()) + ", scanner " + std::to_string(scanned) +
                                    " stalled=" + std::to_string(sc.stalled());
                        }
                    };
                    check("scan");
                    ++step;
                    if (mismatches == 0 && rng() % 5 == 0)   // the EE writes CYCLE directly (0x10003C40)
                    {
                        const uint32_t cyc = (1 + rng() % 4) | ((1 + rng() % 4) << 8);
                        mem.writeIORegister(0x10003C40u, cyc);
                        sc.setCycle(uint16_t(cyc));
                    }
                    if (mismatches == 0 && sc.stalled() && rng() % 2 == 0)
                    {
                        mem.writeIORegister(0x10003C10u, 0x8u);   // STC
                        const unsigned atStc = sc.stc();
                        size_t intc = 0;
                        for (uint32_t c : mem.consumePendingIntcCauses()) intc += (c == 5u);
                        if (intc != atStc || ps2xVif1IsStalled() != sc.stalled())
                        {
                            ++mismatches;
                            first = "case " + std::to_string(caseNo) + " STC: interpreter intc=" + std::to_string(intc) +
                                    ", scanner " + std::to_string(atStc);
                        }
                    }
                }
            }
            t.Equals(mismatches, 0, first);
            PS2Memory cleanup;   // the interpreter's stall is process-global: leave VIF1 reset for the next suite
            cleanup.initialize();
            cleanup.writeIORegister(0x10003C10u, 0x1u);
        });
    });
}
