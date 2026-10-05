#pragma once
// macOS fork, the VU1 worker: the VIF1 i-bit scanner. Task 7 found the i-bit interrupt raised late on the worker (the
// game queued the next frame's drawing ahead of its INTC5 handler's texture uploads: flicker and colour distortion), so
// with the worker on the game thread raises INTC5 itself, from this scanner, at the very call where the interpreter
// would have: at the VIF1 kick, and at FBRST.STC for i-bits held behind a stall.
//
// It walks the bytes exactly as PS2Memory::processVIF1Data does (ps2_vif1_interpreter.cpp): the same loop, the same
// per-command lengths (STCYCL's CL/WL for UNPACK, MPG's num*8, DIRECT's qw and the GIF IMAGE remainder carried across
// calls), the same stall after an i-bit command and the same hold until STC -- and executes nothing. Pure.
// vif1_scanner_tests.cpp holds it to the interpreter call by call on 1,000 random streams.
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

class Vif1Scanner
{
public:
    // One processVIF1Data call's worth of bytes; returns the i-bit commands decoded (each one INTC5).
    unsigned scan(const uint8_t *data, uint32_t sizeBytes)
    {
        if (sizeBytes == 0u)
            return 0u;
        if (m_stalled)
        {
            m_held.insert(m_held.end(), data, data + sizeBytes);
            return 0u;
        }
        unsigned irqs = 0;
        uint32_t pos = 0;
        while (pos + 4 <= sizeBytes)
        {
            if (m_irqPending && m_pendingImageQwc == 0u)
            {
                m_irqPending = false;
                m_stalled = true;
                m_held.assign(data + pos, data + sizeBytes);
                return irqs;
            }
            if (m_pendingImageQwc != 0u)
            {
                const uint32_t availableQw = (sizeBytes - pos) / 16u;
                if (availableQw == 0u)
                    break;
                const uint32_t chunkQw = std::min<uint32_t>(m_pendingImageQwc, availableQw);
                pos += chunkQw * 16u;
                m_pendingImageQwc -= chunkQw;
                continue;
            }
            uint32_t cmd;
            std::memcpy(&cmd, data + pos, 4);
            pos += 4;
            const uint8_t opcode = (cmd >> 24) & 0x7F;
            const uint16_t imm = cmd & 0xFFFF;
            const uint8_t num = (cmd >> 16) & 0xFF;
            if (cmd & 0x80000000u)
            {
                ++irqs;
                if (!m_noIrqStall)
                    m_irqPending = true;
            }
            if (opcode == 0x01)   // STCYCL
            {
                m_cycle = imm;
                continue;
            }
            if (opcode == 0x20)   // STMASK
            {
                if (pos + 4 > sizeBytes)
                    break;
                pos += 4;
                continue;
            }
            if (opcode == 0x30 || opcode == 0x31)   // STROW, STCOL
            {
                if (pos + 16 > sizeBytes)
                    break;
                pos += 16;
                continue;
            }
            if (opcode == 0x4A)   // MPG
            {
                pos += ((num == 0u) ? 256u : uint32_t(num)) * 8u;
                if (pos > sizeBytes)
                    break;
                continue;
            }
            if (opcode == 0x50 || opcode == 0x51)   // DIRECT, DIRECTHL
            {
                uint32_t qwCount = imm ? imm : 65536u;
                const uint32_t availableQw = (sizeBytes - pos) / 16u;
                const bool truncated = qwCount > availableQw;
                if (truncated)
                    qwCount = availableQw;
                if (qwCount > 0)
                {
                    const uint32_t imageQw = imageQwcFromTag(data + pos, qwCount * 16u);
                    if (imageQw != 0u && imageQw > qwCount - 1u)
                        m_pendingImageQwc = imageQw - (qwCount - 1u);
                }
                pos += qwCount * 16u;
                if (truncated)
                {
                    pos = sizeBytes;
                    break;
                }
                continue;
            }
            if ((opcode & 0x60) == 0x60)   // UNPACK
            {
                const uint8_t vn = (opcode >> 2) & 0x3, vl = opcode & 0x3;
                int bitsPerComponent = vl == 0 ? 32 : vl == 1 ? 16 : vl == 2 ? 8 : (vn == 3 ? 4 : 16);
                const int bitsPerVector = (vl == 3 && vn == 3) ? 16 : (vn + 1) * bitsPerComponent;
                const uint32_t bytesPerVector = uint32_t(bitsPerVector + 7) / 8u;
                const uint32_t writeVectorCount = num == 0u ? 256u : uint32_t(num);
                uint32_t cl = m_cycle & 0xFFu, wl = (m_cycle >> 8) & 0xFFu;
                if (cl == 0u)
                    cl = 1u;
                if (wl == 0u)
                    wl = 1u;
                uint32_t sourceVectorCount = writeVectorCount;
                if (cl < wl)
                {
                    const uint32_t fullBlocks = writeVectorCount / wl;
                    uint32_t remainder = writeVectorCount % wl;
                    if (remainder > cl)
                        remainder = cl;
                    sourceVectorCount = fullBlocks * cl + remainder;
                }
                const uint32_t totalBytes = (sourceVectorCount * bytesPerVector + 3) & ~3u;
                pos += totalBytes;
                if (pos > sizeBytes)
                    break;
                continue;
            }
            // every other command is one word
        }
        if (m_irqPending && m_pendingImageQwc == 0u)
        {
            m_irqPending = false;
            m_stalled = true;
            m_held.clear();
        }
        return irqs;
    }

    // FBRST.STC: resume the held bytes (ps2xVif1StallCancel). Returns the i-bits decoded on the way.
    unsigned stc()
    {
        if (!m_stalled)
            return 0u;
        m_stalled = false;
        std::vector<uint8_t> pending;
        pending.swap(m_held);
        return pending.empty() ? 0u : scan(pending.data(), static_cast<uint32_t>(pending.size()));
    }

    // FBRST.RST: vif1_regs zeroed (CYCLE 0), the image remainder and the stall cleared.
    void reset()
    {
        m_cycle = 0;
        m_pendingImageQwc = 0;
        m_irqPending = false;
        m_stalled = false;
        m_held.clear();
    }

    // An EE write to VIF1_CYCLE (0x10003C40): the interpreter's vif1_regs.cycle, which sizes UNPACK.
    void setCycle(uint16_t cycle) { m_cycle = cycle; }
    void setNoIrqStall(bool on) { m_noIrqStall = on; }
    bool stalled() const { return m_stalled; }

private:
    static uint32_t imageQwcFromTag(const uint8_t *data, uint32_t sizeBytes)
    {
        if (!data || sizeBytes < 16u)
            return 0u;
        uint64_t tagLo = 0;
        std::memcpy(&tagLo, data, sizeof(tagLo));
        if (((tagLo >> 58) & 0x3u) != 2u)
            return 0u;
        return static_cast<uint32_t>(tagLo & 0x7FFFu);
    }

    uint16_t m_cycle = 0;
    uint32_t m_pendingImageQwc = 0;
    bool m_irqPending = false;
    bool m_stalled = false;
    bool m_noIrqStall = false;
    std::vector<uint8_t> m_held;
};
