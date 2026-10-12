# Current sprint

The loop's aim. The `loop-iteration` skill (`.claude/skills/loop-iteration/SKILL.md`) reads this file instead of carrying a sprint pointer of its own; the controller
updates it when a sprint opens or closes, and whenever the order changes. **If you are a new controller, read
`docs/HANDOFF.md` first** -- it says what is in flight and what is owed (the traps are `docs/HAZARDS.md`); this file says what to do next.

**The goal every sprint serves:** SOCOM II running natively on PC with online play, that a stranger runs by pointing the
launcher at their own r0001 ISO and playing a round against another stranger on a hosted Horizon server -- from a public
repository another person can fork, build and contribute to.

```
branch:       sprint-18 -- OPEN 2026-10-01 02:40Z ("the PCSX2 door", the owner's word of 2026-10-01), cut off sprint-17
              beside it (the Sprint 12 precedent). Launcher, server/linux and docs only -- no runtime, recomp or parity
              change; it merges main (v0.17.0) forward and goes to main by PR at its close (R359). Spec
              docs/superpowers/specs/2026-10-01-sprint-18-the-pcsx2-door-design.md (APPROVED); plan
              docs/superpowers/plans/2026-10-01-sprint-18-the-pcsx2-door.md (R339-R344, the table, the Log); task book
              docs/superpowers/plans/2026-10-01-sprint-18-tasks.md; GitHub milestone 8. Agents on agent/s18-* worktrees.
              The main-tree controller alone merges, chains and pushes (the owner's ruling of 2026-09-28).
closed:       sprint-17 -- OPEN 2026-09-28 03:05Z off main at d77b58c5 (v0.16.0), CLOSED 2026-10-12 (R351; the seat plan
              docs/superpowers/plans/2026-10-11-handoff-close-17-18-and-the-re-sprint.md): see "Sprint 17 — CLOSED"
              below. Sprint 16's block is in docs/archive/CURRENT_SPRINT-closed-sprint-16.md (moved 2026-10-12).
spec (17):    docs/superpowers/specs/2026-09-27-sprint-17-sixty-and-the-way-back-design.md (four milestones F, Q, A, H in the
              owner's order; approved by the owner 2026-09-27 ~23:00Z). The Sprint 16, 15, 14, 13, 12 and 11 specs closed
              with v0.16.0 down to v0.11.0.
plans:        docs/superpowers/plans/2026-10-01-sprint-18-the-pcsx2-door.md (Sprint 18, open). Closed: the Sprint 17 plan
              docs/superpowers/plans/2026-09-27-sprint-17.md (the task table, the rulings R322-R338, R345, R348, R351-R363, R366-R367 from the global counter, the
              Outcome, the Log newest first -- its entries before 2026-09-29 in docs/archive/2026-09-27-sprint-17-log-to-2026-09-29.md and those to 2026-09-30 ~10Z in docs/archive/2026-09-27-sprint-17-log-to-2026-09-30.md and to 2026-10-01 ~00Z in docs/archive/2026-09-27-sprint-17-log-to-2026-10-01.md, then docs/archive/2026-09-27-sprint-17-log-to-2026-10-03.md and docs/archive/2026-09-27-sprint-17-log-to-2026-10-11.md; its task book 2026-09-27-sprint-17-tasks.md beside it; GitHub milestone 7).
              The Sprint 16 plan (R299-R321, R314 vacant; docs/superpowers/plans/2026-09-27-sprint-16.md) and the Sprint 15
              plan (R282-R289; R298 the close's window, superseded) with the owner's sitting (R290-R297,
              docs/superpowers/plans/2026-09-26-owner-sitting.md) are closed, Sprint 16's block in docs/archive/CURRENT_SPRINT-closed-sprint-16.md and Sprint 15's in docs/archive/CURRENT_SPRINT-closed-sprint-15.md (moved 2026-10-01); the Sprint 14 plan
              (R269-R281) too, its block in docs/archive/CURRENT_SPRINT-closed-sprint-14.md (moved 2026-09-28); the
              Sprint 13 plan's block is in docs/archive/CURRENT_SPRINT-closed-sprint-13.md (moved 2026-09-27); Sprints 12
              and 11 in docs/archive/CURRENT_SPRINT-closed-sprints-11-12.md; Sprint 10's and older in
              docs/archive/CURRENT_SPRINT-sprints-9-to-11.md (the 2026-09-25 split, R268).
issues:       milestone 7 closed 2026-10-12: #94, #91, #71, #70, #57, #41, #28 carried to the backlog; #59, #42,
              #32, #26 moved there under R367 (O36).
human tasks:  docs/HUMAN_TASKS.md      playtest script: docs/PLAYTEST.md
git strategy: docs/GIT_STRATEGY.md     contributing: CONTRIBUTING.md
rulings:      indexed in docs/RULINGS.md (generated: every ruling with its status and home, Sprint 10's
              R181-R244 ledger table below included) and numbered from the one counter line, docs/HANDOFF.md
              section 2 (Sprint 14 D2).
baselines:    the suite counts live in `docs/DEVELOPING.md` (the table under "Build, run, verify — a newcomer's first hour", rows 3–4) and nowhere else -- this
              line said C++ 686/686 and Python 1457 from 2026-09-20 to 2026-09-22, four sprints after they stopped
              being true, which is why `tools_py/tests/test_doc_maintenance.py` now refuses an undated count outside
              that file. `./build.sh test` exit 0 on the renamed tree (2026-09-25); last green chain: Sprint 17's close chain `s17_close_chain_20261012_060736` on `ed519ae5` (2026-10-12 06:07Z-07:04Z: the Python suite 3984 `OK`, `ps2x_tests` 1286/1285/0, gate PASS 3/3, PINS MATCH 13, HELDOUT 12/12, FRAME mean 17.20 worst1s 18.18, SYNCV 24.5/s -- the close proof); before it Sprint 17's chain 2 `s17_adopt_chain2` on `b1b3f023` (2026-10-04 17:34Z-18:53Z: gate PASS 3/3, PINS MATCH 13, HELDOUT 12/12, FRAME mean 17.06 worst1s 17.86, SYNCV 25.3/s -- research/85's adoptions); before it Sprint 17's batch-6 chain `s17_b6r3` on `368afa8c` (2026-10-02 19:02Z-21:08Z: the Python suite 3976 `OK`, gate PINS MATCH 13, HELDOUT 12/12, FRAME mean 16.99 worst1s 17.86, SYNCV 23.1/s -- N1c's adoption gate; the gate and lock fixes in); before it Sprint 17's batch-5 chain `s17_b5` on `1fd3cfdb` (2026-09-30 07:37Z-08:38Z: the Python suite 3939 `OK`, `ps2x_tests` 1159/1158/0, gate PINS MATCH 13, FRAME mean 16.72 worst1s 17.24, SYNCV 23.3/s -- C1's adoption gate; C2, the refusal count, N1 and #116-#118 behind it); before it Sprint 17's batch-4 chain `s17_b4d` on `881c5a18` (2026-09-29 10:03Z-10:55Z: gate 3/3 PINS MATCH, FRAME mean 16.90 worst1s 17.86, SYNCV 23.4/s; the quiet gates on that exe 16.67-16.92 ms, the VBlank cap); before it Sprint 16's batch-3 chain `s16_b3` on `aa030d1a` (2026-09-27 18:57Z-19:56Z: gate 3/3 PINS MATCH, HELDOUT 12/12,
              the player archive); the close gate `s16_b4g` by hand on the batch-4 exe `2430919f…` (2026-09-28 01:00Z-01:16Z: 3/3 PINS MATCH,
              HELDOUT 12/12, FRAME mean 28.82; the chain itself red on HEAD moving under other sessions' commits); the quiet baseline F0's three gates on `4cbbb14f` (median 27.09 ms, spread 7.0 %); before them the Sprint 14
              close chain `s14_close1` on `352fed01` (2026-09-26 15:29Z-16:19Z: gate 3/3 PINS MATCH, HELDOUT 12/12; the
              Sprint 14 plan's Log, 16:20Z); before it (Sprint 13, 2026-09-25): `s13_proof4_gate` 3/3 PINS MATCH (exe f90eeec0..., 22:16Z), `s13_v4_gate1` PASS on the
              same exe (23:51Z), `s13_proof3_gate` 3/3 (r0001, 20:54Z), `s13_names_r0004_gate` 3/3 PINS MATCH (r0004,
              d027546d...); the plan's Log has each; audio parity
              `s16_v0_t1b` 12/48 on the merged tree (2026-09-27, issue #91: the ambient bed 11 dB low then, 7-10 dB since R348; the Sprint 9 capture `s9_q1_parity_ours2` still reads 31/48 under today's compare)
```

Markers used below: **[A]** autonomous; **[O]** the owner's hands, ears, money or decision; **[B: x]** blocked on x.
"Lock-bound" means it needs a build or a launch: the loop lock, and a window -- announced by the session for a build,
named by the owner for a game run (the sitting's ruling of 2026-09-26, R297); "lock-free" can run at any time.

---

## Sprint 18 — OPEN 2026-10-01 02:40Z ("the PCSX2 door"; `sprint-18` off `sprint-17`, opened beside Sprint 17; milestone 8; R339-R344)

The owner's word of 2026-10-01 ~02:00Z: a global NATIVE / PCSX2 client toggle in the launcher with entirely separate
saved settings; in the PCSX2 client, SELECT your PCSX2 or INSTALL the official release (one button, a tooltip); our
server or a custom address, the community server still "coming soon"; the hosted box answers SOCOM II's host names on
53/udp (the owner granted the DNS changes on the Lightsail box; an agent without a live AWS session asks for the
firewall rule). Spec `docs/superpowers/specs/2026-10-01-sprint-18-the-pcsx2-door-design.md`; plan
`docs/superpowers/plans/2026-10-01-sprint-18-the-pcsx2-door.md` (the table, rulings R339-R344, the Log);
task book `docs/superpowers/plans/2026-10-01-sprint-18-tasks.md` (T0 the spike and T1 the box first). Launcher-only
on our side: no runtime, recomp or parity change; its builds take the lock, its PCSX2 boots a window (O20).
State (2026-10-12): T1-T6, T8, T9 and T10 done; T0 and T7 run in full in R352's window (R366); carried only if they cannot (R359).
Next: Sprint 19, the reverse-engineering sprint, off main after both closes (R360, R361).

---

## Sprint 17 — CLOSED 2026-10-12 (merging to `main` as `v0.17.0`; the record of the sprint is the block below)

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
