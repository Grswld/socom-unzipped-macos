# h62 -- the three object definitions

*2026-10-11 22:39Z. Part of the h62 set ([README](README.md)): Harry62's findings, credited to him and restated in
our words. Every h62 claim is believed until a read or probe of ours stands behind it; KNOWN wins.*

His project defines three objects, each anchored at an r0004 **static that holds a pointer** to the object (the
fields are offsets from the pointed-to block, research/84 §6). All three statics turn out to be ones we already use,
reached independently. He gives names and offsets only -- no widths, no types, no sizes.

| his object | r0004 static | r0001 twin | our name for the static | how we placed it | what the object is |
|---|---|---|---|---|---|
| `GameObject` (40 fields) | `0x004446f8` | `0x00437ce8` | `roundState` (`socom2_net_bounds.h`), `net_game` (`tools_py/parity/guest_addresses.py`) | data-via-twin, 29 evidence-twinned referrers, unanimous | the multiplayer round record, class `CZNetGame` (KNOWN §1, the round-state row) |
| `PlayerObject` (118 fields) | `0x0044d648` | `0x00440c38` | one of the statics KNOWN §1 lists as reaching the local player's actor | `python -m tools_py.data_via_twin 0x440c38`, run for this set: 13 evidence-twinned sites in 10 functions, unanimous | the local player's actor, a `CZSealBody` |
| `cAppCamera` (1 field) | `0x004429b0` | `0x00415ff0` | `cameraHolder` (`socom2_addresses.h`) | data-via-twin (the tool's own control case) | the application camera holder |

That `PlayerObject` is our actor is our inference from the static's twin; the field agreements below (position,
health) bear it out. A type master (research/89 Q3, `re/types/`) should carry each table below as `harry62` aliases
on our fields, graded `outside-label` until a read moves them.

Two cautions for every table: his image was taken **outside an online round** (the player statics read zero in it),
so most values cannot be checked from the image; and several of his labels pack two halfword fields into one note
(`0552 = Total Deaths 554 = Total Shots Taken` at `+0x554`), which we split below.

## GameObject: the round record (`*0x004446f8`, r0001 `*0x00437ce8`)

**Our read of his image explains his 4-byte stride.** At `0x004446f8` his image holds `0x00c2e530`, a heap block
whose allocator header gives a 0x160-byte chunk -- consistent with r0001's `0x14c`-byte `CZNetGame` allocation
(KNOWN §3, the Frostfire row; its decomp chain stands). The words from `+0x4` to `+0x70` are **pointers to valve
records** in the `0x00694xxx` range, each record's first word pointing at the valve's own name string. So his
`+0x0`..`+0x78` fields name **valve slots**, not values; the value lives in the valve record (KNOWN §1: "each value a
short 4 bytes into its record"). Reading the name string through each slot gives the game's own name for it, which is
the strongest check his names can get from a static image.

| off | his name | game's valve name (read from his image) | ours | verdict |
|---|---|---|---|---|
| `+0x00` | Rounds | (slot empty in the image) | -- | believed |
| `+0x04` | HalfRounds | `mp_half_rounds` | -- | agrees |
| `+0x08` | RoundTime | (empty) | -- | believed |
| `+0x0c` | RoundCount | `mp_round_count` | KNOWN §1 names `mp_round_count` in this object | agrees |
| `+0x10` | GameOver | `mp_game_over` | KNOWN §1 (`mp_game_over`) | agrees |
| `+0x14` | PlayerTeam | `player_team` | -- | agrees |
| `+0x18` | mpbomb | `mp_bomb` | -- | agrees |
| `+0x1c` | BombOwner | `bomb_owner` | -- | agrees |
| `+0x20` | MajorGameState | `mp_major_game_state` | KNOWN §1 "major/minor/'my' round-state" | agrees: this is the major state's **valve**, see the `+0x113` section |
| `+0x24` | (MinorGameState, on the heap instance) | `mp_minor_game_state` | as above | agrees |
| `+0x28` | JoinGame | `mp_join_game` | -- | agrees |
| `+0x2c` | LateJoiner | `late_joiner` | reCOM's name in KNOWN §1 (the Frostfire row) | agrees |
| `+0x30` | -- | `mp_45_sec_clock` | -- | new from the image |
| `+0x34` | ForceClockTimer | `mp_x_sec_clock` | -- | same slot, his reading of its use |
| `+0x38` | SealHostages | (empty) | -- | believed |
| `+0x3c` | (TerrHostages, on the heap instance) | (empty) | -- | believed |
| `+0x40` | mp_penalty | `mp_penalty` | -- | agrees |
| `+0x44` | PlayerJoinCount | `player_join_count` | -- | agrees |
| `+0x48` | PlayerReadyCount | `player_ready_count` | -- | agrees |
| `+0x4c` | mp_persistent | `mp_persistent` | -- | agrees |
| `+0x50` | mp_prog_timer1 | `mp_prog_timer1` | -- | agrees |
| `+0x54` | TeamUnbalance | `mp_team_unbalance` | -- | agrees |
| `+0x58` | SealsAliveCount | `aiteam_00` | KNOWN §1 reads `aiteam_00/08` = 1 while both live | agrees in meaning (team 00's alive count) |
| `+0x5c` | TerrAliveCount | `aiteam_08` | as above | agrees in meaning |
| `+0x60` | TeamsTooBig | `mp_teams_too_big` | -- | agrees |
| `+0x64` | AllowRespawn | (empty) | -- | believed |
| `+0x68` | IsSpectator | `mp_spectator` | -- | agrees |
| `+0x6c` | SkipEffects | `skip_effects` | -- | agrees |
| `+0x70` | TotalKills | `total_mp_kills` | KNOWN §1 (`total_mp_kills`) | agrees |
| `+0x74` | SEAL Score | (empty) | -- | believed |
| `+0x78` | TERR Score | (empty) | -- | believed |
| `+0x84`..`+0x94` | -- | five pointers 0x30 apart (`0x00725ef0`..) | -- | unnamed on both sides |
| `+0x98`..`+0xa4` | -- | four floats (529, 388, 575, 388) | -- | unnamed on both sides |
| `+0xd2` | -- | -- | KNOWN §3 (Frostfire row): the "you are a ghost" flag, `0xAF` on ours, 0 on the console | ours only |
| `+0xd9` | IsMPBombPlanted | -- | -- | believed (byte) |
| `+0xdc` | BOOL_ForceGame | -- | -- | believed |
| `+0xde` | BOOL_InGame | -- | -- | believed |
| `+0xe4` | MaxRoundTime | -- | -- | believed |
| `+0xec` | ElapsedTime | -- | -- | believed |
| `+0xf0` | RemainingTime | -- | -- | believed |
| `+0xfc` | mp_timer | -- | -- | believed |
| `+0x113` | -- | -- | **major round-state byte, 3 in a round** (`socom2_net_bounds.h`'s animation bound reads it; KNOWN §1 "`+0x113..+0x115`") | **disagreement**, below |
| `+0x115` | GameMode | -- | ours: the "my" round-state byte (KNOWN §1) | **disagreement** |
| `+0x117` | GameState | -- | -- | **disagreement** |
| `+0x119` | PlayerState | -- | -- | **disagreement** |
| `+0x11c` | -- | float 50.0 in the image | -- | unnamed on both sides |
| `+0x120` | TotalRounds | -- | -- | believed |
| `+0x184` | RadioChannel | -- | -- | **outside the object**: past the 0x14c-byte allocation and the image's 0x160-byte chunk; it lands in the next heap block. Either a field of another object or an offset from a different base |

### The `+0x113` disagreement

**Ours:** the round record's byte `+0x113` is 3 while a round is in play; our runtime's animation-packet bound reads it on
both revisions (`socom2_net_bounds.h`, `roundState`), and KNOWN §1 names the major, minor and "my" round-state bytes at
`+0x113`..`+0x115` (research/19 and the decomp's round code). **His:** `+0x115` GameMode, `+0x117` GameState,
`+0x119` PlayerState, and the major state at `+0x20`.

What our read settles: his `+0x20` is the `mp_major_game_state` **valve slot** (a pointer), not the byte we read at
`+0x113`, so those two do not conflict -- the valve and the byte are two homes of the round state, and which one the
game writes from which is a question about the code, not the layout. What it leaves open: the bytes. His image
(outside a round) reads `+0x113..+0x119` as `00 00 00 01 00 00 00` -- only `+0x116` set -- which neither reading
predicts on its own. Two readings remain possible: his labels sit two bytes off ours on the same small group (his
`+0x115` our "my" byte, his `+0x117`/`+0x119` beyond our three), or r0004 shifted these fields and our `+0x113` holds
on r0004 only because the gate has never needed to distinguish them.

**The settling experiment** (one probe, r0004): `PS2X_PEEK=*0x4446f8+0x110:12` across lobby -> round start -> in
round -> round end -> next round, on one online run of the harness. Ours predicts `+0x113` reaching 3 exactly while
the round runs; his predicts state changes at `+0x115`/`+0x117`/`+0x119` with the mode constant across rounds. A body
read of the r0004 writers of `+0x113`..`+0x119` (the round handler's twin, as `socom2_net_bounds.h` cites) is the
no-run alternative. Until then our `+0x113` stands (it is what the runtime uses and what KNOWN holds) and his three
names enter the type master as `harry62` aliases, `outside-label`.

## PlayerObject: the local player's actor (`*0x0044d648`, r0001 `*0x00440c38`)

The actor block is large; his fields reach `+0x1994`. Grouped by purpose, with our cross-references (KNOWN §1 unless
marked):

| off | his name | ours | verdict |
|---|---|---|---|
| `+0x0` | *CZSealBody | word 0 is the actor's vtable (r0001 `0x006691a0`, r0004 `0x00668b20`, `guest_addresses.py` `actor_vtable`) | agrees: his label names the object's class |
| `+0x4` | PlayerIndex | -- | believed |
| `+0x14` | PlayerName | -- | believed |
| `+0x1c` / `+0x20` / `+0x24` | X / Y / Z pos | "actor positions (words 7/8/9)" -- the same three words | **agrees** |
| `+0x28` | *PlayerLocation | -- | believed |
| `+0xc0` | -- | the mover pointer (KNOWN §1: `*(actor+0xc0)` = mover) | ours only |
| `+0xc8` | Player Team ID | -- | believed |
| `+0xd0` / `+0xd4`, `+0x20c` | -- | state words that differed between maps (KNOWN §1, the Frostfire row) | ours only |
| `+0x174` | -- | the spawn-dead state (8 = spawned dead; KNOWN §3, the Frostfire row's decomp chain) | ours only |
| `+0x3c0`..`+0x3c8` | crosshair X / Y / Z | -- | believed |
| `+0x420` | Total Game Time | **the last ground-hit time**, stamped to the guest clock each frame the actor touches ground (KNOWN §1, the snap-back row) | **disagrees in meaning**: both track the clock; ours is proven by the snap-back logic that reads it |
| `+0x520` | hostages controlled | -- | believed |
| `+0x544`..`+0x586` | game totals: headshots, shots hit (`+0x548`), shots fired (`+0x54c`), kills (`+0x550`), deaths (`+0x552`), shots taken (`+0x554`), suicides (`+0x558`), team kills (`+0x55a`), VIPs killed (`+0x55c`), VIPs extracted (`+0x560`, "divide by 2"), grenades thrown (`+0x56c`; his note on it says `0x55a`, which is team kills), primary objectives (`+0x576`), bonus objectives (`+0x578`), bomb defusals (`+0x57a`), bomb plants (`+0x57c`), points (`+0x580`), rounds won (`+0x582`/`+0x586`) | -- | believed; **his own notes disagree on deaths** (`+0x552` in the object, "deaths 556" in his notes on `UpdateStatsFields` `0x002d19c0`) -- one body read of that function settles every offset in this block |
| `+0x58c`..`+0x5c8` | per-round copies: headshots, shots hit/fired, kills, shots taken, VIPs killed and extracted, grenades, secondary shots fired, primary and bonus objectives, points | -- | believed |
| `+0x6c4`..`+0x6d8` | weapon pointers: primary, secondary, equipment slots 1-3, extra item (e.g. detonator) | -- | believed |
| `+0x73c`..`+0x74c` | ammo pointers for the same five slots | -- | believed |
| `+0x7b4`..`+0x878` | magazines: 10 words per slot for primary, secondary and equipment 1-3 (5 x 10 x 4 B) | -- | believed |
| `+0xdf8` / `+0xdfc` | shots before rest / delay between shots | -- | believed |
| `+0xe04` | active weapon slot | -- | believed |
| `+0xe2c`..`+0xe54` | bloom: current, current 2, target, max, min | -- | believed |
| `+0xe58` | primary bullets fired | -- | believed |
| `+0xf7a` | -- | the alive state, 1 -> 2 -> 3 through a death (KNOWN §1, the online kill ladder) | ours only |
| `+0xfb8` / `+0xfbc` / `+0xfc0` | last attacker's player index / killer's player index / killer's weapon ID | -- | believed; the damage-attribution trio a kill readout wants |
| `+0xfc8` | ClientID | -- | believed |
| `+0xfcf` | DoRespawn | in the byte range our Sprint 5 peeks covered without a reading (`docs/archive/sprints-1-6/`) | believed |
| `+0x1044` | Health | **health, a float, 1.0 -> 0.0 on a kill** (KNOWN §1, the online kill rows) | **agrees** |
| `+0x1061` | -- | a bit read in the kill ladder (KNOWN §1) | ours only |
| `+0x1368` | -- | the movement scale (Sprint 6's gate constants, `docs/archive/sprints-1-6/`) | ours only |
| `+0x1994` | animation update pointer | -- | believed |

What it adds: the weapon, ammo and magazine layout (nothing of ours touched it), the bloom fields (the accuracy model
behind `CZKit_SetCurAccuracy`, his "perfectshot"), the stats counters a scoreboard or a stats upload reads, and the
damage-attribution words. The two agreements (position, health) are proven fields of ours, which is the evidence that
his `PlayerObject` and our actor are one block.

## cAppCamera: the camera holder (`*0x004429b0`, r0001 `*0x00415ff0`)

| off | his name | ours | verdict |
|---|---|---|---|
| `+0xbc` | SpectateClient | the actor the camera follows (KNOWN §1: `*0x415ff0+0xbc` reaches the local actor) | **agrees**; his name adds that in spectator mode the same word selects which client is watched, consistent with `CAppCamera_AttachTo(CZSealBody*)` `0x0029c8c0` in our sidecar |

Our `camera_record` static (`guest_addresses.py`, r0004 `0x00442a14`) sits 0x64 past this static's word; it is a
neighbouring static, not a field of his object.
