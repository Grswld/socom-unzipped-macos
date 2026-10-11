# Handoff 2026-10-11 -- close Sprints 17 and 18, then open the reverse-engineering sprint

Written 2026-10-11 20:42Z by a read-only research session for the next main-tree controller. Nothing was committed
for this note. Every fact here comes from that session's three research agents (research of 2026-10-11).
**KNOWN wins:** re-read any number before acting on it.

**Paste to start the session (from `C:\Projects\socom_pc`, the main tree):**

> You are the main-tree controller. Read `CLAUDE.md`, then this file
> (`docs/superpowers/plans/2026-10-11-handoff-close-17-18-and-the-re-sprint.md`). Run its §3 "Opening the gates" with
> me first, in one message, each item with its default. Then run Part A (close 17, then 18), and then Part B
> (research, spec and plan for the reverse-engineering sprint).

Reminder for Craig at the open (memory "one main controller in the main tree"): one controller sits in
`C:\Projects\socom_pc`, and every other session works in its own worktree.

---

## 1. How the seat works (from memory and CLAUDE.md, one line each)

- The main-tree controller alone merges, chains and pushes. Agents work in `scripts/agent_worktree.sh` worktrees,
  are dispatched as **Opus** (pass `model: "opus"`), and are reviewed by a fresh reviewer.
- Use Fable sparingly: at most one important review a day. Check the 7-day usage figure at every iteration and stop
  at 96 % or above.
- Don't ask question dialogs. Write prose plus the default ruling and keep going. 15-20 minutes of silence from
  Craig means keep going.
- Run `date -u` before every stamp. Never pipe a check through `| tail` ahead of a commit.
- Every commit names its paths (`-- <paths>`). Never `--delete-branch`. Remove the worktree first, then merge the
  PR, then delete the branch.
- Builds and game runs only through `loop_lock.sh run`, one at a time, and only in a window Craig names. Builds and
  runs lag Craig's machine while he works.
- The repo is PUBLIC. Leakcheck must pass, and no private IPs go into tracked docs.

## 2. Where it stands (2026-10-11, measured)

**Branches** (all level with origin; no fetch was made):

| Branch | Head | Position |
|---|---|---|
| `main` | `4ca3f4f4` | PR #133, the macOS work |
| `sprint-17` | `b5f8789c` | Contains all of main, plus 115 commits |
| `sprint-18` | `f4c8435c` | 163 commits behind sprint-17 (last merge forward `29785ca6`, 10-03); 58 behind and 60 ahead of main |

- **Local only:** `agent/wizard-harry` (worktree `C:\Projects\wt-wizard-harry`). It is 10 commits ahead of main and
  70 behind, and has never been pushed.
- **Untracked at the root:** `.agents/`, `.codex/` and `AGENTS.md`. They are a Codex port of the harness, dated
  10-08. `AGENTS.md` is a copy of `CLAUDE.md` with some names mangled, e.g. `.Codex/settings.json`.
- **Last tag:** `v0.16.0`. No v0.17.0 exists.

**Sprint 17** is OPEN and PAUSED, with no close date (R338).
- Plan: `docs/superpowers/plans/2026-09-27-sprint-17.md`. Its Log is newest at 2026-10-04 22:38Z: R348 makes the
  voice divisor `0x4000` the default.
- The 2026-10-04 chain 2 ran all green on `b1b3f023`. It proved research/85's adoptions, the LATER 97 lobby loop,
  the z-floor A/B (neutral), and LATER 82 and 86.
- Still owed: the `audio_parity.sh compare s17_voice_fullscale`, then Craig's two calls: the voice-knob default and
  "close it".
- The macOS work (R349/R350, PR #133 merged 10-09) is on the branch but **not in the Log**.

**Sprint 18** ("the PCSX2 door") is OPEN.
- Plan: `docs/superpowers/plans/2026-10-01-sprint-18-the-pcsx2-door.md`. Its Log is newest at 2026-10-03 07:13Z.
- T1-T6 and T8-T10 are done in code.
- T0 (the spike) and T7 (the proof) wait on O33: 53/udp, one real INSTALL, and a window.

**Open PRs:** #134 (macOS CI) and Dependabot #98, #99 and #103.

**In flight from this session:** an implementer reworking the README after the layout of
KIexster/god-of-war-recomp. It works in worktree `C:\projects\wt-readme-remodel`, on branch
`agent/readme-remodel` off `sprint-17`.
- It committed `6a265b9c` (README.md only; leakcheck clean; 196 tests OK).
- Content: a logo across the top, badges, a 5-shot grid from `docs/story/img/`, a status table and a Mermaid
  pipeline. It left one placeholder, for a macOS screenshot.
- A reviewer was dispatched on 10-11. Its verdict was owed when this was written; read the plan's Log, or re-review
  it.
- On a clean review, merge it into sprint-17 **before** 17's close, so the close's README pass reads it.
- The implementer kept the README's status claims but did not re-audit them all against KNOWN.

**Docs that are out of date** (fix them in the 17 close's documentation review):
1. `HANDOFF.md` §2 says main = `a519c9bf` (it is `4ca3f4f4`) and calls research/85 "unproven until a chain". It was
   proven on 10-04.
2. `CURRENT_SPRINT.md`:
   - says nothing of the macOS work, the pause or the Sprint 19 proposal;
   - its "last green chain" is `s17_b6r3` (10-02), but chain 2 on 10-04 was all green.
3. O33(c) in `HUMAN_TASKS.md` asks for that same chain, which has already run.
4. Research numbering clashes:
   - On `sprint-17`, `docs/research/` has 80 twice (online-menus and persona-card), 83 = the VU1 entry, and no 84.
   - Sprint 18's plan names `research/83-pcsx2-door-spike.md` for T0, which collides with that 83.
   - `agent/wizard-harry` holds Harry's note as 84.
   - grswld's `perf-vu1-worker` holds a different 84.
   - Renumber before merging. The next free number on sprint-17 is **89** (88 went to the online-menus note, renumbered from 80 on 2026-10-11). Check every branch you merge.

## 3. Opening the gates -- put these to Craig in ONE message at session start

Each item has the default the loop runs on if Craig doesn't answer. Write each answer as a ruling from HANDOFF §2's
counter. **The next free ruling is R351;** re-read the counter, because the macOS plan may have used more.

| # | Decision (Craig's) | Default if unanswered |
|---|---|---|
| G1 | **Close Sprint 17**: "close it". Also confirm the voice-knob default (R348 = `0x4000`), and choose: run the `s17_voice_fullscale` compare, or waive it. | Close; R348 stands. The compare runs in the G2 window if one is given, and is otherwise recorded as waived, with its issue carried. |
| G2 | **A window** (O20/R297) for the lock-bound steps: the voice compare, 18's T0 spike and T7 proof, and the two close gates. About 2-3 hours, with Craig away from the machine. | No window: Part A closes on code. The game-run steps are carried as issues. |
| G3 | **O33, Craig's hands:** (a) open 53/udp (`aws login`, then the task book's Step 6); (b) permission for one real PCSX2 INSTALL from GitHub; (c) his BIOS dump and r0001 ISO in place for the window. | T0 and T7 are carried to a later milestone. Sprint 18 closes on code with "proof pending". |
| G4 | **Sprint 18's close shape:** full (T0/T7 proven in G2's window) or code-only (T0/T7 carried). | Code-only, unless G2 and G3 are both granted. |
| G5 | **Releases:** tag `v0.17.0` and `v0.18.0` on the merge commits as `v0.16.0` was, with the release archives. Publishing a GitHub Release or an announcement is Craig's. | Tags pushed. Archives built and attached as a draft; Craig publishes. |
| G6 | **Pending PRs:** #134 (macOS CI) and Dependabot #98, #99 and #103. | Merge on green CI under R294; #134 after its review. |
| G7 | **Harry62, H0:** Craig asks Harry for permission to publish names derived from his `.ide` in this public repo, with credit. **H1:** what merges from `agent/wizard-harry`: research/84 (renumbered), the assets and the extractor. | No permission yet. His names may be used locally and in private redotcom, never in a public commit. The branch's note merges renumbered, but `user_labels.csv`/`new_function_names.csv` stay out of the tree until H0 is yes. |
| G8 | **redotcom to socom_pc.** redotcom is PRIVATE and socom_pc is PUBLIC. May its findings (DME messages, the ledger, structs, constants) flow into the public tree as facts with citations, and may its text and tools be copied? | Facts flow, re-derived and cited as "redotcom research N". No text is copied verbatim, no private paths appear, and no tool is copied until Craig says. |
| G9 | **Publishing reconstructed game code.** A decompilation branch that commits hand-written C of the game's own functions is a new legal posture. Today the recomp output is git-ignored (`.gitignore:12 /recomp/output_*/`). Choose: (a) public, as other decompilation projects are; (b) a private repo; (c) public, but only names, types, notes and tools, with no reconstructed bodies. | **(c)** until Craig rules otherwise. |
| G10 | **The new sprint replaces the PROPOSED Sprint 19 "two cartographers" spec** (`docs/superpowers/specs/2026-10-02-sprint-19-two-cartographers-design.md`, drawn into it, not discarded). Name and number: Sprint 19. It branches off `main` after both closes. | As stated. The old spec gets a "superseded by" banner. |
| G11 | **Deep research mode for Part B, phase 0.** Either a multi-agent Workflow (needs Craig's explicit "use a workflow"; about 6-8 Opus agents), or 3-4 parallel Opus Explore agents plus one Fable synthesis review. | The agents, not a Workflow. |
| G12 | **The untracked `.agents/`, `.codex/` and `AGENTS.md`:** commit them (after fixing the name mangling), git-ignore them, or leave them. | Leave them untracked and untouched. |
| G13 | **Owner rows O21-O30.** The close's circuit breaker (R271) strikes any row that has stood for two sittings. Answer any you want kept open. | Struck by default under a ruling, and reopenable by number. |

## 4. Part A -- close Sprint 17, then Sprint 18

Order: **17 first.** It goes to main, and 18 then merges main forward. Use the `sprint-close` skill for each close,
plus each plan's Task 99.

1. **Housekeeping (lock-free).**
   - Land the README remodel (§2) if it has a clean review.
   - Bring the Sprint 17 Log up to date with the 10-04 to 10-09 commits: the macOS work R349/R350 and PR #133 with
     its merge back.
   - Fix the out-of-date docs listed in §2.
2. **In G2's window, if given:** the voice compare, then Sprint 17's close gate through the `run-gate` skill.
   - Mind the memory floor (memory "0xC0000142"): 4 GB free with the launcher closed, one suite at a time, never a
     suite under a build.
3. **Sprint 17 close.** Run the sprint-close skill in full: the doc review, the issue stack review, the carry, and
   SITTING. Then:
   - PR `sprint-17` -> `main`, merged with `--merge` (never `--delete-branch`).
   - Tag `v0.17.0` on the merge commit.
   - Write the CLOSED block in CURRENT_SPRINT.
4. **Sprint 18 forward.** In the main tree, merge `main` (the new v0.17.0) into `sprint-18`.
   - Expect conflicts in docs only: CURRENT_SPRINT, HANDOFF, HUMAN_TASKS and CHANGELOG. 18 is launcher, server and
     docs only.
   - Keep merge subjects near 100 characters (memory "commit subject").
   - After the merge, `git add -- <path>` each conflict before the bare commit (memory "bare merge commit takes only
     the index").
   - Then run the suite and push.
5. **In G2's window, with G3 granted:** T0 (the spike; its note takes a free research number, **not 83**), then T7
   (INSTALL -> BIOS -> LAUNCH -> wizard -> our lobby).
   - T7 talks only to our box (3.143.65.100). Nothing reaches a PSRewired host (R293).
6. **Sprint 18 close** (sprint-close skill): PR to main, tag `v0.18.0`, the CLOSED block. If G4 is code-only, file
   T0 and T7 as issues with the O33 row, and label the close "proof pending".
7. **After both closes:**
   - Remove the finished worktrees with the script.
   - List the remaining ones (`wt-wizard-harry`, `wt-i110-lock-heartbeat`, `wt-ds-web`, ...) in HANDOFF with their
     dispositions; don't delete any without its owner row.
   - Rewrite HANDOFF.md for the new sprint.

## 5. Part B -- Sprint 19: one reverse-engineering sprint (Harry + redotcom), kickstarting a lasting RE branch

**Aim (Craig, 2026-10-11):** combine Harry62's names and redotcom's findings into one sprint. It should extend into a
lasting reverse-engineering foundation:
- **first**, for what directly benefits redotcom;
- **second**, as the start of future decompilation work for PC engine fixes (later, not this sprint).

### 5.1 What exists to build on (research of 2026-10-11)

**Harry62** (`agent/wizard-harry`, research/84 there; corpus `game/harry62/SOCOM2.ide`, git-ignored):
- **The corpus:** an r0004 memory image with 5,956 labels.
  - 512 name a function we still call `FUN_`. By r0001 twin, 299 are exact, 80 relinked, 31 seed+delta, 1
    hash+callees and 101 have none.
  - Where his set overlaps ours, 524 agree, 55 differ, and only 5 are real conflicts.
  - It holds 3 object definitions, including `PlayerObject` (118 fields) and `GameObject` (40 fields).
- **His strength is the network layer:** `NetI*`, `rt_msg_client_*` and the Medius wrappers.
- **He is thin on the render path.** His set has 42 render names against our 95.
- **His caveat:** "a lot of imported s1 demo labels, most variables wrong". §10 measured this: dead code is the
  game's own, not a defect of either label set.
- **The machinery:** the PROPOSED Sprint 19 spec already designs it. That covers the ps2dis reader, the `.cds`
  exporter, a lever with `Pass=ps2dis-ide`, the name steps N1-N4 (N4 is lock-bound), the disagreement rulings
  D1-D3, the protocol map P1-P3 against `server/horizon-server`, and the census and credits C1-C4. **Reuse it.**

**redotcom** (`C:\Projects\redotcom\redotcom\docs\research\`, 196 notes; head `0c454ba8d`). Its sources are **our**
artifacts: the r0001 Ghidra decompilation, the ELF, the s4 PCSX2 Medius log and Horizon. All its addresses are
**r0001**, graded "reading, pending review". Ranked:
1. **The DME in-match messages** (`193-m2-mp-net-replication.md:225-276`, corrections at §12):
   - all 24 class-2 message ids, each with its handler, length and payload;
   - Hit is 8 bytes;
   - 7 object types, the player object being 37 fields in 188 bytes, sent every 100 ms online and 200 ms on LAN;
   - the size threshold for splitting a message, and who is authoritative for what.

   **New to us.**
2. **The placeholder ledger** (`ledgers/2026-10-04/LEDGER-ALL.tsv` + `findings.jsonl`): 697 rows, about 430
   resolved, with roles, struct offsets and constants. In a spot check of 16 of its functions, only 2 are already
   in `recomp/socom2_names.csv`.
3. **The game logic runs at 30 Hz** (`110`, `208`). This contradicts our `docs/research/09-menu-movie-ipu.md:36`.
4. **The stats record and ladder** (`155`, `261`): the 256-byte record format with the `'E'` version, and the
   Elo-style rating with a floor of 200 (`FUN_00239d40`).
5. **The UI scripting:** VALVE ops and the 207-row UI command table at 0x3dd4d0 (`104`, `107`).
6. **The card codec, byte-exact:** the 0xAF fill and the flattening rule (`102`), and the libmc commands (`134`).
   The stub should answer -4 to Mkdir.
7. **PSS/movie facts:** MENULOOP has 3732 pictures (we say 3790), and the disc layout uses 0x200-byte blocks (our
   research 32 says sample-interleaved, but that measures a different layer, so reconcile it).

- **Not real RE:** its netcode protocol 11, its room server, anti-cheat, replay, voice, `*_PLACEHOLDER` rows, values
  tuned by feel, and anything filled in from reCOM (SOCOM 1's engine).
- **Reusable tools** (TypeScript): `tools/dump-medius-ids.ts`, `tools/elfImage.ts`, `tools/cardFile.ts`,
  `packages/sound`, `packages/fmv/src/pss.ts` and `packages/archive/src/isoAssetSource.ts`.

**Our own:**
- Ghidra under `tools/ghidra`.
- PCSX2 under `tools/pcsx2`, and `tools/pcsx2_b`.
- The naming pipeline (DEVELOPING "Names in the generated code": one sidecar, one writer, levers),
  `address_matcher.py`, `carry_names.py` and `ui_binding_lever.py`.
- The SOCOM 1 demo `.symtab` (research/44), the vtables (research/60) and the ghidra-ee-reloaded symbols (memory
  "external repos").
- The runtime's `PS2X_CALL_TRACE` and `game_overrides_socom2.cpp`, an existing place where a recompiled function is
  replaced by a hand-written one.
- The parity gate and harnesses, which can serve as a **differential oracle**.

### 5.2 Phase 0 -- deep research first (lock-free; the result is a research note and the sprint spec)

This environment is not a normal decompilation project. Answer these with evidence before designing anything:

1. **Which revision is canonical?** We ship r0004, Ghidra's decompilation is r0001, and redotcom's addresses are
   r0001. Harry's image is r0004.
   - Decide one canonical address space for the RE corpus (likely r0004, with r0001 cross-references).
   - Measure how good the r0001 -> r0004 mapping (`address_matcher.py` / `carry_names.py`) is on the redotcom
     ledger's functions.
2. **Matching or functional decompilation?** A matching decompilation needs the original compiler. Find out what
   built SOCOM II: ee-gcc 2.9x, or SN Systems ProDG, judged from code idioms, the libsn/libc in the loader and the
   strings.
   - A recompiled game already has every function as C++. That makes a **functional** decompilation the natural
     route: replace a function with hand-written C/C++ through the override mechanism, then prove it is equivalent
     with differential traces and the gate.
   - Recommend one route, with its cost.
3. **One source of truth for types.** Harry's `objects.txt`, redotcom's struct offsets, research/60's vtables and
   Ghidra's data types all need one home.
   - Choose the format (a header set, JSON, or a Ghidra GDT) and the exporters to the Ghidra project, to the
     recomp's names and addresses headers, and to **redotcom as JSON or TypeScript typings**.
4. **One grading scheme.** Map redotcom's PROVEN/STRONG/BELIEVED/reading onto KNOWN's proven/believed/retracted.
   Write down what evidence promotes a finding: an r0004 body read, a runtime probe, or a capture match.
5. **The corpus layout and where it lives,** given G7, G8 and G9.
   - A long-lived branch (`re/main`?) or a directory on main (`re/`: `names/`, `types/`, `notes/`, `tools/`,
     `facts/`).
   - Who writes the sidecars, and how the names reach redotcom (a published `facts.json` it vendors?).
6. **Tooling gaps:**
   - Ghidra headless scripts for importing names and types.
   - A tested ps2dis `.ide` reader.
   - A function-coverage bitmap in `lookupFunction`: about 30 lines, lock-bound to collect, and it doubles as the
     browser-multiplayer size spike from the same 10-11 research.
   - The PCSX2 debugger and DebugServer (parked).
   - Watch the r0004-only differences, such as the DNAS bypass and the online code.
7. **What redotcom needs most.** Read its roadmap (`docs/roadmap/`) and its open validation gaps, then rank the RE
   work by what raises its validation confidence. The likely order: the DME messages and replication, the weapon
   and damage constants, the stats and ladder, the UI command table.

Deliverables of phase 0:
- a research note at the next free number (89+);
- the Sprint 19 spec, PROPOSED to Craig with the old two-cartographers spec folded in;
- the plan with its task table.

A Fable review of the spec counts as the one Fable review for that day.

### 5.3 Phase 1 -- the deliverables the spec will likely hold (refine from phase 0)

Ordered by benefit to redotcom; all lock-free unless marked.

- **R1 The DME message spec, at r0004.**
  - Map redotcom 193's 24 handlers from r0001 to r0004.
  - Cross-check them against Harry's `NetI*`/packet names.
  - Verify them against a real capture: the s4 PCSX2 log, or our box's Horizon DME relay logs. Reading only, no new
    connection.
  - Deliverables: `facts/dme_messages.json` and a KNOWN row per verified id.
  - Feeds redotcom's netcode fidelity and Horizon's DME (Sprint 19 spec P1-P3).
- **R2 The combined name corpus.** Harry's 512 plus redotcom's ~430 resolved rows, through one lever with
  provenance per source.
  - A name is promoted only when a lever of ours agrees, or after a body read. The two-cartographers T1-T3/N1-N3
  machinery, generalised to two sources.
  - N4's recomp, build and gate are **lock-bound, one chain**.
  - Publication is gated by G7.
- **R3 The type database.** PlayerObject, GameObject and cAppCamera, plus redotcom's offsets (body+0xfe0..,
  player+0x58c ...).
  - Exported to Ghidra, the runtime headers and redotcom.
- **R4 The facts export.** A machine-readable, graded and cited `facts/*.json` that redotcom consumes:
  - the frame clock;
  - replication rates and smoothing;
  - the stats record and rating formula;
  - the weapon tables;
  - the UI command table;
  - the card codec rules.
- **R5 The contradictions resolved:** 30 Hz vs research 09, MENULOOP 3732 vs 3790, and 0x200 blocks vs
  sample-interleaved. Each gets a ruling and a KNOWN or retraction row.
- **R6 Quick wins back into socom_pc:**
  - the MemoryCard stub's Mkdir answer (-4);
  - card bytes identical to the game's in `tools_py/card_save.py` (0xAF fill, flattening);
  - a check of whether the launcher's card creator should adopt them.
- **R7 The coverage bitmap.** Lock-bound, in a window: the MP-session hot set, which gives the priority order for
  later decompilation and for the browser size spike.
- **R8 The kickstart of the RE branch:**
  - the layout and README for the corpus;
  - a CONTRIBUTING section on how an outside reverse-engineer (Harry) adds names or types with provenance;
  - one worked example of the functional-decompilation workflow, on a small leaf function. Override it, prove it
    with a differential trace, and commit **only** if G9 allows reconstructed bodies.
  - "Later" list of the PC engine fixes it is for: the frame-drop paths, the quit exit fault
    (kCopy/InitExecPS2), and the VU1/GS hot spots. Named, not started.

### 5.4 Constraints for the sprint

- KNOWN wins. Every redotcom or Harry fact enters as `believed` until an r0004 read or a probe proves it.
- No connection to a server that is not ours (R293). Captures are read only from files already on disk.
- Private redotcom text and paths stay out of the public tree unless G8 allows it. The leakcheck passes on every
  push.
- One chain for the sprint: N4 and R7 together, in one window.
- Harry and redotcom are credited in the README and the research note (memory "take compatible commits with
  credit").
