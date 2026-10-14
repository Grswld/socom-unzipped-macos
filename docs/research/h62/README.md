# h62 -- Harry62's study of SOCOM II r0004, digested onto ours

*2026-10-11 22:39Z (by `date -u`). A research set (class S by location): dated, superseded by a newer note, never
rewritten. Ruling R364 (`docs/superpowers/plans/2026-10-11-sprint-19-re-foundation.md`). KNOWN wins on any
disagreement.*

## What h62 is

**h62** is our name for the body of reverse-engineering work **Harry62** shared with the project owner: a
disassembler project of SOCOM II built in his own tool (ps2dis#), holding a full 32 MB memory image of the game
taken from a running console session of the **r0004** pressing, 5,956 labels he wrote by hand over it, and three
object (struct) definitions; plus his notes on the game's network layer, object layouts and cheat-relevant functions.
He has worked on this game for years (he is the author of the community's r0005 patch and of the ps2dis# and Code
Designer Lite tools), and his labels are the best outside map of the game's online code we have seen.

**Thanks.** This set exists because Harry62 shared his work and **approved our including his findings**. Every page
here credits him. What is tracked is our own restatement of what he found, mapped onto our names, addresses and
types; his project, his label text in bulk and his conventions are not reproduced. Anything sent back to him is the
owner's to send (R364, D-d).

## Where his raw work lives

Locally, git-ignored, never in the public tree: `game/harry62/` in the main checkout. It holds his project file
(`SOCOM2.ide`, 37 MB, the memory image plus labels), the return set we built for him on 2026-10-01 (`return_set/`,
`return_set_summary.txt`), and `h62_extracts/` -- our first analysis note of his work (research/84, written on the
branch `agent/wizard-harry`, never merged) with its extractor and the derived tables (`user_labels.csv`,
`new_function_names.csv`, `disagreements.csv`, `objects.txt`). A contributor without that folder can read this set
alone; a contributor with it can re-derive every number here. The folder is ignored because it holds the game's
memory and his labels in bulk; one of its files carries a private network address, which never enters the tree.

## How to read this set

| page | what it maps |
|---|---|
| [network.md](network.md) | the game's network layer -- RT message client, transport, Medius, keys, the game's own packet code, the chat path -- his strongest area, against our runtime's address tables |
| [objects.md](objects.md) | his three object definitions (GameObject 40 fields, PlayerObject 118, cAppCamera 1) as field tables with offsets, our anchors, the game's own valve names, and the `+0x113` round-state disagreement |
| [names.md](names.md) | the name map: 524 agree / 55 differ / 5 real conflicts; the 512 names new to us by category and by mapping class, every one listed with its r0004 address and r0001 twin |
| [subsystems.md](subsystems.md) | everything else: render (thin on his side), voice and speech recognition, UI command layer, weapons and player stats, AI and animation; where his in-body notes cluster |
| [caveats.md](caveats.md) | what not to take at face value: 1,577 SOCOM 1 demo labels at the demo's addresses, his purpose-named labels, the live cheat codes in the image, the resident patch region; what is r0004-only |

Every table names the **r0004** address first (his labels and our canonical space are r0004: research/89 Q1), then
the **r0001** twin from `game/r0004/match.json` with the matcher's `how` (`exact`, `relinked-body`, `seed+delta`,
`hash+callees`, or none), then our name where we have one. Names are written in our form (`Class_Method`, as
`tools_py/readable_names.py` prints a mangled symbol), not in his project's.

## Grades: every h62 claim is believed

An h62 claim enters the project as **believed** (KNOWN's word) -- research/89's evidence kind `outside-label`. It
becomes **proven** only when one of our own reads or probes stands behind it: a body read at the r0004 address with a
citation and a second reader, a runtime peek or call trace (`PS2X_PEEK`, `PS2X_CALL_TRACE`), or a value-for-value
match against a capture we hold. Where this set says **agrees with ours**, the agreement is with a row of ours that
carries its own evidence (`recomp/socom2_names_r0004.csv`'s `Pass` and `Evidence`, a runtime header's comment, a
KNOWN row); his label then corroborates, it does not prove. Where his reading and a proven row of ours disagree, ours
stands until a read moves it. **KNOWN wins** on every disagreement; a fact here that contradicts `docs/KNOWN.md` is
this set's error.

Three things in this set are our own reads made while digesting (2026-10-11, read-only, no run): the game's valve
names behind his GameObject fields (read from the memory image in his project), the r0004 data-reference census of
his data labels (every `lui`/low-half pair in `game/disc_r0004/socom2_game.elf`), and the demo-symbol test (his label
text against `game/demo_scus_972_05/SCUS_972.05`'s symbol table at the same address). Each is marked where it is used.

## What h62 adds, in ten lines

1. 512 function names we did not have, 450 of them in the game's overlays; 299 carry to r0001 exactly, 101 have no
   r0001 twin at all (names an r0001-only map could never hold).
2. The RT message client by name (`rt_msg_client_process_message` is our runtime's `rtDispatch`), the Medius wrappers,
   the key and hash functions (`KM_*`, `md5*`), and the game's `NetI*` send, ping and registration layer.
3. Independent agreement with our hand-placed runtime anchors: chat receive, sender-name lookup, the player slot
   table, the DNAS check, the server memory read and write handlers, the camera holder, the round record.
4. The round record's slots named, which our read of the image ties to the game's own valve names
   (`mp_round_count`, `mp_major_game_state`, `aiteam_00` ...).
5. The local player's actor laid out to `+0x1994`: per-weapon magazines, stats counters, bloom, damage attribution,
   health at `+0x1044` (agreeing with KNOWN).
6. A live disagreement worth one probe: his state bytes at `+0x115/+0x117/+0x119` against our `+0x113`.
7. The vote, ban, host-selection and whisper paths named (`SendVotePacket`, `CheckBanPlayer`, `DetermineHost`,
   `WhisperCheck`).
8. The speech recogniser's command table (`iARCHI_*`) and the headset callbacks.
9. A measured warning: 1,577 of his labels are SOCOM 1 demo symbols at the demo's addresses, none referenced by r0004.
10. Five real name conflicts with ours, each settled by one body read.

## Index

- [README.md](README.md) -- this page
- [network.md](network.md)
- [objects.md](objects.md)
- [names.md](names.md)
- [subsystems.md](subsystems.md)
- [caveats.md](caveats.md)

Related: research/89 (the RE foundation, phase 0, where Harry's work enters Sprint 19), research/44 (the demo
symbols), research/60 (vtable slots), research/43 (what changed in r0004), research/61 (who consumes names).
