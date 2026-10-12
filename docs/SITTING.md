# The owner's sitting

> **Generated -- do not edit.** Written by `python -m tools_py.sitting` from `docs/HUMAN_TASKS.md` (the O rows and the stamp of the sittings), the rulings `docs/RULINGS.md` shows (`tools_py.rulings.rows()`), `docs/BACKLOG.md` (the `Carried` column) and `docs/PLAYTEST.md` (its build block). Change a source and regenerate; `python -m tools_py.sitting --check` exits 1 when this file is stale.

The page as of 2026-10-12, for the sitting after the one of 2026-10-11: 4 open O rows (0 answered or struck); 17 active rulings since 2026-10-11 (16 dated, 1 placed by number or home, 201 undated and unplaceable, not listed); 10 issues carried twice; the build: built.

**How to answer.** One line per item, by number -- "O5: acceptable for v1", "R271: overturn", "#25: close" -- in the next session's prompt or as a line in the open plan's Log.

## 1. The O rows

4 open, 0 answered or struck. Each stands on its default until you answer; days waited are to 2026-10-12.

| O | the hand needed | the default the loop is on | first asked | days waited |
|---|---|---|---|---|
| O33 | The PCSX2 door, your hands | T0/T7 run in R352's window; if they cannot, carried as issues (R359) | 2026-10-02 | 10 |
| O34 | Publish the draft GitHub releases `v0.17.0` and `v0.18.0` | the drafts wait, unpublished | 2026-10-11 | 1 |
| O35 | Two Sprint 17 items named as yours | (a) the knob stays on by default and #104 stays open; (b) both stay merged in the tree, the box keeps its current build | 2026-10-11 | 1 |
| O36 | Keep or close four issues carried twice | they stay open in the backlog, unmilestoned; no sprint takes them until you say | 2026-10-12 | 0 |

Answered or struck since the last sitting (struck rows live in the archive, `docs/archive/HUMAN_TASKS-to-2026-09-25.md`, its "Struck rows moved from the live table" section; reopenable by number):

- none

## 2. The rulings since the last sitting

17 active rulings on or after 2026-10-11, in the counter's order (the sprint-local names last, by date): 16 dated on or after it, and 1 with no date in the label but *placed by number* -- above the highest-numbered active ruling dated before 2026-10-11 -- or *placed by home*, its file dated on or after it. Each stands until you overturn it; an overturn is its number and the word. Left out: 201 active rulings with no date, which neither signal places (`docs/RULINGS.md` lists every ruling).

- **R351** (2026-10-11) Sprint 17 closes; R348's default stands; the `s17_voice_fullscale` 48-window compare is waived and #91 is left as KNOWN states it (7-10 dB short). -- overturn by number
- **R352** (2026-10-11) one window tonight for the lock-bound steps, opening no earlier than 03:00-04:00 ADT on 2026-10-12 (06:00-07:00Z), coordinated with the web main controller. -- overturn by number
- **R353** (2026-10-11) the owner's three O33 items are not done for this window; the controller does what it can without them. -- overturn by number
- **R354** (2026-10-11) names and findings derived from Harry62's `.ide` may be published in this repository, credited where they add to our own findings; his full corpus may not. (am… -- overturn by number
- **R355** (2026-10-11) for now the public tree takes no reconstructed game-function bodies: names, types, notes and tools only. -- overturn by number
- **R356** (2026-10-11) redotcom's findings enter this public tree only as facts, re-derived against our own artefacts and cited as "redotcom research N". -- overturn by number
- **R357** (2026-10-11) `v0.17.0` and `v0.18.0` are tagged on their close merge commits and pushed, each with its release archives attached to a DRAFT GitHub release; the owner publis… -- overturn by number
- **R358** (2026-10-11) PR #134 (macOS CI) merges after its review on green CI; Dependabot #98, #99 and #103 merge on green CI (R294). -- overturn by number
- **R359** (2026-10-11) Sprint 18 closes in full only if T0 and T7 both run in R352's window; otherwise it closes on code, labelled "proof pending", with T0 and T7 carried as issues o… -- overturn by number
- **R360** (2026-10-11) the reverse-engineering sprint (Harry62 plus redotcom) is Sprint 19. -- overturn by number
- **R361** (2026-10-11) Sprint 19's phase-0 research runs as 3-4 parallel Opus agents plus one Fable synthesis review, not a Workflow. -- overturn by number
- **R362** (2026-10-11) owner rows O21-O30 are struck at the Sprint 17 close under R271's circuit breaker; any one is reopened by its number. -- overturn by number
- **R363** (2026-10-11) the untracked Codex harness port (`.agents/`, `.codex/`, `AGENTS.md`) is removed. -- overturn by number
- **R364** (2026-10-11) all of Harry62's work comes into the project, credited as "h62": his raw work (the `.ide`, the return set, the label CSVs, his scripts and object notes) stays… -- overturn by number
- **R365** (2026-10-11) the spec is APPROVED with D-a..D-i at their defaults, D-d as R364 amends it, and D-h amended: when the redotcom export (`re/exports/`) is finished it is handed… -- overturn by number
- **R366** (placed by number) (2026-10-11 23:09Z, the owner's word, O33(b); Sprint 18 T7) -- one real PCSX2 INSTALL from GitHub is granted: the launcher's INSTALL fetches PCSX2 v2.8.2's rel… -- overturn by number
- **R367** (2026-10-12) #59, #42, #32 and #26, each carried twice, leave milestone 7 for the backlog uncarried, so the milestone closes empty; keeping or closing each is the owner's q… -- overturn by number

## 3. The issues carried twice

10 issues carried twice at `Carried` 2 or more in `docs/BACKLOG.md`: keep each on the backlog, or close it as not planned under a ruling.

- #25 (linux, carried 2) The Linux VM's suites are not green on the merged tree: four C++ and four Python cases
- #26 (harness, carried 2) A received game-lobby chat line does not pass the client's chat receive wrap; the receiving function is unidentified
- #28 (audio, carried 2) The mission music degrades with time in a mission; the long in-mission capture against PCSX2 has not run
- #32 (render, carried 2) The menus and the mission re-upload identical tiles: the identical-bytes skip for same_rewritten tiles is the untested fix
- #41 (harness, carried 2) The console-replay pixel test covers a gameplay frame only; a menu-frame fixture is missing
- #42 (audio, carried 2) About 50 ms of the mission music is lost between the mixer's render() and the device, on any endpoint
- #57 (recomp, carried 2) #57's r0004 leg: the r0004 gate 3/3 PINS MATCH on the new emitter's output is owed (rides on R2's chain)
- #59 (render, carried 2) The mission frame rate has no bar: three gates on three exes read 21.9-32.4 ms per guest VBlank over the scripted walk
- #70 (launcher, packaging, carried 2) D2: the launcher builds socom2_game.elf from the player's own disc on first run; the ELF leaves the download
- #71 (online, launcher, carried 2) The launcher fetches the r0004 package as a console does, decrypts it with its own identity and builds the r0004 ELF locally

## 4. The build

The build `docs/PLAYTEST.md` names, to play and to check against:

```
build:    2026-10-12T07:04:19Z   commit ed519ae5ae05 (sprint-17)
archive:  socom2-portable.zip   [player]   (dist-release/portable/socom2-portable.zip)
sha256: d9922495b2552b3fb4930d67b0490cabbcec805dee45a45db92575d3beced452
exe:      socom2.exe sha256: da8d62a88d3201b111476ea48cefc5fda88833e7785b6e665d4e76065c9a62f0
```
