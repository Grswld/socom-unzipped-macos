# Sprint 19 Implementation Plan — "the RE foundation" (PROPOSED 2026-10-11 22:05Z; opens on `sprint-19` off `main` after Sprints 17 and 18 close, R360)

> **For agentic workers:** REQUIRED SUB-SKILL: the project's `loop-iteration` skill runs this plan (one task at a time,
> a failing test first, a fresh reviewer per task, the lock for every build and run, every lock-bound step in chain Z).
> It is the project's form of superpowers:subagent-driven-development.

> Spec: `docs/superpowers/specs/2026-10-11-sprint-19-re-foundation-design.md` (PROPOSED). Research:
> `docs/research/89-re-foundation-phase0.md`. Drafted in the worktree `agent/s19-spec` off `sprint-17` at `d5fe0d59`.
> Task bodies are written at the open, one per task, in this file or a `-tasks` sibling as Sprint 18 did.

**Goal:** redotcom's readings become measurements on our instruments (one capture window), and the reverse-engineering
work of Harry62, redotcom and ours lands in one graded, cited, exported corpus (`re/`, the sidecars, the type master),
with a proven worked example of functional decompilation whose body stays private.

## Global Constraints

- **Order:** T1 and L1 first (lock-free, parallel); then Y, N, W, S, U, M, X1, Q, B1, E1-E3 lock-free in `agent/s19-*`
  worktrees; C1 once B1 and the probes are known; **Z** once in an owner-named window (O20, R297); C3, X2/X3's
  records and K last. A task out of order needs a ruling.
- **The owner's rulings** R354, R355 and R356 bind every task: Harry's derived names with credit, never his corpus;
  no reconstructed body in the public tree; redotcom only as facts re-derived against our artefacts, cited "redotcom
  research N", no text, paths or tools copied.
- **Servers:** no connection to a server not ours (R293); captures read only files on disk or our local stack.
- **Agents:** Opus (`model: "opus"`); a fresh reviewer per task; Fable only for the spec review (the day's one).
- **The repo is public:** leakcheck on every commit; no private or LAN address (Harry's codes file stays out).
- **Commits:** `type(scope): what and why`, subject near 100 chars, `-- <paths>`, the session trailer.
- **Documents:** the `doc-maintenance` skill before writing into KNOWN, HAZARDS, LATER, DEVELOPING, HUMAN_TASKS.

## The task table

Kind: **free** = lock-free; **Z** = a step of the one chain. Bars in short; the spec §5 holds them in full.

| id | title | kind | depends | bar |
|---|---|---|---|---|
| T0 | The open: `sprint-19` off `main`, milestone, the spec's D-a..D-h numbered as rulings, CURRENT_SPRINT | free | 17 and 18 closed | docmaint OK; rulings in this file |
| T1 | The twin table: `re/exports/twins.csv` generated from match.json, bogus and capsule rows flagged | free | T0 | 14,879 rows; `how` counts equal the summary; regenerates byte-identical under test |
| L1 | `re/` README, the types and facts schemas, the validating test, the registry row | free | T0 | planted bad anchor, bad evidence kind, over-grade each fail; docmaint OK |
| Y1 | The type master: `GameObject`, `PlayerObject`, `cAppCamera`, the replication record, aliases per source | free | L1, N-T1 | every field cited; the `+0x113` / `+0x115..+0x119` conflict held with its settling read |
| Y2 | The runtime header `socom2_types_generated.h` + `static_assert`s in the hand headers | free (compiles in Z) | Y1 | `clang -fsyntax-only` clean; regenerates byte-identical |
| Y3 | Ghidra `ImportNames.java`/`ImportTypes.java`; `re/exports/types.json`, `facts.json`, `socom2-types.d.ts` | free (never beside a build) | Y1 | one headless run on r0004 with counts in the Log; exports fresh under test |
| N-T1 | The ps2dis reader promoted to `tools_py` (object block -> `re/types` candidates tagged `harry62`) | free | T0 | synthetic round trip byte-identical; real file reproduces 5,956 labels, 3 objects, 512 new names |
| N-T2 | The `.cds` exporter and the labels-only way back (old T2, T3) | free | N-T1 | unittest: no memory or uninterpreted block in the output |
| N1 | The lever: `Pass=ps2dis-harry62`, provenance per source, screening pass, loose (D-c) | free | N-T1 | `apply_names --report-only` accounts for every candidate |
| N2 | Hand file for the 62 self-evident loader names, `file:line` each | free | N1 | report-only: applied count, 0 refused |
| N3 | r0004-only body reads (101 no-twin + 112 non-exact) within the time box, `Pass=hand` | free | N1 | `name_provenance audit` 0 findings; uncovered count recorded |
| N5 | redotcom-sourced names as `Pass=hand` rows on our own read, "redotcom research N" in `Source` | free | T1 | each row our `file:line`; a reviewer finds no copied text or path |
| D1 | The 32 disagreements dispositioned under test; aliases into Y1 | free | N-T1 | test: every row has a disposition |
| D2 | The five real conflicts, one body read each, a ruling and holds line per moved name | free | D1 | five artefacts; report-only shows no unruled change |
| W1 | Weapons and damage at r0004 into `re/facts/weapons_damage.json` (incl. `MPBOMB Ammo` `damage_point`) | free (probes in Z) | T1, L1 | each entry an r0004 read, re-read by a second agent |
| S1 | The stats record and rating at r0004 into `re/facts/stats.json` | free (PEEK in Z) | T1, L1 | as W1 |
| U1 | The UI command table at r0004 into `re/facts/ui_commands.json` | free (trace in Z) | T1, L1 | as W1; constant vs non-constant answers marked |
| M1 | DME class-2 ids and the voice pair at r0004 into `re/facts/dme_messages.json`, checked against the s4 log and the box's DME logs on disk | free | T1, L1, N1 | one KNOWN row per verified id |
| M2 | Protocol map vs `server/horizon-server` (old P1-P2): a `Server.Test` case or a LATER row per missing message | free | M1 | `dotnet test` green; every row classified |
| M3 | #26's receive path as the map's first entry (old P3) | free | M2 | LATER row 7 points at it |
| X1 | MENULOOP's picture count from our ISO, SHA checked | free | T0 | count and SHA in the Log; KNOWN row; banner on research/09 |
| Q1 | The MemoryCard stub's `Mkdir` answer, re-derived against our libmc read | free (builds in Z) | T0 | planted runtime test |
| Q2 | `tools_py/card_save.py` fill byte and flattening, re-derived against the game's own card writes | free | T0 | Python test |
| Q3 | Whether the launcher's card creator adopts Q2 | free | Q2 | a LATER row or a task |
| B1 | The coverage bitmap (knob-gated in `lookupFunction`) and its `tools_py` reader | free (collected in Z) | T0 | reader unittest on a synthetic dump; tail-call caveat printed |
| E1 | `PS2X_PRIVATE_OVERRIDES_DIR`: an optional CMake hook adding outside sources | free (builds in Z) | T0 | configures unset and set; no public body |
| E2 | The `PS2X_SHADOW` harness: snapshot, original, restore, body, compare GPRs/`sp`/`f0`/write log | free (builds in Z) | E1 | a planted mismatch caught under test |
| E3 | `CZWeaponList_GetWeapon` (r0004 `0x003e6cc0`): the public contract note; the body private (D-a) | free | E2 | the note in `docs/research/`; a test that the public tree holds no body for it |
| C1 | The capture script and manifest: the drivable items, the knobs, the facts each settles | free | B1, W1, S1, U1 | dry-run of the script; manifest schema-tested |
| Z | The chain (spec §3): apply N, recomp, census, build, r0001 + r0004 gates, the E gate with shadow, C2 twice, the PCSX2 legs X2 and X3 | **lock-bound, one window** | all above | census `extents_changed: 0`; gates 3/3 PINS MATCH; shadow calls > 0 and mismatches 0; two capture runs equal apart from timings; X2's counter reads; X3's recording |
| C3 | Promote the facts C2 settled to `capture`/`probe` evidence; the coverage hot set recorded | free | Z | facts re-validated; hot-set file in the Log |
| X2 | The frame clock: an r0004 read of the cap word (before Z), Z's counter reads (after) | free + Z leg | T1 | KNOWN row; research/09 banner |
| X3 | The movie PCM ring: Z's recording cross-correlated both ways; `Mixer::pcmStream*` checked | free + Z leg | Z | KNOWN row; research/32 banner; a fix task for the next window if blocks win |
| K1 | The census before and after, per revision, region and provenance | free | Z | printed in the Log (baseline r0004 1,736/16,417, r0001 1,871/14,879, 2026-10-02) |
| K2 | DEVELOPING "Reverse engineering", README credits (Harry62, redotcom), KNOWN rows | free | C3 | docmaint OK; leakcheck passes |
| K3 | LATER rows: the PC engine fixes the route is for, DebugServer, matching rejected, the next functions | free | E3 | rows with triggers |

## Rulings made on the owner's behalf

None. The spec's D-a..D-h are numbered at the open from HANDOFF's counter when the owner gives his word, or proceed
as their defaults under that counter.

## Outcome

(filled at the close: what landed, Z's artefacts, the facts promoted, the KNOWN rows moved, the LATER rows written, the
owner's rows left)

## Log (newest first)

- **2026-10-11 22:05Z** -- Drafted, PROPOSED: the spec, this plan and `docs/research/89-re-foundation-phase0.md`, from
  the four phase-0 reports (revisions and tooling; compiler and route; types, grading and layout; redotcom's needs) of
  R361's agents, in the worktree `agent/s19-spec` off `sprint-17` at `d5fe0d59`. The two-cartographers spec of
  2026-10-02 carries a superseded banner (R360). No ruling taken; the owner's Fable review and his word are owed.
