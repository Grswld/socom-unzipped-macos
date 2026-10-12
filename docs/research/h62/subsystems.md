# h62 -- render and the other subsystems

*2026-10-11 22:39Z. Part of the h62 set ([README](README.md)): Harry62's findings, credited to him and restated in
our words. Every h62 claim is believed until a read or probe of ours stands behind it; KNOWN wins.*

Outside the network layer his coverage follows what a patcher touches: weapons, the player, the HUD and message
feed, the UI command layer, the speech recogniser. The render path, which is where most of our own work has gone, is
thin on his side.

## Render: thin on his side

Counted by one rule over function names (any of Draw, Render, Visual, 2D/bitmap/poly, texture, decal, particle, lens,
fader, camera, GS, DMA, VIF, VU, frame): **he names 120 render-family function starts, we name 278; 78 are named by
both, 42 by him alone.** Of our 63 render-core functions (the `zVid`, VIF/VU1, `sceGs`, `sceDma`, `CVisual`, Draw
and Render names), 39 carry any label of his -- one each, nearly all the `sceGs*`/`sceDma*` library names agreeing
with ours, plus four notes on `zVid_Swap` (his "fps fnc").

His 42 render-family names new to us:

- **12 VU0 library routines** (`sceVu0ApplyMatrix`, `InnerProduct`, `Normalize`, `DivVector[XYZ]`, `InterVector[XYZ]`,
  `TransMatrix`, `CopyMatrix`, `UnitMatrix`, `ClampVector`, `ViewScreenMatrix`; `0x001bfc00`..`0x001c0768`, all at the
  same address on r0001). `sceVu0InterVectorXYZ` at `0x001c0768` is the call our skeleton-root probe once read on
  return from (archived Sprint 6 plan), so the name closes an old note.
- **HUD, map and 2D**: `C2DBitmap` draw and position, `C2DPoly` UV and colour, `C2DBitmapPoly` translucency and
  frame-buffer alpha, `C2DString`, `CRenderableString_SetMapOffset`, `CCheckBox_LoadSpec`, `C2DButton` alpha,
  `MapCompass` texture and transparency, the claymore, MP-bomb and player map items' draw.
- **World and effects**: `zdb_CVisual_ApplyDecalRender` `0x003d5aa0`, `zdb_CMeshDecalPool_Uninit`,
  `LensFlareManager` `0x001ff060`, the `Particle` source and instance routines (`0x00344bc0`..`0x00348030`),
  `GetTextureByName` `0x001fba30`, `zdb_CClutter_GetTranslate`, `CFader_SetMinBrightness`.

Nothing of his reaches the VU1 microprograms, the GS packet builders or the path-3 uploads that our render research
(research/07, 12, 13, 15, 81, 83) lives in. His notes there are uses, not identities: "render texture fnc1" on the
runtime's `node2` `0x00355f50`, "maprender" on `zdb_CGrid_addOrderedCellAtom` `0x002f4060`, fog start, distance and
height loads in the player-camera function `0x00295600` (r0001 `0x00294070` relinked-body, unnamed by us).

## Voice and speech recognition

56 new names. Two groups:

- **The headset and capture path**: `flushbuffer` `0x001e7f58`, `closemic` `0x001e8098`, `getsamples` `0x001e80f0`
  (the last two already ours on r0001, agreeing), `lgAudAEnumHint` `0x002455c0`, `Headset_StaticSoundSysCallback`
  `0x00368b70` (no r0001 twin), `mics1` `0x003f71d0` (his scratch label on the mic settings code).
- **The speech recogniser** (the SASE neighbour of research/56): 40 `iARCHI_*` entries at `0x00655658`..`0x00655f18` --
  the recogniser's command table (change context, enable/disable input, start/stop detection, flush, go to sleep,
  load language, the distance-calculation, gender, trigger and VAD parameter get/set, the decoder switches) -- the
  8 kHz / 11 kHz spline and beta tables `aQRSpline8kHz`, `aBeta8kHz`, `aQRSpline11kHz`, the context-tree and sentence
  confidence routines (`CreateCtxTree` `0x004d52f8`, `CalcSentenceConfidence` `0x004decc0`), and 27 data labels on the
  front end's parameter tables (frame window, FFT, mel, VAD, spectral subtraction, pitch, gender) at
  `0x006567f0`..`0x00656ab0`. 47 of the 56 carry to r0001 exactly: this library did not move between pressings.

For our audio work the value is orientation: the voice-command recogniser is a separate library from the voice chat
codec, and both are now bounded by name.

## The UI command layer

Ten of his eighteen aliases ([names.md](names.md)) sit on UI command functions we named from the menu script's
binding table (`ui-binding`, `ui-binding-derived`): `joinmproom` = `UIJoinMPRoom`, `CreateMPGame` = `UICreateMPGame`,
`IsSessionMaster` = `UIIsSessionMaster`, and so on. Those two passes are the ones with no demo behind them, so an
outside labeller landing on the same functions is real corroboration for them. Two of the five real conflicts
(`0x0027b310`, `0x0027ba30`) are in this layer as well. New here: `AdvanceMPMission` `0x0027a480`,
`CanInvitePlayerToClan` `0x0027b8f0`, `Readytoload` `0x0027dd00`, `GetMapName`
`0x00280b10`, `SetMpStatUiVars` `0x00224670` -- all r0004-only or seed-placed.

## Weapons, the kit and the player

His readings over the weapon code, which no pass of ours reached (r0001 twins mostly exact):

| r0004 | name | r0001 twin (how) |
|---|---|---|
| `0x003e6d90` | `GetWeaponID` | `0x003c4c10` exact |
| `0x003f13f0` | `GetWeaponInfo` | `0x003cf1f0` exact |
| `0x003f3bf0` / `0x003f4a90` | `GetWeaponName` (two) | exact / seed+delta |
| `0x003f3cb0` | `GetCrosshairType` | `0x003d1a60` exact |
| `0x005cdee0` | `GetWeaponObject` | `0x005c67d0` exact |
| `0x005cfd50` | `DoesPlayerHaveItem` | `0x005c8640` exact |
| `0x005c4b60` | `weaponSetUpWeaponsForSpawn` | `0x005bdfb0` exact |
| `0x005c0100` | `DisplayBullets` | `0x005b9990` exact |
| `0x005c77f0` | `HandleWeaponStuff` | `0x005c0ae0` relinked-body |
| `0x005a50c0` | `PickupWeapon` | `0x0059f610` relinked-body |
| `0x0033d320` / `0x003ed340` | `SpawnWeaponObject` / `SpawnProjectileObject` | seed+delta / relinked-body |
| `0x005c3be0` | `CZKit_FireGrenade` (his note: "to spawnProjectile, c4 check") | `0x005bd030` exact |
| `0x005c4a70` | `CZKit_SetItemAmmo` (his note: "set ammo count") | `0x005bdec0` exact |

With the PlayerObject's weapon, ammo, magazine and bloom fields ([objects.md](objects.md)) this is the first map of
the weapon state on our side. The message feed is named too: `PrintMessageFeed` `0x002b72a0` and
`PrintMessageToMessageFeed` `0x002b7fa0` (relinked-body twins), with 35 and 20 of his in-body notes walking the line
layout and colours -- relevant to the kill readout (research/22).

## AI, animation and the world

The 130 mangled game-class names he placed spread over Zipper's classes: `CZSealBody` (9: aim, reticule, death
animation, mission start, MP bomb gear, ghost state), `zdb_CNode` (7, the scene graph), `CSealCtrlAi` (6), `CZAnimMain`
(4), `CZKit` (4), the `zmath` static initialisers (4), `CEntity`, `CMission`, `CZBombState`, `zdb_CGrid`,
`zdb_CWorld`, `CAiMap`, `CSealUnit` (3 each) and a long tail. 102 of the 130 carry to r0001 exactly. They are demo
identities he placed ([caveats.md](caveats.md)); the `exact` ones are as safe as our own `exact` pass, the others
want the same scrutiny as our inferred passes. His readings add `PlayAnimation` `0x0058e160`, the lean-animation
selector `0x005829f0` (his notes list the stand/prone lean states), `GetPlayerDynamicsStart` `0x00592550` and the
look-bone helpers `qlookhead`/`qlookneck`/`qlookspinelo` (`0x0064ee50`..`0x0064efa0`, r0004-only).

## Where his in-body notes cluster

His 1,914 free-text notes and 414 numbered cross-references sit on instructions and data. The densest sit inside
functions we have not named; with what they describe (each is a ready-made reading guide for a body read):

| r0004 | r0001 twin | ours | his notes |
|---|---|---|---|
| `0x002b72a0` | `0x002b5830` | -- (his `PrintMessageFeed`) | 35: message feed layout -- counters, strings, spacing, Y offset |
| `0x003984e0` | `0x00379f70` | -- | 30: text rendering -- new line, colour codes, float colours |
| `0x002269a0` | `0x002269a0` | -- (his `etags`) | 29: the name-tag pass -- entity, player check, distance, team, hostages, screen coords |
| `0x002d19c0` | none | -- (his `UpdateStatsFields`) | 28: the stats copy, with the PlayerObject offsets |
| `0x00599050` | none | -- | 27: an online-checked action chooser -- team check, rifle action, body pickup, victory dance |
| `0x005cc9d0` | none | -- (his "spawn projectile") | 27: projectile spawn -- player object, weapon slot, position, direction, grenade timer |
| `0x002828e0` | `0x002815d0` | -- | 20: weapon enable flags |
| `0x00223970` | none | -- (his "UpdateStats GameEnd") | 19: the end-of-game stats roll-up |
| `0x00317f00` | none | -- (his `MediusUpdateAccountStats`) | 19: the account stats upload fields |
| `0x00295600` | `0x00294070` | -- | 18: the player camera and fog |
