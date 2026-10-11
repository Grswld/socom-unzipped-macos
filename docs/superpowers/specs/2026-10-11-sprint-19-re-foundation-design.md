# Sprint 19 design — "the RE foundation": Harry62's names and redotcom's findings joined to ours, measured on our instruments, in a lasting `re/` corpus

Date: 2026-10-11 22:05Z (by `date -u`). Status: **PROPOSED** to the owner (his word owed; the decisions in §4 are
numbered from HANDOFF's counter at the open). Supersedes and folds in the PROPOSED "two cartographers" spec
(`docs/superpowers/specs/2026-10-02-sprint-19-two-cartographers-design.md`) by R360; branches off `main` after
Sprints 17 and 18 close (R360). Drafted in the worktree `agent/s19-spec` off `sprint-17` at `d5fe0d59` from the
phase-0 research (`docs/research/89-re-foundation-phase0.md`, four Opus agents per R361). The plan is
`docs/superpowers/plans/2026-10-11-sprint-19-re-foundation.md`. The vocabulary is `docs/KNOWN.md`'s; KNOWN wins.

## The owner's word / why now

The owner, 2026-10-11 (the handoff's §5): combine Harry62's names and redotcom's findings into one sprint that extends
into a lasting reverse-engineering foundation -- **first** for what directly benefits redotcom, **second** as the start
of future decompilation work for PC engine fixes (later, not this sprint). His rulings bound it: R354 (Harry's derived
names publishable with credit, his corpus not), R355 (the public tree takes names, types, notes and tools, no
reconstructed bodies; a private project may hold them), R356 (redotcom enters only as facts re-derived against our
artefacts, cited "redotcom research N"; no copied text, paths or tools), R360 (this is Sprint 19), R361 (phase 0 as
parallel Opus agents plus one Fable synthesis review).

Phase 0's central finding sets the order: redotcom's parity registry holds 964 entries and **13 proven**; nearly every
reading in it waits on a capture or trace window that has never run, and the instruments those windows name are ours
(research/89 Q7). The decompilation has given redotcom readings; what it lacks is measurement. So this sprint spends
its one window measuring, and its lock-free days building the corpus the measurements land in.

## Why this is a sprint and not a task

It touches seven homes in one order: a new `re/` directory (types, facts, exports), the name pipeline and both
sidecars, the runtime (a coverage bitmap, a shadow harness, a private-overrides hook, one card fix), `ghidra_scripts/`,
the parity harness's online stage (the capture session), the research notes it corrects, and KNOWN/DEVELOPING/README.
The names change the generated code and the runtime gains instruments, so one chain (recomp, build, gates, the capture
session, the PCSX2 legs) must run once, after everything it carries is decided.

## 1. What is established — [verified: research/89, re-read 2026-10-11]

- **Canonical space: r0004**, every row carrying its r0001 twin and the matcher's `how` (research/89 Q1). The map
  `game/r0004/match.json` resolves 81.1 % of 14,879 r0001 functions; on the 689 functions redotcom's ledger cites it
  resolves 74 % (227 exact, 284 by relink, 178 unresolved, clustered in the online and UI overlays r0004 rebuilt).
- **Compiler: Metrowerks CodeWarrior "MW MIPS C Compiler 2.4.1.01"** for the game code, GCC 2.x for the linked
  middleware; not SN ProDG (research/89 Q2, five independent lines of evidence).
- **Route: functional decompilation** -- a hand body replaces a recompiled function through `replaceFunction` and the
  override registry, proven by an in-process shadow comparison and the gate. Matching is rejected (8-15 agent-days
  before the first match, then years; R355 keeps bodies out of the public tree anyway).
- **Types:** no curated home today (Harry's three objects, research/60's vtables, two runtime headers, the harness's
  dict, a KNOWN prose row); a live disagreement (`+0x113` ours vs Harry's `+0x115..+0x119`) the home must hold.
- **Grading:** by evidence kind, the word derived (research/89 Q4): capture and probe prove; an r0004 read proves
  structure only, after a second agent's re-read and a pinning test; outside labels are placeholders until re-derived.
- **Layout:** `re/` on main; names stay in `recomp/`, notes in `docs/research/`, tools in `tools_py/` and
  `ghidra_scripts/` (research/89 Q5).
- **Tooling gaps:** the `.ide` reader untested off-tree; no Ghidra import of names or types; no coverage bitmap (with a
  tail-call blind spot); DebugServer parked (research/89 Q6).
- **Three contradictions** with one settling measurement each: 30 Hz vs research/09:36; MENULOOP 3,732 vs 3,790; the
  movie PCM ring's layout (research/89, "The three contradictions").

## 2. Goals, ordered by benefit to redotcom, then by the foundation they build

Each goal names the plan's task ids and whether it is **lock-free** or **lock-bound**. Every lock-bound step is in the
one chain **Z** (§3).

### 2.1 For redotcom, in its order of need (research/89 Q7)

1. **C -- captures redotcom cannot run, on our instruments** (C1 lock-free, C2 in Z, C3 lock-free). A scripted
   session on our local Horizon stack (two clients, the harness's online stage; R293: our server only) covering the
   items our harness can drive: send rates, a 2v1 leave, a hit, a claymore left by a quitter, late joins, a weapon
   pickup, one Crossroads Demolition round with the plant and the bridge; with `PS2X_CALL_TRACE`, `PS2X_PEEK`,
   `PS2X_SOCOM2_NET_TRACE`, `PS2X_RDRAM_DUMP` and the coverage bitmap on. Run twice. Archived outside git under
   `logs/`, with a tracked manifest naming each run, its script, its knobs and the facts each item settles.
2. **W -- weapons and damage** (W1 lock-free; its probes in Z). The weapon table and damage rules re-derived at the
   r0004 twins (bullet scale and falloff, armour step, limb cap and spill, hit location, penetration, friendly-fire
   gate, r0004's `MPBOMB Ammo` `damage_point`) into `re/facts/weapons_damage.json`; the 8-byte Hit payload, one
   piercing hit's rounding and the friendly-fire branch taken from C2's traces.
3. **T -- the r0001 <-> r0004 twin table** (T1 lock-free, first). A generated, tracked `re/exports/twins.csv`: every
   r0001 function, its r0004 twin, the `how`, the size and whether the address moved; the bogus 0x404f60-0x4c5380 rows
   and the capsule region flagged. Lets an r0004 trace honestly promote an r0001 reading.
4. **S -- the stats record and ladder** (S1 lock-free; its PEEK in Z). The record layout, its versions and the rating
   at r0004 into `re/facts/stats.json`; settled by a `PS2X_PEEK` of the stats block after C2's match and the
   `UpdateAccountStats` post decoded field by field from the box's or the local stack's logs.
5. **U -- the UI command table** (U1 lock-free; its trace in Z). The 207 rows at r0004 into `re/facts/ui_commands.json`,
   with which answers are constants; a call trace on the dispatcher from boot to the options in Z.
6. **M -- DME semantics** (M1-M3 lock-free). The 24 class-2 ids and the voice pair mapped to r0004 (packet-ID words
   r0001 `0x440e00`+8k = r0004 `0x44d820`+8k), each handler, length and payload; cross-checked against Harry's `NetI*`
   names; verified against files already on disk (the s4 PCSX2 Medius log, our box's DME relay logs) -- reading only,
   no new connection; into `re/facts/dme_messages.json` and a KNOWN row per verified id. Folds in the old spec's
   protocol map against `server/horizon-server` (P1-P3: modelled / relayed opaque / client-local / missing, a
   `Server.Test` case or a LATER row per missing message, #26's receive path as the first entry).

### 2.2 The lasting foundation

7. **L -- `re/` and its rules** (L1 lock-free, first beside T1). `re/README.md` (what it is, the grade table, how an
   outside reverse-engineer adds a name or a type with provenance), the JSON schemas for types and facts, one test
   that validates the schema, checks every anchor equal to its member in `socom2_addresses.h` /
   `socom2_net_bounds.h` / `guest_addresses.py`, and checks each grade against its evidence; a registry row in
   `docs/DOC_MAINTENANCE.md`.
8. **Y -- the type master and its exporters** (Y1-Y3 lock-free; the header compiles in Z). `re/types/*.json` for
   `GameObject`, `PlayerObject`, `cAppCamera` and the replication record (two types, not one name), seeded from
   Harry's object block and research/60, with aliases per source; the `+0x113` / `+0x115..+0x119` disagreement held
   open with its settling read. Exporters: `ImportTypes.java` and `ImportNames.java` (headless, never beside a build
   for memory); a generated `socom2_types_generated.h` with `static_assert`s in the hand headers (syntax-checked
   lock-free with `clang -fsyntax-only`); `re/exports/types.json`, `facts.json` and `socom2-types.d.ts` for redotcom.
9. **N -- one name corpus through one lever with provenance** (N1-N3, N5, D1-D3 lock-free; N4 in Z). The old spec's
   machinery, carried, changed or dropped as research/89 Q5 lists: carried -- the tested ps2dis reader (`tools_py`,
   synthetic round trip; the object block parsed into `re/types` candidates tagged `harry62`), the `.cds` exporter, the
   labels-only way back, the 62 self-evident loader names as a hand file, the r0004-only body reads, the 32-row
   disposition table and the five conflicts settled by body read; changed -- one pass name `ps2dis-harry62` and a
   provenance-per-source intake; redotcom's names enter only as `Pass=hand` rows whose evidence is our own decomp or
   ELF `file:line` and whose source says "redotcom research N" (R356); aliases go into the type master, not only a
   note; the objects cross-check becomes the type master's seed and test; dropped -- the publish decision and Harry's
   consent question (R354 settles both), and the third-branch placement (R360).
10. **X -- the contradictions resolved** (X1 lock-free; X2, X3 legs in Z). MENULOOP's picture count from our ISO; the
    frame clock by an r0004 read of the cap word then a PCSX2 counter read 600 VBlanks apart; the movie PCM ring by a
    PCSX2 recording of the title loop cross-correlated against the disc track decoded both ways. Each ends in a KNOWN
    row (a §3 row where ours was acted on) and a superseding banner on research/09 or research/32.
11. **Q -- quick wins back into socom_pc** (Q1-Q3 lock-free; Q1's build in Z). The MemoryCard stub's `Mkdir` answer
    and the card bytes in `tools_py/card_save.py` (fill byte, flattening), each re-derived against our own libmc and
    card reads first (redotcom research 102, 134 as the pointer, not the proof); whether the launcher's card creator
    adopts them, written as a LATER row or a task. **The possible movie-audio ring-layout bug**: if X3 finds the ring
    carries blocks, the `Mixer::pcmStream*` fix is a task with a test, built in the next window (not this chain).
12. **B -- the coverage bitmap** (B1 lock-free; collected in Z). Knob-gated in `lookupFunction`, a `tools_py` reader
    mapping slots to names, the tail-call blind spot stated in its output; C2's session gives the MP hot set -- the
    priority order for later decompilation and the browser size spike.
13. **E -- the functional-decompilation worked example** (E1-E3 lock-free; E4 in Z). `CZWeaponList_GetWeapon` (r0001
    `0x003c4b40`, r0004 `0x003e6cc0`, exact): the public pieces -- an optional `PS2X_PRIVATE_OVERRIDES_DIR` CMake hook
    (`target_sources` from outside the tree; absent, nothing changes), the `PS2X_SHADOW` harness with tests, a contract
    note; the body in a private location (R355); proven in Z with shadow mode on. The LATER list of what it is for:
    the frame-drop paths, the quit exit fault (kCopy/InitExecPS2), the VU1/GS hot spots -- named, not started.
14. **K -- the record** (K1-K3 lock-free, last). The census before and after, DEVELOPING "Reverse engineering: names,
    types and facts from outside the tree", README credits for Harry62 and redotcom, KNOWN rows, LATER rows
    (DebugServer parked; matching decompilation rejected with its cost; the next functions).

## 3. Lock-bound vs lock-free: one chain, Z, in one owner-named window

Everything except Z is lock-free (suites in worktrees per the build.sh rule; the slow Python suite once, never under a
build). **Z** runs once, in a window the owner names (O20, R297), coordinated with the web controller, through
`scripts/loop_lock.sh run`, in this order:

1. Apply N1-N3/N5 to both sidecars; recomp; `recomp_census` (`extents_changed: 0`, `kind_changed: 0`).
2. One build carrying B1, E1/E2 (the hook and the shadow harness), Q1, Y2's generated header and its asserts.
3. The r0001 and r0004 gates with every new knob off (the build changes nothing by default).
4. The r0004 gate with E3's private override and `PS2X_SHADOW=0x003e6cc0` on.
5. C2: the capture session, twice, with the coverage bitmap and the W/S/U probes on.
6. The PCSX2 legs: X2's frame counter at the main menu, X3's title-loop recording.

A step that fails stops the chain; what ran is recorded and the rest is carried, never re-run in a second window
this sprint without a ruling.

## 4. Decisions for the owner -- each with the default the sprint proceeds on

| # | Decision | Default |
|---|---|---|
| D-a | A private repository for reconstructed bodies (R355 allows one; creating it is the owner's) | Until the owner creates one, E3's body lives in a git-ignored folder in the main tree, read by `PS2X_PRIVATE_OVERRIDES_DIR`; nothing public depends on it |
| D-b | The Metrowerks compiler (obtaining `mwccps2` 2.4.1.01, its licence) | **Moot** under the functional route: no compiler is obtained or used |
| D-c | The strength of Harry's lever (the old spec's D2) | Loose (lands where a strict lever of ours agrees); the 62 self-evident loader names as a hand file; r0004-only names after one body read each. research/84 §10's evidence (dead code is the game's; 524:55 agreement, five real conflicts) is the case for stronger, the owner's to make |
| D-d | What of `agent/wizard-harry` merges | The research/84 note (its patch-code sections and any LAN address left out) and the three scripts only as sources for N's tools; no label CSV, `.ide` or return-set archive (R354) |
| D-e | The capture session's hands | Scripted by the harness (`PS2X_HOST_GAMEPAD`) on two of our clients or ours plus a PCSX2 instance; an item that needs a human pair of hands becomes an owner row, not a blocker |
| D-f | The window for Z | One window the owner names (O20/R297); until then every lock-free task lands |
| D-g | README credit wording for Harry62 and redotcom | One line each in "License and credits", drafted by K2; the owner may reword |
| D-h | redotcom's use of the exports | We publish `re/exports/` on `main`; how redotcom vendors it is its own seat's call (this sprint edits nothing in redotcom) |

## 5. The acceptance bar, per deliverable

- **T1:** `re/exports/twins.csv` has 14,879 rows; its `how` counts equal match.json's summary (exact 6,209,
  relinked-body 2,875, seed+delta 2,733, hash+callees 254, unresolved 2,808); flagged rows present; a test regenerates
  it byte-identical when match.json is present and skips cleanly when not.
- **L1:** the schema test green on the seeded files; a planted wrong anchor, an unregistered evidence kind and a
  grade above its evidence each fail it; `python -m tools_py.docmaint` OK with the registry row.
- **Y1-Y3:** the four types in `re/types/` with every field cited; the `+0x113` conflict present as two aliased
  candidates with its settling read named; the exporters' outputs regenerate byte-identical under test; the generated
  header passes `clang -fsyntax-only` lock-free and compiles in Z; `ImportNames.java`/`ImportTypes.java` run headless
  once on the r0004 program with their counts in the Log.
- **N:** the old spec's bar, carried -- the reader's synthetic round trip byte-identical and the real file reproducing
  `summary.txt` (5,956 user labels, 3 objects) and the 512 new names; `apply_names --report-only` accounts for every
  candidate; `name_provenance audit` 0 findings on both pairs; `recomp_census` `extents_changed: 0`; 32/32
  disagreements dispositioned under test, the five conflicts each with a body-read artefact; every redotcom-sourced
  name with our own `file:line`.
- **W1, S1, U1, M1:** each fact file valid under L1's test; every entry an r0004 read with `file:line` and a second
  agent's re-read; M1's ids checked against the s4 log and the box logs on disk, one KNOWN row per verified id; M2's
  `dotnet test` green.
- **C:** two runs of the same script whose traces differ only in timings; the manifest tracked; each item mapped to the
  facts it settles; the facts it settles move to `capture`/`probe` evidence (C3).
- **X:** X1 a count and the ISO file's SHA in the Log; X2 the two counter reads; X3 the two correlations; each a KNOWN
  row and a banner on the corrected note.
- **Q:** Q1 a planted runtime test of the `Mkdir` answer; Q2 a Python test of the fill and flattening against a read
  of the game's own card writes; Q3 a LATER row or a task.
- **B1:** the reader's unittest on a synthetic dump; Z's session yields a hot-set file and a count of functions hit,
  the tail-call caveat printed.
- **E:** the hook builds with the variable unset and set; the shadow harness's planted mismatch is caught; in Z the r0004
  gate passes with the override and shadow on, PINS MATCH, the shadow call count above zero and the mismatch count 0;
  the body is not in the public tree (a test that `game_overrides` has no body for that address).
- **K:** the census before and after per revision (baseline r0004 1,736/16,417, r0001 1,871/14,879, 2026-10-02); the
  DEVELOPING section, README credits, KNOWN and LATER rows in; docmaint OK; leakcheck passes.

## 6. Scope -- out

A matching decompilation and any Metrowerks toolchain; reconstructed bodies in the public tree (R355); any runtime
behaviour change by default (every new instrument is knob-gated off; Q1 is the one deliberate fix); the PC engine
fixes themselves (named only); the movie-ring fix itself if X3 calls for it (next window); new server handlers beyond a
proven-missing message a client sends; any connection to a server not ours (R293); anything in the redotcom repository;
DebugServer.

## 7. Risks

- **The window is long.** Z carries a full recomp, a long build, four gates, two capture runs and two PCSX2 boots. The
  throughput rules (memory floor, launcher closed, one chain a day) apply; the order puts the gates first so a late
  failure costs only measurements.
- **The capture script may not drive every item.** D-e's default turns an undrivable item into an owner row.
- **A wrong name in the generated code.** The loose lever, the screening pass, the holds file and the census.
- **R356 leakage.** Every redotcom-sourced row cites our own `file:line`; a reviewer checks no sentence of its text and
  no private path entered; leakcheck on every commit.
- **Scope.** 14 goals is a large sprint. The redotcom goals (C, W, T, S, U, M) and Z are the core; Y3's `.d.ts`, the
  Ghidra import and N3's r0004-only reads take what the time box allows, the rest LATER rows with triggers.

## 8. Open questions for the owner

1. D-a: will you create a private repository for reconstructed bodies, or keep them in an ignored folder for now?
2. D-c: loose for Harry's names, or the 299 exact-twin names on his word?
3. D-f: which window for chain Z, given the web project's priority?
4. D-e: is a harness-driven two-client session acceptable for the captures, with any hands-only item as your row?
