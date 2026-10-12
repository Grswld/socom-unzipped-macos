# 89 -- The reverse-engineering foundation, phase 0: seven questions answered before Sprint 19 is designed

Date: 2026-10-11 22:05Z (by `date -u`). Snapshot (class S): a dated investigation, superseded, never rewritten.
Read-only: four parallel Opus research agents (R361) on `sprint-17` at `d5fe0d59`, `agent/wizard-harry` at
`9a5705b3`, and redotcom's working tree (head `0c454ba8d`); nothing was built, run, locked or committed by them. This
note is their synthesis for the spec `docs/superpowers/specs/2026-10-11-sprint-19-re-foundation-design.md`. The
vocabulary is `docs/KNOWN.md`'s, and KNOWN wins on any disagreement. Throwaway measurement scripts stayed in the
session's scratchpad; none is in the tree.

**Credit.** Harry62's ps2dis project (an r0004 memory image with 5,956 hand labels, research/84 on
`agent/wizard-harry`) and redotcom's research (196 notes over our r0001 decompilation, our ELF, our s4 PCSX2 Medius log
and Horizon) are the two outside bodies of work this sprint joins to ours. Under R354 Harry's derived names may be
published with credit, not his corpus; under R356 redotcom enters only as facts we re-derive against our own artefacts,
cited "redotcom research N". Nothing here quotes redotcom's text or names its private paths, and nothing here reproduces
Harry's patch codes (one of his files carries a LAN address; it stays out).
Harry's work digested onto ours (R364): [`docs/research/h62/`](h62/README.md).

## Q1. The canonical address space: r0004, every row carrying its r0001 twin

**Answer: r0004 is canonical; the r0001 twin and the matcher's `how` are mandatory columns.** We ship r0004; Harry's
labels are r0004 (overlay and data segments match r0004 at 99.8 % against 6.5 % / 17.6 % for r0001, research/84 §2);
the r0004 map is the larger (16,417 functions in `recomp/socom2_ghidra_r0004.csv` against 14,879 in
`recomp/socom2_ghidra.csv`, both re-counted 2026-10-11). r0001 stays the language of our Ghidra decompilation and of
redotcom, so the twin column is the bridge every citation crosses, not an option.

**The map.** `game/r0004/match.json` (git-ignored, 1,717,561 B, written 2026-09-23 by `tools_py/address_matcher.py`
with 285 seeds). Its summary, re-read 2026-10-11: 14,879 r0001 functions; exact 6,209, relinked-body 2,875,
seed+delta 2,733, hash+callees 254, **unresolved 2,808 (resolved rate 0.811)**. Only 2,931 functions sit at the same
address in both builds. Tracked derivatives: `recomp/socom2_names_r0004.csv` (`tools_py/carry_names.py`,
`Pass=carried:<pass>`) and the r0004 column of `socom2_addresses.h` (`tools_py/addresses_from_match.py`). The map itself
is not tracked; the corpus needs it (or a twin table generated from it) as a tracked, regenerable artefact.

**A defect not to inherit.** The r0001 csv holds 47 bogus rows in 0x404f60-0x4c5380 (e.g. `FUN_00404f60` spanning
787 KB), a Ghidra export artefact; the r0004 csv's first row `FUN_0009f000` (0x9f000-0x180008, 921,608 B) is the
resident capsule/hook region from the PCSX2 memory image, not game code. Both are excluded from any coverage figure.

**The map measured on what redotcom cites.** Every `FUN_` token and 5-8 digit hex in redotcom's 2026-10-04 placeholder
ledger (697 rows) and its findings file (40 rows) was resolved to its containing r0001 function and looked up:

| how (689 distinct r0001 functions) | count | share |
|---|---|---|
| exact | 227 | 33 % |
| relinked-body | 264 | 38 % |
| seed+delta | 20 | 3 % |
| hash+callees | 0 | 0 % |
| **mapped** | **511** | **74 %** |
| **unresolved** | **178** | **26 %** |

- The ledger misses more than the whole map (26 % against 19 %): 112 of its 178 unresolved sit in the ftscore overlay
  and 54 in zsealetc -- the online and UI code r0004 rebuilt. None carries a recorded tie; they are genuine non-matches,
  exactly where an r0004-canonical corpus needs a body read rather than a carried address.
- Only 64 of the 689 keep their address in r0004: an r0001 address is wrong in r0004 for about 91 % of them.
- 103 of the 689 have a row in `recomp/socom2_names.csv` (15 %): the ledger's functions are mostly unnamed by us.

**Harry's labels through the map, the other way.** Of his 1,058 identifier/mangled labels on an r0004 function start,
946 (89.6 %) have an r0001 twin (exact 764, relinked-body 132, seed+delta 50, hash+callees 2) and 110 have none. Of his
512 names new to us, 101 have no twin -- names an r0001-canonical corpus could not hold.

**r0004-only facts the corpus must mark, not mistake for game behaviour** (all `docs/KNOWN.md` rows): the capsule's
`jr $ra; nop` at `0x002CC670` (`DNASAuthenticate` stubbed; undone by `tools_py/overlay_repair.py`), the one served code
word at `0x1E70CC` and 365 runtime-written data bytes; the version token at `0x00421998`; the bigger online overlays
(`MediusGameInfoRequest` new); harness constants still r0001-only (`sp_death_probe`, `freeze_trace`). Harry's dump also
carries the effects of his own live codes (server strings rewritten, a custom-maps table), so every label needs a
provenance column.

## Q2. The compiler, and the route: functional, not matching

**Verdict: Metrowerks CodeWarrior for PS2, "MW MIPS C Compiler 2.4.1.01", built the game code (about 99 %
confidence); Sony's, SCE-RT/Medius and the libc/libm middleware were linked prebuilt from GCC 2.x (ee-gcc). Not SN
Systems ProDG.** The evidence, strongest first:

1. **Producer string.** `game/disc/SCUS_972.75` holds exactly one `MW MIPS C Compiler (2.4.1.01).PlayStation2`
   (re-counted 2026-10-11) and no `GCC: `, `SN Systems`, `ProDG`, `snc`, `libsn` or `gcc2_compiled.`. The Aug 18 demo
   has the same string plus an `_IOP` twin. The overlays are `MWo3` images (research/05), Metrowerks' overlay format.
2. **The SOCOM 1 demo's symbols and debug (research/44).** 959 `MW MIPS C Compiler` debug producers, each a game unit,
   CodeWarrior MSL paths, `.mwcats`/`.relmain`; beside them 325 `gcc2_compiled.` markers, and the first function after
   each is middleware every time (`sce*`, `__ieee754*`, `rt*`, `Medius*`, `libnet_init`, `sceBInet*`).
3. **Prologue idiom, learned on labelled code, applied to retail.** In MW functions that save registers, `ra` is stored
   first in 3,982 of 3,982; in GCC functions first in 209, last in 298. MW leaves 37.3 % of branch delay slots as
   `nop` against GCC's 10.5 %. On retail FTSCore `ra` comes first in 4,688 and last in 197, the 197 concentrated in
   0x200000-0x2fffff (middleware); 0x300000-0x4fffff is 2,352 to 0.
4. **Vtables and RTTI.** `[RTTI, this-offset, slot0, ...]` with qualified names (`zdb::CNode`), Metrowerks' layout; no
   GCC thunk vtable among the 292 counted (research/60).
5. **Mangling.** Template arguments spelled out in `<...>` (`swap__Q210Metrowerks34cdeque<...>`), which GCC 2.x does
   not emit; 395 MSL classes (research/55).

One open detail: a CodeWarrior 3.0.1 install was also on the build machine in 2002; it may have built some MSL objects.

**Matching decompilation: rejected.** It needs `mwccps2` 2.4.1.01 (a discontinued commercial product; its availability
and licence are the owner's question and are moot under the route below), the MSL headers and flags, a splitter for
MWo3 overlays, and a C++/MSL-template match. Estimate: 8-15 agent-days before the first function matches, then 0.5-3
agent-days per non-trivial function over about 14,657 functions (research/85 §5.1) -- years, for a rebuildable PS2 image
the project does not need, whose bodies R355 keeps out of the public tree anyway.

**Functional decompilation: recommended.** A readable C/C++ body replaces one recompiled function through the existing
override path and is proven equal by an in-process differential check, then the gate.

- *The mechanism exists.* `PS2Runtime::replaceFunction(addr, fn)` writes the dense function table; every JAL reaches
  `dispatchGuestBranch`, so a replacement is seen by every caller; `PS2_REGISTER_GAME_OVERRIDE` descriptors are all
  applied; `game_overrides_socom2.cpp` is compiled into the runner; the music trace and the OSK prefill already wrap
  an original; `src/lib/vu/native/socom2_dispatch_0x1b50.cpp` with `vu1_replay --verify --native` is the precedent of
  a proven native replacement (research/82 §5).
- *The workflow, per function:* pick a named function and confirm its bounds and callees; read the disassembly and the
  generated body, recording its resume entries; write a public contract note (signature, offsets, float behaviour --
  PS2 FPU rules, the runtime's float helpers); write the body (private, R355), handling `ctx->pc == entry` and handing
  any resume entry back to the original; **shadow-compare in process** (new, public: under `PS2X_SHADOW=0xADDR` each
  call snapshots the context, runs the original, restores, runs the body, and compares `v0`/`v1` in full, the
  callee-saved registers, `sp`, `f0` and a write log of guest memory); cross-check with `PS2X_CALL_TRACE`'s `[ret]`
  lines; then the three-stage gate with the override and shadow mode on; record the call and mismatch counts.
- *Cost:* infrastructure 3-4 agent-days (an optional `PS2X_PRIVATE_OVERRIDES_DIR` CMake hook adding `target_sources`,
  the shadow harness with tests, a contract-note template); a leaf 0.25-0.5 agent-days, a non-leaf 1-3; the worked
  example about 1 agent-day plus one gate window.
- *The worked example:* `CZWeaponList_GetWeapon`, r0001 `0x003c4b40`, r0004 twin `0x003e6cc0` (exact, from
  `recomp/socom2_names_r0004.csv`), 72 B. It walks a circular list (sentinel at `this+4`, `next` at `+4`, data at
  `+8`) and returns the first weapon whose byte at `+0x7c` equals the id, else 0. Integer-only, a leaf, no stores, no
  frame; 38 call sites in 23 functions, the most-called named game leaf outside libc. It carries one hazard worth
  proving -- a resume entry at the loop head (`0x3c4b50` in r0001): the generated body returns mid-loop at the
  back-edge when `eeCheckpointDue()`, leaving `pc` at the loop head rather than `ra`, so a shadow comparison must detect
  an original exit with `pc != ra` and skip or re-queue that call -- and one quirk to reproduce: `a0` returns as
  `this+4` and `v1` holds the masked id, so the first comparison is all GPRs, an ABI-only mode only after it passes.
  `CQuat_Apply` (76 sites, VU0 macro mode, stack stores) is round two.

## Q3. One source of truth for types: a JSON master with generated outputs

| Source today | Form | What it holds |
|---|---|---|
| Harry62's object definitions (research/84 §6) | text, r0004 statics | `cAppCamera` (1 field) via `0x4429b0`, `GameObject` (40) via `0x4446f8`, `PlayerObject` (118) via `0x44d648`; names and offsets only, no widths |
| research/60 vtables | lever into the sidecars | class -> vtable -> slot names (194 `vtable-slot` rows on r0004) |
| Ghidra | `ghidra_proj/` git-ignored, no `.gdt` | nothing curated |
| runtime headers | `socom2_addresses.h`, `socom2_net_bounds.h` | statics per revision with evidence comments; offsets as scattered literals |
| `tools_py/parity/guest_addresses.py` | Python dict | the harness's own per-revision statics |
| KNOWN | prose row | the round-state object and its bytes at `+0x113..+0x115` |
| redotcom research 193 | prose tables, r0001 | a 37-field player state and a 0x5c-byte replication record |

**A live disagreement the home must hold:** our round-state byte is `+0x113` (KNOWN, `socom2_net_bounds.h`, and
redotcom research 193 agree); Harry names `+0x115 GameMode`, `+0x117 GameState`, `+0x119 PlayerState`, `+0x20
MajorGameState`. Either his labels sit on other bytes of the same halfwords or the in-memory layout differs. And
"GameObject" names two layouts: Harry's in-memory object and redotcom's packed replication record.

**Recommendation: one JSON file per type under `re/types/`**, each field with offset, width, C type, name, aliases per
source (`harry62`, `redotcom research N`), a grade and an evidence list; anchors per revision naming the existing
address-table member and checked equal to it, never re-stated. Not a header set (no room for provenance, grade or
aliases), not a `.gdt` (binary, Ghidra-only, and the project is ignored). Exporters, each with a test that holds its
output fresh (class G, the way `rulings --check` holds `docs/RULINGS.md`):

1. **Ghidra:** `ghidra_scripts/ImportTypes.java` and `ImportNames.java`, headless like
   `scripts/ghidra_export_functions.sh`; today nothing writes labels or data types into the Ghidra project (a grep for
   `createLabel|setName|DataType` across `ghidra_scripts`, `tools_py`, `scripts` finds none).
2. **The runtime:** one generated `socom2_types_generated.h` (`constexpr` offsets, widths, anchors); hand headers gain
   `static_assert`s against it. Names stay in `recomp/socom2_names*.csv`; the address table keeps its own generator.
3. **redotcom:** `re/exports/types.json` plus a generated `socom2-types.d.ts`; whether and how it vendors the export
   is its own seat's call.

## Q4. One grading scheme: grade by evidence kind, derive the word

KNOWN has proven / believed / retracted. redotcom has levels proven (the original measured running) / reading
(transcribed from decomp or disc) / placeholder, and its 2026-10-04 wave graded PROVEN (an exact source read), STRONG
(a read plus one interpretive step), BELIEVED, UNRESOLVED. Each fact carries typed `evidence[]`; the grade is computed,
so both projects read their own word off the same record:

| evidence kind | KNOWN | redotcom level |
|---|---|---|
| `capture` -- value-for-value against a file on our disk (a scored capture, the s4 Medius log, our box's DME logs) | proven | proven |
| `probe` -- our runtime (`PS2X_PEEK`, `PS2X_CALL_TRACE`, a gate pin) | proven | proven |
| `r0004-read` -- body or data at the matcher-placed twin, `file:line`, re-read by a second agent | proven for structure (address, offset, width, id, size, table contents); believed for behaviour | reading |
| `r0001-read` -- our decompilation, no twin read | believed (settled by the r0004 read) | reading |
| `inference` -- one interpretive step | believed only with a named settling experiment | reading, pending |
| `outside-label` -- Harry's name, a redotcom claim not yet re-derived | not in KNOWN | placeholder |
| contradicted by a capture or probe | retracted (§3, the loser) | retracted |

Incoming grades (R356: only after re-derivation): wave PROVEN -> `r0001-read` once re-read at our decompilation;
STRONG -> believed, the interpretive step named as the settling experiment; BELIEVED -> `inference`; UNRESOLVED and
placeholders -> not entered. **Promotion to KNOWN §1:** one capture or probe, or (structural only) one r0004 read with a
reviewer's re-read and a test pinning the value. Demotion: a contradicting probe or capture.

## Q5. Layout: a directory `re/` on main, not a branch

A branch of a public repository is just as public (R355's bodies go to a private repository by its own text), a second
copy of `recomp/socom2_names*.csv` would break the one-writer rule (`tools_py/apply_names.py`, DEVELOPING "Names in the
generated code"), and redotcom's submodule pins `main`. The layout respects the existing homes:

```
re/README.md      what re/ is, the grade table, how an outside reverse-engineer contributes (a registry row is owed)
re/types/*.json   the type master (hand-edited, schema-tested)
re/facts/*.json   graded, cited facts (dme_messages, weapons_damage, stats, ui_commands, frame_clock, ...)
re/exports/       GENERATED: types.json, facts.json, socom2-types.d.ts, the twin table -- held fresh by a test
names             stay in recomp/ (sidecars, holds, hand proposals); no re/names/
notes             stay in docs/research/; tools in tools_py/ and ghidra_scripts/
```

Writers: the sidecars only through `apply_names.py`; Harry's names through one lever with `Pass`/`Source` provenance;
redotcom-sourced names never as a lever on its word -- a `Pass=hand` proposals file whose evidence is our own
decompilation or ELF `file:line` and whose source names "redotcom research N"; `re/exports/` only through its exporters.

## Q6. Tooling gaps

- **The ps2dis `.ide` reader** exists on `agent/wizard-harry` (`ide_extract.py`, 195 lines, plus `cds_export.py` and
  `build_return_set.py`), untested and outside `tools_py/`. Gap: promote it with a synthetic-fixture test (the real
  37 MB file stays git-ignored).
- **Ghidra import of names and types:** none (Q3). `ImportNames.java` is small (60-80 lines).
- **A function-coverage bitmap** in `PS2Runtime::lookupFunction` (`ps2_runtime.cpp`, beside the dense slot table): a
  knob-gated byte array of SlotCount/8, one racy-but-monotone OR per lookup, dumped like `HleStats`; 25-35 runtime
  lines plus a ~60-line `tools_py` reader. **Blind spot:** a direct tail `j` to a known function is emitted as a C++
  call and bypasses the table, so tail-only callees are undercounted. Collecting it is lock-bound.
- **PCSX2 DebugServer:** parked (memory "external repos", 2026-09-22); nothing on disk. We have PINE read/write
  (`tools_py/parity/pine.py`), no breakpoints; PCSX2 PINE refuses pages with custom VTLB handlers.

## Q7. What redotcom needs most

**The measure:** of the 964 entries in redotcom's parity registry, **13 are proven** (counted 2026-10-11; the open
rows and the captures they owe are redotcom research 101). Nearly every reading names the same promotion route -- a recomp trace window, an online
reference set, or a capture session -- and none has run. **The decompilation has given it readings; what it lacks is
measurement, which is what our instruments do.** Ranked by what raises its validation confidence:

1. **The capture window on our local stack.** Our knobs (`PS2X_RDRAM_DUMP`, `PS2X_CALL_TRACE`, `PS2X_MPEG_TRACE`,
   `PS2X_AUDIO_DUMP`, `PS2X_MC_TRACE`, `PS2X_PEEK`, `PS2X_HOST_GAMEPAD`, the sceCd log, `PS2X_SOCOM2_NET_TRACE`) are the
   ones its trace window lists. The session it wants: send rates, a 2v1 leave, a hit, a claymore left by a player who
   quit, late joins at four phases, a weapon pickup, one scripted Crossroads Demolition round with the plant and the
   bridge. Each archived run with a manifest moves a named group of its entries from reading to proven; acceptance is
   two runs of one script giving the same trace apart from timings.
2. **Weapons and damage.** About 25 round-damage readings and 91 weapon readings exist (the ZWEAPON table, bullet scale
   and falloff, the armour step, limb cap and spill, hit location, penetration). Settled by: the 8-byte Hit payload at
   its send in a capture (including one piercing hit, to fix the rounding), a trace with friendly fire off at the
   gate's branches, and a read of r0004's new `MPBOMB Ammo` `damage_point` (`docs/research/43-what-changed-in-r0004.md:75`, §4).
3. **An r0001 -> r0004 twin table for every function it cites.** It presents r0001; the captures we can take are
   r0004. Without the table, an r0004 trace cannot honestly promote an r0001 reading. Cheap, mechanical, lock-free.
   r0004 rewrote the multiplayer game-state block (r0001 0x2a6ad0-0x2a7d40) and added per-map collision fixups.
4. **The stats record and ladder.** Read and unit-tested there; open are the older record versions, unread bits of two
   bytes, and the wire tail. Settled by a `PS2X_PEEK` of the stats block after a local-stack match and the
   `UpdateAccountStats` post decoded field by field. The original server's own refusal statuses are unobservable.
5. **The UI command table** (207 rows, 124 constant answers; 83 non-constant answers, the node handler unread). Settled
   by a call trace on the dispatcher from boot to the options, and an RDRAM dump after the main menu's start.
6. **The DME in-match messages** (24 class-2 ids plus 2 for voice). redotcom does not speak DME -- its own server makes
   rates and ownership differ by design -- so what matters is semantics: the Hit quantisation and the host-arbitrated
   objects (the bomb state machine, the pickup ask/answer, ladder exclusivity, team switch, the eject vote, chat). The
   ids at r0004 pair with r0001's packet-ID words (r0001 `0x440e00`+8k = r0004 `0x44d820`+8k); the 0-23 order rests on
   one record in our s4 PCSX2 log.
7. The frame clock: settled there (below). 8. The card codec: lowest (its card is its own format now); residual libmc
   result codes settle with one `PS2X_MC_TRACE` per card state.

## The three contradictions, each with its single settling measurement

| # | Ours | redotcom | Likely right | The one measurement |
|---|---|---|---|---|
| 1 | `docs/research/09-menu-movie-ipu.md:36` -- the shell runs "every shell tick (60 Hz)", no citation | 30 Hz game logic, dt 525/15734 s: the swap waits fields against a cap word that reads 30 in the ELF and in 25 RDRAM dumps (redotcom research 110, 208) | redotcom; our own research/87 (pad read once per 30 Hz frame) and research/32 §7.1 (a picture every 2 fields) agree with it | In **PCSX2** (not the recomp, whose HLE paces by PTS), at the main menu, read the frame counter twice 600 VBlanks apart: +300 means 30 Hz. `_DAT_004a4490` is an **r0001 data** address and match.json maps functions only, so the leg names its revision: boot the plain r0001 disc (not instance A, whose card loads r0004) and read `0x004a4490`; or, on r0004, first derive the counter's address lock-free from the twinned functions that access it (the data-via-twin method `tools_py/parity/guest_addresses.py` used for `guest_clock`) and the cap word's xrefs |
| 2 | `docs/research/09-menu-movie-ipu.md:45` -- MENULOOP 3790 pictures (~126 s), no method | 3,732 pictures counted three independent ways and decoded by two decoders, 124.4 s at 30 fps (redotcom research 109, 120) | redotcom | Demux the video PES (0xE0) payloads of `RUN/MOVIES/MENULOOP.PSS` from our ISO and count `00 00 01 00` picture start codes, after checking the file's SHA against the one redotcom measured. Lock-free |
| 3 | research/32: §7.1 reads the disc track as 512-byte blocks left then right; §7 measured the EE-written PCM ring as sample-interleaved (even/odd correlation 0.97, adjacent blocks 0.00) and the mixer plays it interleaved | 0x200-byte blocks per channel on the disc, measured on all 69 tracks (block-edge steps 1.8-8.6x the in-block step; block decoding 1.7-1.9x smoother) (redotcom research 137) | the disc layer is blocks on both sides; the open question is the **ring** a movie fills, since §7.1's 0.99 check is void if its reference was decoded the same way. A possible bug: a ring carrying blocks played interleaved swaps channels every 256 frames | Record PCSX2's output over the title loop and cross-correlate its left channel against the disc track decoded both ways; the layout reaching ~0.99 wins. Then check `Mixer::pcmStream*` against it |

Measurements 1 and 3 boot PCSX2 and are lock-bound; 2 reads the ISO and is not. Each ends in a KNOWN row (or a §3
retraction where ours was acted on) and a superseding banner in the research note it corrects (S notes are superseded,
never rewritten).

## What in redotcom is NOT reverse engineering (never imported as fact)

Its netcode (an authoritative Node room server, its tick and snapshot rates, its protocol versions, catch-up and backlog
tolerances, its load barrier); its services (the Medius stand-in, its store, accounts and cookies, and every status
borrowed from Horizon's code -- a community server's choice, not Sony's); its rate limits and caps; its anti-cheat;
replays and playtests; the voice transport (the original's push-to-talk reading is RE, the transport is not); its
deliberate deviations (the hybrid frame, keyboard/mouse/touch, the modern UI lane, colour packs); every placeholder
row; reCOM and SOCOM 1 material (used there only where the decomp agrees; some labels come from the SOCOM 1 demo
symbols, and some of Harry's r0004 labels at 0x4759xx are unreferenced statics); and everything still stamped
"pending review" there.

## What this note does not settle

The strength of Harry's lever (the old spec's D2, now an owner question), the private repository for bodies (R355
allows one; creating it is the owner's), and whether the capture session needs hands the harness cannot script. Each is
an open question with a default in the spec.
