#pragma once
// macOS fork, the VU1 worker's Task 0 (docs/superpowers/specs/2026-10-04-vu1-worker-design.md, 3.5 and 5).
// Read-only: how much of the inline VIF1/VU1/GS work ran while another guest thread was ready (the time a worker on
// its own core could overlap), and how the VIF1 i-bit interrupt and its FBRST.STC resume sit in the frame.
// Pure: the window and its report. vu1_overlap_stats.cpp holds the process-wide window, the knob and the probe.
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>

namespace vu1overlap
{
    // What the scheduler says when a VIF1 kick is processed: guest threads in Ready other than the current one
    // (-1: no scheduler), and the current guest thread's id (-1: none).
    struct Probe
    {
        int readyOthers = -1;
        int currentTid = -1;
    };

    // Task 0d: the EE accesses a VU1 worker would have to wait at (spec 3.3), by kind.
    enum class SyncKind : uint8_t { None, Vif1Reg, Dma1Reg, DmaCtl, GifReg, GsPriv, Vu1Mem, Count };

    inline SyncKind syncKindOfIo(uint32_t phys)
    {
        if (phys >= 0x10003C00u && phys < 0x10003E00u)
            return SyncKind::Vif1Reg;   // VIF1 STAT, FBRST, ERR, MARK, CYCLE, MODE, NUM, MASK, CODE, ITOPS, ...
        if (phys >= 0x10009000u && phys < 0x10009100u)
            return SyncKind::Dma1Reg;   // DMA channel 1 (VIF1): CHCR, MADR, QWC, TADR, ASR0/1
        if ((phys >= 0x1000E000u && phys < 0x1000E100u) || (phys >= 0x1000F520u && phys < 0x1000F5A0u))
            return SyncKind::DmaCtl;    // D_CTRL, D_STAT, D_PCR, ...; D_ENABLER/W
        if (phys >= 0x10003000u && phys < 0x10003100u)
            return SyncKind::GifReg;    // GIF CTRL, MODE, STAT, ...
        return SyncKind::None;
    }

    struct Window
    {
        uint64_t vif1Ns = 0;            // host time inside processPendingTransfers' VIF1 loop (VU1 and GS inline)
        uint64_t overlappableNs = 0;    // the part of it that ran while another guest thread was Ready
        uint64_t kicks = 0;
        std::map<int, uint64_t> kicksByTid;
        uint64_t ibits = 0, stcs = 0;
        uint64_t stcLatencyNsTotal = 0, stcLatencyNsMax = 0, stcLatencySamples = 0;
        uint64_t lastIbitNs = 0;
        bool ibitOpen = false;
        uint64_t syncs[int(SyncKind::Count)] = {};

        void noteSync(SyncKind k)
        {
            if (k != SyncKind::None)
                ++syncs[int(k)];
        }

        void addVif1(uint64_t ns, const Probe &p)
        {
            vif1Ns += ns;
            ++kicks;
            if (p.readyOthers > 0)
                overlappableNs += ns;
            if (p.currentTid >= 0)
                ++kicksByTid[p.currentTid];
        }
        void noteIbit(uint64_t nowNs)
        {
            ++ibits;
            lastIbitNs = nowNs;
            ibitOpen = true;
        }
        void noteStc(uint64_t nowNs)
        {
            ++stcs;
            if (!ibitOpen)
                return;
            ibitOpen = false;
            const uint64_t d = nowNs >= lastIbitNs ? nowNs - lastIbitNs : 0u;
            stcLatencyNsTotal += d;
            stcLatencyNsMax = d > stcLatencyNsMax ? d : stcLatencyNsMax;
            ++stcLatencySamples;
        }

        std::string report(double seconds) const
        {
            const double s = seconds > 0.0 ? seconds : 1.0;
            const double vif1 = double(vif1Ns) / 1e6 / s, over = double(overlappableNs) / 1e6 / s;
            const double pct = vif1Ns ? 100.0 * double(overlappableNs) / double(vif1Ns) : 0.0;
            const double mean = stcLatencySamples ? double(stcLatencyNsTotal) / double(stcLatencySamples) / 1e6 : 0.0;
            char buf[256];
            std::snprintf(buf, sizeof buf,
                          "[vu1-overlap] vif1=%.0f ms/s overlappable=%.0f ms/s (%.0f%%) kicks=%.0f/s ibits=%.1f/s "
                          "stc=%.1f/s stc_latency mean=%.2f max=%.2f ms by_tid=",
                          vif1, over, pct, double(kicks) / s, double(ibits) / s, double(stcs) / s, mean,
                          double(stcLatencyNsMax) / 1e6);
            std::string line(buf);
            std::snprintf(buf, sizeof buf, "syncs vif1=%.1f dma1=%.1f dmactl=%.1f gif=%.1f gspriv=%.1f vu1mem=%.1f /s ",
                          double(syncs[int(SyncKind::Vif1Reg)]) / s, double(syncs[int(SyncKind::Dma1Reg)]) / s,
                          double(syncs[int(SyncKind::DmaCtl)]) / s, double(syncs[int(SyncKind::GifReg)]) / s,
                          double(syncs[int(SyncKind::GsPriv)]) / s, double(syncs[int(SyncKind::Vu1Mem)]) / s);
            line.insert(line.rfind("by_tid="), buf);
            bool first = true;
            for (const auto &[tid, n] : kicksByTid)
            {
                line += (first ? "" : ",") + std::to_string(tid) + ":" + std::to_string(n);
                first = false;
            }
            return line;
        }
    };

    // Process-wide (vu1_overlap_stats.cpp): PS2X_VU1_OVERLAP_TRACE, read once. Game thread only.
    using ProbeFn = Probe (*)(void *ctx);
    bool on();
    void setProbe(ProbeFn fn, void *ctx);
    Probe probe();
    uint64_t nowNs();
    void addVif1(uint64_t ns, const Probe &p);   // prints the window once a second
    void ibit();
    void stc();
    void sync(SyncKind k);                       // an EE access a worker would have to wait at (Task 0d)
}
