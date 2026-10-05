// macOS fork, the VU1 worker Part 3 Task 3.4: the guest allocator's faster bookkeeping must return exactly the
// addresses the original returns. The reference below is a verbatim model of the original PS2Runtime algorithm
// (first fit over an address-ordered block list; full coalesce after a free; realloc shrink / grow-in-place / move),
// driven side by side with the real PS2Runtime::guestMalloc / guestFree / guestRealloc on 100,000 random operations.
#include "MiniTest.h"
#include "ps2_runtime.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace
{
    struct Block
    {
        uint32_t addr, size;
        bool free;
    };
    constexpr uint32_t kDefaultAlign = 16u;

    uint32_t alignUp(uint32_t v, uint32_t a)
    {
        if (a == 0)
            return v;
        const uint32_t m = a - 1u;
        if (v > std::numeric_limits<uint32_t>::max() - m)
            return std::numeric_limits<uint32_t>::max();
        return (v + m) & ~m;
    }
    uint32_t normAlign(uint32_t a)
    {
        if (a == 0u || (a & (a - 1u)) != 0u)
            return kDefaultAlign;
        return std::max(a, kDefaultAlign);
    }

    struct RefHeap
    {
        std::vector<Block> blocks;
        uint32_t alloc(uint32_t size, uint32_t alignment)
        {
            if (size == 0u)
                return 0u;
            const uint32_t na = normAlign(alignment);
            if (size > std::numeric_limits<uint32_t>::max() - (kDefaultAlign - 1u))
                return 0u;
            const uint32_t allocSize = alignUp(size, kDefaultAlign);
            if (allocSize == 0u)
                return 0u;
            for (size_t i = 0; i < blocks.size(); ++i)
            {
                const Block b = blocks[i];
                if (!b.free)
                    continue;
                const uint64_t bs = b.addr, be = bs + b.size;
                const uint32_t aligned = alignUp(b.addr, na);
                if (aligned < b.addr || aligned > be)
                    continue;
                const uint64_t ae = uint64_t(aligned) + allocSize;
                if (ae > be)
                    continue;
                std::vector<Block> rep;
                if (aligned - bs > 0)
                    rep.push_back({b.addr, uint32_t(aligned - bs), true});
                rep.push_back({aligned, allocSize, false});
                if (be - ae > 0)
                    rep.push_back({uint32_t(ae), uint32_t(be - ae), true});
                blocks.erase(blocks.begin() + std::ptrdiff_t(i));
                blocks.insert(blocks.begin() + std::ptrdiff_t(i), rep.begin(), rep.end());
                return aligned;
            }
            return 0u;
        }
        void coalesce()
        {
            size_t i = 1;
            while (i < blocks.size())
            {
                Block &p = blocks[i - 1], &c = blocks[i];
                if (p.free && c.free && uint64_t(p.addr) + p.size == c.addr)
                {
                    p.size += c.size;
                    blocks.erase(blocks.begin() + std::ptrdiff_t(i));
                    continue;
                }
                ++i;
            }
        }
        int find(uint32_t addr) const
        {
            const uint32_t a = addr & PS2_RAM_MASK;
            for (size_t i = 0; i < blocks.size(); ++i)
                if (!blocks[i].free && blocks[i].addr == a)
                    return int(i);
            return -1;
        }
        void free(uint32_t addr)
        {
            if (addr == 0u)
                return;
            const int i = find(addr);
            if (i < 0)
                return;
            blocks[size_t(i)].free = true;
            coalesce();
        }
        uint32_t realloc(uint32_t addr, uint32_t newSize, uint32_t alignment)
        {
            if (addr == 0u)
                return alloc(newSize, alignment);
            if (newSize == 0u)
            {
                free(addr);
                return 0u;
            }
            if (newSize > std::numeric_limits<uint32_t>::max() - (kDefaultAlign - 1u))
                return 0u;
            const uint32_t na = normAlign(alignment);
            const uint32_t req = alignUp(newSize, kDefaultAlign);
            const int idx = find(addr);
            if (idx < 0)
                return 0u;
            const size_t bi = size_t(idx);
            const uint32_t oldAddr = blocks[bi].addr, oldSize = blocks[bi].size;
            if (req <= oldSize)
            {
                if (req < oldSize)
                {
                    blocks[bi].size = req;
                    blocks.insert(blocks.begin() + std::ptrdiff_t(bi + 1), Block{oldAddr + req, oldSize - req, true});
                    coalesce();
                }
                return oldAddr;
            }
            if (bi + 1 < blocks.size())
            {
                Block &next = blocks[bi + 1];
                const uint64_t end = uint64_t(blocks[bi].addr) + blocks[bi].size;
                if (next.free && end == next.addr && uint64_t(blocks[bi].size) + next.size >= req)
                {
                    const uint32_t extra = req - blocks[bi].size;
                    blocks[bi].size = req;
                    if (next.size == extra)
                        blocks.erase(blocks.begin() + std::ptrdiff_t(bi + 1));
                    else
                    {
                        next.addr += extra;
                        next.size -= extra;
                    }
                    return oldAddr;
                }
            }
            const uint32_t n = alloc(newSize, na);
            if (n == 0u)
                return 0u;
            free(oldAddr);
            return n;
        }
    };
}

void register_guest_heap_equivalence_tests()
{
    MiniTest::Case("GuestHeapEquivalence", [](TestCase &tc)
    {
        tc.Run("100,000 random malloc/free/realloc: every address the original's", [](TestCase &t)
        {
            PS2Runtime runtime;
            runtime.configureGuestHeap(0x00200000u, 0x00600000u);
            RefHeap ref;
            const uint32_t base = runtime.guestHeapBase(), limit = runtime.guestHeapLimit();
            ref.blocks.push_back({base, limit - base, true});
            std::mt19937 rng(20261005u);
            std::vector<uint32_t> live;
            const uint32_t aligns[] = {0u, 1u, 3u, 16u, 32u, 64u, 128u, 256u, 4096u};
            std::string first;
            int mismatches = 0;
            for (int op = 0; op < 100000 && mismatches == 0; ++op)
            {
                const uint32_t r = rng() % 100u;
                const uint32_t size = (rng() % 4u == 0u) ? 1u + rng() % 65536u : 1u + rng() % 512u;
                const uint32_t align = aligns[rng() % 9u];
                if (r < 50u || live.empty())
                {
                    const uint32_t a = runtime.guestMalloc(size, align), b = ref.alloc(size, align);
                    if (a != b)
                    {
                        ++mismatches;
                        first = "malloc op " + std::to_string(op) + ": " + std::to_string(a) + " vs " + std::to_string(b);
                    }
                    if (a)
                        live.push_back(a);
                }
                else if (r < 85u)
                {
                    const size_t i = rng() % live.size();
                    runtime.guestFree(live[i]);
                    ref.free(live[i]);
                    live[i] = live.back();
                    live.pop_back();
                }
                else
                {
                    const size_t i = rng() % live.size();
                    const uint32_t a = runtime.guestRealloc(live[i], size, align), b = ref.realloc(live[i], size, align);
                    if (a != b)
                    {
                        ++mismatches;
                        first = "realloc op " + std::to_string(op) + ": " + std::to_string(a) + " vs " + std::to_string(b);
                    }
                    if (a)
                        live[i] = a;
                    else
                    {
                        live[i] = live.back();
                        live.pop_back();
                    }
                }
            }
            t.Equals(mismatches, 0, first);
            // the heap must also be in the same shape: fill it from empty-ish with the same requests
            for (uint32_t a : live)
            {
                runtime.guestFree(a);
                ref.free(a);
            }
            t.Equals(runtime.guestMalloc(16u, 16u), ref.alloc(16u, 16u), "after freeing everything, the same first address");
        });
    });
}
