# 90 -- The PCSX2 door spike: five questions on a fresh PCSX2 (Sprint 18 T0)

Run 2026-10-12 08:09-08:16Z in R352's window, under the loop lock, by the main-tree controller. The task book's T0
(`docs/superpowers/plans/2026-10-01-sprint-18-tasks.md`) numbered this note 83; 83-89 were taken by then, so it is 90.

**The install.** The task book fetched the release by hand; R366 granted one GitHub fetch, and that one is T7's
INSTALL. So the spike ran on a fresh portable folder made from the program files of the harness's PCSX2 (`tools/pcsx2`,
the same v2.8.2 INSTALL fetches): no `inis/`, `memcards/`, `cache/`, `logs/` or `portable.ini` carried over;
`portable.txt`, the BIOS (`ps2-0200a`, `ps2-0220a`), the master pnach `scripts/parity/pcsx2/0F6FC6CF.pnach` in
`patches/`, and an ini holding only what the launcher writes. `EthDevice` and `[MemoryCards]` were left out on purpose.
The disc was the r0001 ISO; the command `pcsx2-qt.exe -batch -- <iso>`. Scratch: `logs/s18_spike/` (git-ignored):
`emulog.txt`, `PCSX2.ini.after`, `run1_settings_prompt.png`, `boot_150s.png`, `run1_PCSX2.ini.after`.

| # | Question | Answer | Read from | Consequence |
|---|---|---|---|---|
| 1 | Does Windows' own `tar.exe` (bsdtar) extract the release `.7z`? | **Yes**, answered by T7's INSTALL (08:18Z): `pcsx2-v2.8.2-windows-x64-Qt.7z` extracted with `pcsx2-qt.exe` and its DLLs at the top level, no subfolder; INSTALL wrote `portable.txt` and its marker and left no `.7z` | `logs/s18_proof/install.log`; `home/pcsx2/` | T4's extract step stands as written |
| 2 | Does `-batch` boot straight into the disc? | **Only with the `[UI]` seed.** Run 1 (an ini with `[DEV9/Eth]` alone) stopped at a modal before any boot: "Settings failed to load, or are the incorrect version. Clicking Yes will reset all settings to defaults. Do you want to continue?" -- no emulog, no card, the ini untouched. Run 2, with `[UI] SettingsVersion = 1` and `SetupWizardIncomplete = false` added, booted: `ELF cdrom0:\SCUS_972.75;1 with entry point at 0x00180008 is executing` at 3.8 s, the game at its "Select MEMORY CARD slot" screen by 150 s | `run1_settings_prompt.png`; `emulog.txt` | T10 (R347) is required, not a nicety: without the seed a first boot asks to reset everything, and Yes would wipe the `[DEV9/Eth]` merge |
| 3 | Does PCSX2 load our pnach from `patches/`? | **Yes**: "Found 1 game patches in ...\patches\0F6FC6CF.pnach", "8 game patches are active"; the bundled `patches.zip` set disabled because an unlabeled patch loaded | `emulog.txt` | T3's write of the master pnach into `patches/` under the CRC's name is the right place |
| 4 | Are memory cards made without a `[MemoryCards]` section? | **Yes**: two 8 MB cards created in `memcards/` (`Mcd001.ps2` slot 0, `Mcd002.ps2` slot 1), unformatted; the game formats on its own prompt | `emulog.txt`; `memcards/` | T3 writes nothing for cards; the game's own format prompt is the player's first step |
| 5 | Does DEV9 find an adapter with no `EthDevice`? | **No**: "DEV9: Socket: Failed to Get Adapter", "Failed to GetNetAdapter()", at open and again at boot | `emulog.txt` | The launcher must write `EthDevice`; T4/T6 do (the `GetAdaptersAddresses` pick, the ADVANCED row) |

**Also seen.** PCSX2 leaves `[DEV9/Eth]` byte-identical after a boot (`PCSX2.ini.after`), so the merge survives a
run once the seed is in. After 150 s a `CloseMainWindow` did not end the process within 30 s, and the spike killed it. T7 found
why: closing the window opens PCSX2's own "Confirm Shutdown" dialog ("Are you sure you want to shut down the virtual
machine?", Save State For Resume unticked); after Yes the process exits and LAUNCH's LAST RUN reads "PCSX2 closed". The BIOS's
`.nvm` was missing from the copy and PCSX2 made a default one without complaint.
