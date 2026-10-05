# 84 — How much of VU1 a worker core could overlap (2026-10-04, macOS fork)

Context: `docs/superpowers/specs/2026-10-04-vu1-worker-design.md` section 3.5 and the Task 0 gate;
`docs/superpowers/plans/2026-10-04-vu1-worker.md` Tasks 0a-0d. Instrument: `PS2X_VU1_OVERLAP_TRACE`
(`runtime/vu1_overlap_stats.h`), read-only. r0001, single-player mission, owner at the keyboard,
`PS2X_GAME_THREAD_QOS=1 PS2X_VU_STATS=1`, M2 Pro. Logs (git-ignored): `logs/overlap/game.log` (session 1),
`logs/overlap/game2.log` (session 2). Mission seconds are those whose `[vu1-stats]` line shows more than 15,000 VU1
programs a second (133 in session 1, 121 in session 2).

## Findings

| | Session 1 | Session 2 |
|---|---|---|
| SYNCV (game frames/s), mean | 24.5 | 23.5 |
| VIF1 loop host time (VIF1, VU1 and the GS front end inline), mean | 351 ms/s | 336 ms/s |
| ... of it while another guest thread was Ready | **0 %** (every second) | — |
| VIF1 kicks | ~33,000/s | ~19,500/s (~830 per frame) |
| Kicking guest thread | thread 1 (main) only | — |
| VIF1 i-bit interrupts | ~420/s (~17 per frame) | ~400/s |
| i-bit to FBRST.STC | mean 0.1 ms, max 1.4 ms | — |

EE accesses a worker would have to meet (session 2, per game frame):

| Kind | Per frame | A real wait? |
|---|---|---|
| DMA channel 1 registers (CHCR, MADR, QWC, TADR, ASR) | ~834 (one per kick) | **No.** The chain walk that sets them runs at kick time on the game thread and stays there; a kick completes at once as the game observes it, and the data is copied then. |
| VIF1 registers | 1.7 | Yes (drain) |
| DMA control (D_STAT, ...) | 1.7 | Yes, for VIF1-related bits |
| GIF registers | 0.9 | Yes (drain) |
| GS privileged reads | 0 | — |
| VU1 memory through the EE mapping | 0 | — |

- No other guest thread is ever ready while VU1 runs inline: the game drives VIF1 from its main thread, ~1,300
  small MFIFO kicks a frame in session 1. Overlap therefore comes from **the main thread's own code after each kick**,
  which today waits for VU1 to finish inside the CHCR store.
- The i-bit fires ~17 times a frame and its INTC5 handler answers within ~0.1 ms (it kicks PATH3, then writes STC).
  With the queue this needs no meeting: the handler's PATH3 work and STC are enqueued behind the stalled VIF1 data.
- The real meetings are **about four per game frame** (VIF1, D_STAT, GIF register reads).

## Verdict

**CONFIRMED, with section 3.5's premise corrected.** The overlap is not with other guest threads but with the main
thread's continuation after each kick, and the threads need to meet only about four times a frame. DMA channel 1
status reads (~834 a frame) are served at once, without a wait, because the chain walk stays at the kick.

Expected gain, by arithmetic: removing ~335 ms/s of VIF1/VU1/GS work from the game thread leaves about 28 ms of
game-thread work per frame at today's frame rate, under the 33.4 ms budget of a locked 30; the worker carries about
14-18 ms per frame on its own core. The margin is thin at the worst frames, so the spec's p95/p99 gate stands.
