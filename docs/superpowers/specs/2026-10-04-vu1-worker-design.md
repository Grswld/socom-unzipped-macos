# VU1 on its own core — design (macOS fork; frame-rate sub-project 1)

Date: 2026-10-04. Branch: `perf-vu1-worker` (on `perf-thread-qos`). Scope: the macOS fork; written so it can be
offered upstream later (nothing in it is macOS-only except the QoS call).

## 1. Intent and decomposition

The owner wants SOCOM II at 60 fps across the whole game, single-player first; online play is upstream's to solve
first and the fork joins later, so nothing here may close that door.

The game targets **30 fps on the console** (one game frame per two VBlanks; `docs/KNOWN.md` line 133). Today a mission
runs at ~19 game frames/s (SYNCV; the framerate spike of 2026-10-04): the game thread is ~80 % busy and its frames
take ~50 ms. The work is three sub-projects with decision points between them:

1. **Locked 30 (this spec)** — the primary objective: the original game's behaviour, needed for future online
   compatibility, and the fix for what the owner feels today. Game-thread work per frame under 33.4 ms.
2. **Headroom for 60** — under 16.7 ms per frame. Started only if the measurement after sub-project 1 shows headroom
   toward it; otherwise the fork stops at 30 and the owner decides whether 60 is worth the further investment.
3. **The 60 fps patch** — halving the game's frame interval, behind a switch. SOCOM II measures its frame time on
   timer T0 and integrates with it (`EeScheduler.cpp` accounting comment: the camera spring), so it may be a
   variable-timestep engine needing far less constant-patching than a typical 60 fps hack. Its design needs targeted
   decompilation of the timing functions (which values are fixed per frame; how measured frame time feeds movement,
   animation, physics and AI; what breaks when the interval halves), each decompiled function checked against the
   recompiled original. Not this spec.

A full decompilation is a different, years-long goal (a native port, a rewritten renderer, deep modding) and is not
part of the frame-rate work.

## 2. Where the time goes (the evidence)

Game thread, four mission profiles each before and after the upstream merge (`logs/spike_fps*/`, macOS `sample`):

| Share | What |
|---|---|
| 27-32 % | VU1 emulation (native, generated and interpreted programs; the clipper) |
| 12-13 % | GS emulation (the front end: CLUT loads, GS memory spans, uploads) |
| 5-7 % | VIF / DMA |
| 14 % | the recompiled game code |
| 8-11 % | runtime / HLE |
| 2-7 % | the guest allocator (scene-dependent) |
| 20-28 % | waiting (VBlank alignment of slipped frames, among others) |

On the console VU1 runs concurrently with the EE. Here it runs inside the VIF1 DMA kick on the game thread
(`EeScheduler.cpp`: "the VU1 interpreter runs inside the DMA kick"); VIF1, VU1 and the GS front end together are
~45-50 % of the game thread.

## 3. The design

### 3.1 The cut

A **VU1 worker** thread, QoS user-interactive (`runtime/host_thread_qos.h`), owns everything downstream of a VIF1 DMA
kick: the VIF1 command interpreter (UNPACK, MPG, MSCAL/MSCALF/MSCNT, ITOP, BASE/OFFSET, the i-bit stall), the VU1
program runs (native, generated, interpreted), XGKICK, the GIF arbiter and the GS front end (GS packets into backend
commands). The GL render thread is unchanged; the worker feeds it instead of the game thread.

### 3.2 One ordered queue

The game thread appends work items to one FIFO; the worker consumes them strictly in order.

- **VIF1 DMA data, copied at kick time, following the whole chain.** The snapshot walks every DMA tag the chain
  contains (REF, REFE, REFS, CNT, NEXT, CALL, RET, END) and copies every block it references — not only the block that
  starts it — in the order the DMA would deliver it. Today a kick completes at once as far as the game can observe,
  so copying at kick time preserves what the game sees.
- **PATH3** (GIF DMA, the texture uploads) and **PATH2** packets go through the same queue, as do the GS privileged
  register writes the game makes (display setup, CSR, IMR). All GS input keeps the order the game produced it in.
- **Backpressure:** `PS2X_VU1_QUEUE_FRAMES` (integer, default 1) is how many whole game frames the worker may still
  be working on when the game thread crosses a frame boundary (`sceGsSyncV`); past it the game thread waits there.
  `0` drains the queue at every frame boundary: the threads overlap within a frame only, the least added latency.
- **The switch:** `PS2X_VU1_THREAD=0` keeps today's synchronous path byte for byte; it is the A/B baseline, the
  fallback and the verification reference. Default 0 until the gates of section 5 pass, then 1.

### 3.3 Sync points (the game thread drains the queue first)

- EE reads of VU1 code or data memory through the EE mapping (`0x11008000`, `0x1100C000`), and writes to it (the
  worker may be using it).
- Reads of VIF1 registers (`0x10003C00`-`0x10003DFF`: STAT, ITOP, TOP, MARK, ...) and VU1 state through VU0
  (`CFC2`/`CTC2` of VU1-side registers).
- GS reads: privileged registers (CSR's FINISH/SIGNAL/LABEL results, SIGLBLID), local-to-host transfers, `ReadVram`.
- DMA status reads stay instant (the data was already copied).

Every sync is counted per frame, by kind, in a `[vu1-worker]` stats line, so the overlap lost to each is visible.

### 3.4 The guest clock

Today VU1's host time is subtracted from guest time (`ps2GuestClockExcludedNs`), because the game thread stalls inside
it and the game must not see a stall as a long frame (it integrates the camera with T0's dt). With the worker the game
thread no longer stalls in VU1, but it can wait at sync points and on backpressure: **those waits are subtracted exactly
as VU1 time is today.** What the game measures on T0 stays its own EE work plus VBlank waits — what today's build and
the console both show it — so the simulation's time steps keep their meaning.

### 3.5 The VIF1 i-bit interrupt

The worker reaches the i-bit, keeps the rest of the stream in its stall buffer (as `ps2_vif1_interpreter.cpp` does
today) and flags INTC5; the game thread takes it at its next interrupt check. The handler's PATH3 uploads and its STC
resume are enqueued in order, so the worker runs the uploads, then resumes VIF1. The game thread does not wait there.
**This depends on a measurement made first (Task 0):** how often the i-bit fires per frame, where in the frame, and
whether the game's frame loop waits on it before starting the next frame. If it does, the threads meet there every
frame, the overlap is capped, and the design is revised before code is written.

### 3.6 Performance cores

The owner's Mac is an M2 Pro: 6 performance and 4 efficiency cores. The heavy threads become three (game thread, VU1
worker, GL render thread); audio is light. A base chip with four performance cores would be tight; the profile is
checked for contention.

## 4. Verification

- **At the queue, not in live play.** Timing shifts change the simulation, so live runs with the switch on and off
  legitimately produce different frames. Instead the queue's contents are recorded once (the ordered items, on the
  synchronous path) and replayed through both the synchronous path and the worker; the GS output (the backend command
  stream and the frame dumps) must be **bit-identical**. The same rig as the batching fix, at a different boundary.
- **Unit tests** for the queue (order, backpressure, the chain snapshot against hand-built DMA chains of every tag
  type, the sync points' drain), and for the clock exclusion.
- **The full suite** green on the branch (the one pre-existing Python failure, `test_launcher_wording` on README.md,
  excepted and named).

## 5. Gates and the decision point

- **Task 0 gate:** the i-bit measurement (3.5) supports the design.
- **Correctness gate:** the queue replay is bit-identical; the suites are green.
- **Performance gate, judged by the worst frames:** a locked 30 means the **95th and 99th percentile** game-frame
  times under 33.4 ms in a mission, not the mean (the 25-30 ms estimate assumes perfect overlap; every sync point
  reduces it).
- **Latency:** the added input-to-display latency is measured with the switch on and off (internally: kick to the
  GS output reaching the renderer, per frame; and the owner's feel), and `PS2X_VU1_QUEUE_FRAMES` is the lever that
  trades throughput for responsiveness.
- **Decision point after A:** with the measurements in, either continue toward 16.7 ms (sub-project 2) or stop at a
  locked 30 and put the 60 fps question to the owner.

## 6. Out of scope

The 60 fps patch and its decompilation (sub-project 3); VU1 parallelism across several cores; the Metal renderer;
the allocator, CLUT and rounding-mode items (candidates for sub-project 2, or folded in where nearly free); online play.
