# h62 -- caveats, and what is r0004-only

*2026-10-11 22:39Z. Part of the h62 set ([README](README.md)): Harry62's findings, credited to him and restated in
our words. Every h62 claim is believed until a read or probe of ours stands behind it; KNOWN wins.*

Harry62 was candid about his project's limits. His first word on the return set (2026-10-02): his project "contains a
lot of imported s1 demo labels, but most variables are wrong or unused in S2". This page measures that and the other
reasons not to take a label at face value, and says how each kind is treated.

## 1. Imported SOCOM 1 demo labels: 1,577 data labels at the demo's addresses

Our test (2026-10-11): take each of his labels, look up the SOCOM 1 demo's symbol table
(`game/demo_scus_972_05/SCUS_972.05`, the source of research/44) at the **same address**, and ask whether a symbol of
that exact name sits there. **1,577 of his 5,956 labels do** -- every one a data symbol, all between `0x00438770` and
`0x004c4d20`. That range is the zero-filled area past the end of SOCOM II's overlay segment, where the demo's
variables happened to live in the demo's own layout. The labels name SOCOM 1's variables at SOCOM 1's addresses; in
SOCOM II the same addresses hold something else, or nothing.

Our r0004 data-reference census agrees: of the 1,577, **4** are reached by any `lui`/low-half pair in the r0004 image
(`database`, `ai_cmdid` and two animation-name words -- coincidences of layout), against 265 of his 874 other data
labels. The whole block `0x00474000`-`0x0047a000` (1,498 of the imports) has **no** r0004 reference at all.

What this retires. Research/84 §3 (on the unmerged `agent/wizard-harry` branch) listed among "data labels worth
having" the in-game packet-type constants `0x0047598c`..`0x004759cc`, the `m_NetStructIndex*`/`m_*FieldID` statics
`0x004759d4`..`0x00475ab0`, `NetTickSleepTime`, the `zNetMedius` statics `0x004766cc`..`0x004767b0`, the DME callback
pointers around `0x0043b954`, and `_send_buffer`/`_packet_buffer`, beside `theLobby` `0x00452730` and
`gGameNetConnectionInfo` `0x0066a270`. **All of them except those last two are demo imports at the demo's addresses**
and are not SOCOM II data (the last two are his own, and referenced 210 and 6 times); the superseded two-cartographers
spec (`docs/superpowers/specs/2026-10-02-sprint-19-two-cartographers-design.md`) repeats the packet-type range. As a
**vocabulary** they keep value -- the names of what SOCOM 1 replicated and kept in its Medius module are a guide to
what SOCOM II has somewhere ([network.md](network.md)) -- but no address of them is to be used. The transport's
packet-type table at `0x00686248` is different: not a demo import, and all 21 words are referenced.

What it does not touch: his **function** names. None of the 524 names that agree with ours is a demo-address import,
and only 1 of the 512 new names is (`zdb_CNode_DeleteChild` at `0x00227650`, which then happens to sit on a function
start in SOCOM II too). His "most variables are wrong or unused" is true of the imported data labels and of little
else. On functions, research/84 §10 measured the other half of his remark: the share of his named functions that
nothing calls or points to (15 %) equals ours (15 %) -- dead code is the binary's, not the labels'.

**Treatment:** the 1,577 never enter a table of ours with an address. A type or fact that wants one of their names
re-derives the SOCOM II address from code first.

## 2. Mangled names: demo identities, placed by him

207 of the 212 mangled names he put on function starts exist in the demo's symbol table (and 187 of the 194 new ones
that carry a mangling). These are not at the demo's addresses; he matched demo functions to SOCOM II bodies himself,
as our own demo passes do (research/44, 45, 52-54). His placement method is not recorded, so a mangled name of his
carries the same weight as one of our **inferred** passes: strong where its r0001 twin is `exact` and the body's size
matches the demo's, open otherwise. Three of the five real conflicts ([names.md](names.md)) are of this kind.

**Treatment:** as a lever with its own `Pass` name and a score set below `string-set` until his strength is settled
(research/89's open D2); each name's evidence column carries the twin's `how`.

## 3. Purpose names and scratch labels

A patcher names a function by what changing it does: `perfectshot` on the accuracy setter, `shootthroughobjects` on
the material reader, `infammo`, `health2`, `recoil1`, `scopes1`, `foilage`, `gamespeed1`; and leaves scratch marks
(`unk1`..`unk3`, `d`, `T`, `fnc`, `nothing`, `Label`) where he worked. 51 of his 512 new names are of this kind,
plus 6 among the differences. They are valuable as **pointers** -- each marks a body that controls something a player
sees -- and wrong as **identities**.

**Treatment:** aliases only, with the note "what patching it changes"; never a sidecar name.

## 4. The image is a live console with his codes on

His memory image is not the disc's bytes: it is a running PS2's RAM, taken with his code list active. What that means
for anyone reading values out of it:

- **Loader region**: 81 % of its 64-byte chunks match the r0004 ELF (the data half is written by the running game);
  the overlay and data segments match r0004 at 99.8 % (research/84 §2). It is r0004, not a third pressing.
- **The two server host-name strings** (`0x00422530`, `0x00422550` on r0004) are rewritten by one of his codes; the
  bytes there are his, not Sony's. (Our launcher redirects the same names by DNS instead.)
- **The DNAS check**'s first words at `0x002cf330` are replaced by an immediate return -- the same two-word patch the
  community capsule applies -- so a body read of that function from his image reads the patch.
- **A custom map list** is resident at `0x000c0000`, and the map count and list pointer at `0x0044e074`/`0x0044e078`
  hold the values his code wrote, not the game's defaults; the addresses are the game's own map-table head (both are
  referenced by r0004 code), the values are not.
- **Other codes** in his list touch round timing and the round's valve records; values read near them may be his.
- **Below the game** (under `0x00100000`, 67 labels, most in `0x00080000`-`0x000b1000`) his labels sit on resident patch code and data -- a C
  library, a checksum, a key, an anti-cheat routine, per-player weapon/recoil/bloom flags -- loaded under the game by
  his patch, not SOCOM II's code. Research/89 Q1 excludes the overlapping region `0x0009f000`-`0x00180008`
  (a function-map artefact of the same resident code) from every coverage figure.
- **Heap addresses** (334 labels at `0x00700000` and above: weapon data such as "damage modifier", object instances)
  are valid for this one image only; heap placement differs by map and by platform (KNOWN §1: identify a valve by its
  name bytes, never by a copied pointer).
- **The codes themselves** are not reproduced in this set, and the folder holding them stays ignored.

## 5. What is r0004-only

His labels are r0004 addresses, and our map from r0004 back to r0001 resolves only 81 % of functions
(`game/r0004/match.json`, research/89 Q1). So:

- **101 of the 512 new names have no r0001 twin** -- SDK and middleware 28, game online and session 20, network 16,
  his purpose labels 12, game classes 12, his game-logic readings 9, voice 2, render 2. Examples: `rt_msg_client_process_message`
  `0x0063dea0` (our runtime header supplies its r0001 address where the matcher did not), `ArbitrateJoinPacketHandler`,
  `StreamMediaClientInfoPacketHandler`, `MediusUpdateAccountStats`, `DetermineHost`, `FindHost`,
  `SendMapNameToServer`, `UpdateStatsFields`, `Headset_StaticSoundSysCallback`. These can live only in the r0004
  sidecar; most sit in the online overlay code r0004 rebuilt (research/43).
- **Every data address** of his is r0004. A data address crosses to r0001 only through `tools_py.data_via_twin`
  (the three object statics all do: [objects.md](objects.md)); nothing else of his data has been carried.
- **Object offsets** are believed revision-independent (handler twins read the same offsets, research/89 Q3), but
  that is checked per field, not assumed; the `+0x113` question ([objects.md](objects.md)) is exactly such a check.

## How we treat h62, in one table

| kind of h62 claim | grade on entry | what makes it proven | where it goes |
|---|---|---|---|
| a function name agreeing with ours | believed (corroboration of our row) | nothing new needed: our row has its own evidence | an alias column, or nothing |
| a new function name, exact twin | believed | a body read, or a second lever of ours agreeing | a lever with its own `Pass` (research/89 Q5) |
| a new function name, no twin or seed-placed | believed | a body read at r0004 | the r0004 sidecar, after the read |
| a mangled name in conflict with ours | believed; ours stands | one body read against the demo symbol's body | [names.md](names.md) conflicts |
| a purpose note or scratch label | pointer only | -- | alias with "what patching changes" |
| an object field | believed (`outside-label`) | a probe of the field, or a read of its writer | `re/types/` alias on our field |
| a data label that is a demo import | not entered | -- | nowhere with an address |
| a value read from his image | his image only | the same read on a clean image of ours | not entered as game behaviour |
