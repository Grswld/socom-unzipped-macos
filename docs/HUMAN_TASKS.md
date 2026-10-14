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
| O33 | ~~**The PCSX2 door, your hands** (Sprint 18): (a) open 53/udp on the box; (b) one real INSTALL from GitHub; (c) a window for T0's five questions and T7's proof~~ **done 2026-10-12**: (a) opened 2026-10-11 ~21:51Z after your `aws login`; (b) granted 23:09Z (R366), run 08:18Z; (c) R352's window: T0 (research/90) and T7, which reached our lobby (KNOWN §1) | ~~T0/T7 run in R352's window; if they cannot, carried as issues (R359)~~ done | Sprint 18 T0, T1 Steps 6-7, T7 | 2026-10-02 |
| O34 | **Publish the draft GitHub releases `v0.17.0` and `v0.18.0`** (R357): each is tagged on its close merge commit and pushed, its release archives attached to a DRAFT release (v0.18.0's tag and draft follow its PR's merge); publishing is yours. Before you publish, two facts about the archive (both from the v0.17.0 draft, 2026-10-12): its `version.txt` names the nearest tag `git describe` found, `web-sprint-2-cloud-close-...`, not `v0.17.0`; and it still carries `socom2_game.elf`, because R3b, which takes the ELF out of the player archive, was carried (#70). | the drafts wait, unpublished | the two releases | 2026-10-11 |
| O35 | **Two Sprint 17 items named as yours** (the Sprint 17 plan's Task 98 section, "The research/85 adoptions"): (a) Frostfire's band by eye with `PS2X_GS_ZFLOOR` 0 and 1 (#104, LATER 84; the gate's map read flat on 2026-10-04); (b) the hosted deploy of the five r0004 MLS models (#72, `bbf2a702`) and Horizon upstream #38 (`05365bf6`) to the box. | (a) the knob stays on by default and #104 stays open; (b) both stay merged in the tree, the box keeps its current build | #104; #72 and LATER 86 on the box | 2026-10-11 |
| O36 | **Keep or close four issues carried twice** (R367, the Sprint 17 close): #59 (the frame-rate bar's refusal), #42 (~50 ms of mission music lost before the device), #32 (identical tiles re-uploaded) and #26 (the lobby chat receive wrap). Each has a KNOWN row; a third carry is not the loop's. #28, #41, #57, #70 and #71 reach two carries at this close and are asked at the next one. | they stay open in the backlog, unmilestoned; no sprint takes them until you say | #59, #42, #32, #26 | 2026-10-12 |

O21-O23 and O25-O30, struck by default on 2026-10-11 (R362), are in `docs/archive/HUMAN_TASKS-struck-2026-10-11.md`; O9 and O17, answered, are in `docs/archive/HUMAN_TASKS-to-2026-09-25.md`; O1, O3, O6, O11, O12, O13, O14, O16, O18, O19, answered, are in `docs/archive/HUMAN_TASKS-answered-to-2026-09-27.md` (their struck rows verbatim, moved 2026-09-27 when this file's ceiling fired, DOC_MAINTENANCE check 7); O2, O4, O5, O7, O8, O10, O15 (closed by default 2026-09-27, R321) and O24 (done 2026-09-28) are in the same file, moved 2026-09-28 when the ceiling fired again; all reopenable by number. O20 (answered by R338) and O31 (struck) moved there 2026-10-02.

**How to answer.** One line per decision — "O5: acceptable for v1", "O12: off" — in the next session's prompt or as a
line in the open plan's Log. The loop strikes the row with the date and the ruling that records the answer, and moves
the work it opens into a task.
