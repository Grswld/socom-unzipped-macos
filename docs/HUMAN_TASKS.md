# Human tasks — the owner's sitting

sittings: 2026-09-17, 2026-09-26, 2026-09-27, 2026-10-11 (read by tools_py.sitting)

What only the owner can decide or do, in one table. Every row stands on a default: the loop proceeds on it and never
waits. Rewritten 2026-09-25 (Sprint 13 Task R4) from the project audit's owner sitting
(`docs/audits/2026-09-25-project-audit.md` §3, rows O1–O13), with two rows that audit missed (O14, O15).

**The rule.** The loop adds a row only when it reaches a step it cannot verify or a decision that is not its to make;
it writes the default it proceeds on beside it and moves on. Anything the loop can do itself (a window on this machine,
a capture, a VM run, a draft, a document) is a sprint task or a backlog row, never a row here -- except the window
itself: a game run's window is the owner's (O20, R297).

**Numbers.** `O<n>` is a row of this table. A Sprint 13 task in milestone O is written "S13 O1". The old decision
numbers are named by their sprint — "Sprint 11 D2", "naming D1–D7" (Sprint 12), "r0004 D1" — because the list this
replaced used bare D-numbers for three different tables (documents audit D61).

**Before 2026-09-25.** The list this replaced — its "Start here" blocks, the "Sprint 11 close" and "Sprint 12 close"
sections, the numbered items and the `## Open` checkboxes — is `docs/archive/HUMAN_TASKS-to-2026-09-25.md`, verbatim,
under a table that gives each of its 87 items a disposition. Any citation of this file's sections, items or line
numbers written before that day means the archive.

| O | the decision or the hand | the default the loop is on | settles | first asked |
|---|---|---|---|---|
| O33 | **The PCSX2 door, your hands** (Sprint 18; the door is complete in code, T1-T6, T9 and T10 (`7f1dd777`) on `sprint-18`): ~~(a) open 53/udp on the box~~ (done 2026-10-11 ~21:51Z after the owner's `aws login`: `open-instance-public-ports` added 53/udp, the other rules untouched; from here `socom2-prod.muis.pdonline.scea.com` resolves to the box, `example.com` NXDOMAIN); ~~(b) one real INSTALL from GitHub~~ (granted 2026-10-11 23:09Z, R366: once, in R352's window, for T7); (c) a window for T0's five questions and T7's proof (INSTALL, BIOS, LAUNCH, the wizard, our lobby; your BIOS dump and r0001 ISO; the first boot's DEV9 lines). The window is R352's (after 03:00-04:00 ADT 2026-10-12); (a) and (b) are done (R366), so T0/T7 run only as far as they can without them (R359). | T0/T7 wait; T8 proceeds | Sprint 18 T0, T1 Steps 6-7, T7 | 2026-10-02 |
| O34 | **Publish the draft GitHub releases `v0.17.0` and `v0.18.0`** (R357): each is tagged on its close merge commit and pushed, its release archives attached to a DRAFT release; publishing is yours. | the drafts wait, unpublished | the two releases | 2026-10-11 |
| O35 | **Two Sprint 17 items named as yours** (the Sprint 17 plan's Task 98 section, "The research/85 adoptions"): (a) Frostfire's band by eye with `PS2X_GS_ZFLOOR` 0 and 1 (#104, LATER 84; the gate's map read flat on 2026-10-04); (b) the hosted deploy of the five r0004 MLS models (#72, `bbf2a702`) and Horizon upstream #38 (`05365bf6`) to the box. | (a) the knob stays on by default and #104 stays open; (b) both stay merged in the tree, the box keeps its current build | #104; #72 and LATER 86 on the box | 2026-10-11 |

O21-O23 and O25-O30, struck by default on 2026-10-11 (R362), are in `docs/archive/HUMAN_TASKS-struck-2026-10-11.md`; O9 and O17, answered, are in `docs/archive/HUMAN_TASKS-to-2026-09-25.md`; O1, O3, O6, O11, O12, O13, O14, O16, O18, O19, answered, are in `docs/archive/HUMAN_TASKS-answered-to-2026-09-27.md` (their struck rows verbatim, moved 2026-09-27 when this file's ceiling fired, DOC_MAINTENANCE check 7); O2, O4, O5, O7, O8, O10, O15 (closed by default 2026-09-27, R321) and O24 (done 2026-09-28) are in the same file, moved 2026-09-28 when the ceiling fired again; all reopenable by number. O20 (answered by R338) and O31 (struck) moved there 2026-10-02.

**How to answer.** One line per decision — "O5: acceptable for v1", "O12: off" — in the next session's prompt or as a
line in the open plan's Log. The loop strikes the row with the date and the ruling that records the answer, and moves
the work it opens into a task.
