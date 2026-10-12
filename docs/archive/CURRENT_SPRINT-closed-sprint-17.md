# CURRENT_SPRINT.md -- the Sprint 17 CLOSED block (archived 2026-10-12)

> **ARCHIVED 2026-10-12 (the Sprint 18 close: the live file keeps one CLOSED block, Sprint 18's, under its 12,700-byte
> ceiling; `docs/DOC_MAINTENANCE.md` §5 step 5) -- superseded by the live `docs/CURRENT_SPRINT.md`.** Moved out of the
> live file verbatim but for its heading, which now carries the merge (PR #137, `b4eaa61c`, `v0.17.0`); the rulings
> R322-R338, R345, R348, R351-R363 and R366-R367 stay in the Sprint 17 plan.

## Sprint 17 — CLOSED 2026-10-12 (merged to `main` as `v0.17.0` at `b4eaa61c`, PR #137, 08:09Z; the record of the sprint is the block below)

**Close-out (the PR body).** Opened 2026-09-28 03:05Z off `main` at `d77b58c5`, closed 2026-10-12 on the owner's word
(R351, 2026-10-11): four milestones in the owner's order, F, Q, A and H, with R carried. Every merged branch was
reviewed by a fresh agent. Landed:
- **F, the frame rate:** the hash fold, the PNG encode off the GL thread, `PS2X_VU_STATS` and SYNCV in the gate, and
  the research/85 adoptions (Z floor, stack pool, five Medius models, Horizon #38, VPK fixes). FRAME 21.95 / 27.03 ms
  at the open; 17.20 / 18.18 at the close (bar 17.0 / 20.0). SYNCV 18.7 -> 24.5/s. #59 stays open (O36).
- **Q, the way back:** the online logoff restarts in-process to the main menu (research/78; Q1 did not fire).
- **A, the sound:** the lobby's loop flag (`567b127c`; the music repeats every 188.5 s); the voice divisor on by
  default (R348, #91 left as KNOWN states it, R351).
- **H, the online menus' harness:** scoped in research/88.
- **Beside the plan:** the chain pin G1, macOS tier (a) and its CI (R349/R350, PRs #133 and #134), the README remodel,
  Harry62's digest `docs/research/h62` (R364), and Sprint 19's phase 0 and spec (research/89, R365).

Not done: F2, F5, F6, Q3 and A2-A4 are backlogged (LATER), and R1b/R2/R3b/R4/X3b are carried (#70, #71, #57). The
close proof is the close chain, ALL GREEN at `ed519ae5` (2026-10-12 06:07-07:04Z): gate `s17_close_chain_20261012_060736`
3/3 PASS, PINS MATCH (13), HELDOUT 12/12, exe `057416df…`, plus the release build. Issues since the open: opened 21,
closed 4 (#95, #108, #113, #116), carried 7 (#28, #41, #57, #70, #71, #91, #94); #59, #42, #32 and #26 moved to the
backlog under R367 (O36); the highest #132; milestone 7 closed. The Outcome in the plan has each milestone's bar.

Next: Sprint 18 (above) merges `main` forward, runs T0/T7 in R352's window and closes (R359); then Sprint 19, the
reverse-engineering sprint, off `main` (R360, R365; spec `docs/superpowers/specs/2026-10-11-sprint-19-re-foundation-design.md`).
