# Keyboard-and-mouse play for the macOS fork -- design

Date: 2026-10-12. Status: owner's request (relayed), fork only, never upstream; experimental, work in progress, to be continued over time. Branch `feat-kbm-play` off `main`
(cf449442 + the changelog). Builds on mouse look (`socom2_mouse.*`, upstream #133, R349: off upstream).

## What the owner asked

1. Mouse look on by default in the fork, the pause-menu gate finished.
2. Every SOCOM II control on the keyboard and mouse, documented, with a remap file; menu navigation by mouse if
   cheap.
3. Movement speed on the mouse wheel (Splinter Cell PC): the wheel steps the walk speed, WASD moves at that
   speed by scaling the left stick; the lowest step must still move; the speed shown on screen.

## What is already there (2026-10-12)

- `main` has all of `feat-mouse-look` plus the menu-state gate (`m_menustate` at controller +0x221, 0 = play) and
  its test `Socom2MouseMenuState`: capture, R1, aim-hold and direct look are all gated. Item 1's open review item
  is done; only the default remains.
- The keyboard: `launcher/mapping.h`'s table (frozen on purpose: the harness posts its keys and gates pin its
  hash) under R210's scope -- full in developer mode, menus only for a player. `run.sh` sets `PS2X_DEV=1`.

## The game's controls (read from the disc's own preset screen, 2026-10-12)

Preset 1 "Precision Shooter" (the default): L1 quick-select primary weapon; L2 quick-select secondary weapon; R1
fire weapon / throw grenade; R2 open/close weapon selection; Triangle change stance (pressure: light toggles
stand/crouch, firm goes prone or stands from prone, `host_crouch_shortcut.h`); Circle command teammates; Cross
execute command / pick up weapon (and every "press X" prompt); Square jump; D-pad Up zoom sniper scope / NVG;
D-pad Left/Right peek; D-pad Down zoom out; SELECT TacMap / objectives; START pause; left stick move/strafe, L3
change fire rate; right stick aim / rotate, R3 force reload.

## Measured: the left stick (self-run, r0001, Seeding Chaos spawn, 2026-10-12)

Scripted holds (`PS2X_SOCOM2_INPUT_SCRIPT`), position peeks at 5 Hz (`*0x408c58+0x1c`):
- No movement at push 0.297 and 0.3125 (LY 90, 88); movement at 0.344 (LY 84). **Dead zone ~0.33**, consistent
  with the right stick's ~50/127 (mouse-look research).
- Crouched (the spawn): ~10 units/s at every push from 0.34 to 0.47 -- the fixed crouch walk below 0.838.
- Standing, through the layer itself (`PS2X_KBM_SCRIPT` + `PS2X_KBM_TRACE`, steady seconds): push 0.36 ~7, 0.45
  ~15, 0.50 ~19, 0.55 ~32, 0.60 ~39, 0.65 ~45, 0.70 ~53 units/s; full push ~64 (the run). Roughly linear above the
  dead zone, saturating near 0.78. Crouched (the spawn): ~10.5 at every push under 0.838, then it stands and runs.
- `actor+0x240` (the axis copy named in research/79) reads 0 on r0001: not the field there.

## Design

A fork-only layer, `socom2_kbm`, between the keyboard/mouse and the pad state. Upstream's table and R210 stay
untouched; the layer replaces the keyboard step when it is on.

- **On/off.** `PS2X_KBM` (Flag, fork default 1). Off for a scripted or replayed run (`PS2X_SOCOM2_INPUT_SCRIPT` /
  `_FILE` set) unless the knob says 1 -- the same rule as the mouse's final review. Off is exactly today's path.
  On: the keyboard scope is Full and mouse look defaults on (`PS2X_MOUSE` still overrides either way).
- **Contexts.** *Game*: an actor whose controller validates and `m_menustate == 0` (the mouse module's
  predicate). *Menu*: everything else (front end, pause, other menus). Each context has its own bindings.
- **Bindings.** An action is a PS2 button, a stick direction, or a special: `crouch` (light Triangle), `prone`
  (firm Triangle), `aim_hold` (the mouse module's right-click aim-hold), `speed_up` / `speed_down` (one step),
  `run` (full push while held). An input is a keyboard key (`launcher::mapping` key names), `mouse_left`,
  `mouse_right`, `mouse_middle`, `mouse_4`, `mouse_5`, `wheel_up`, `wheel_down`. Several inputs per action.
- **Defaults** (game): WASD move; mouse look; left click fire (R1); right click aim-hold; wheel speed; Shift
  run; Space jump (Square); C crouch; Left Ctrl prone; F and Enter execute / pick up / continue (Cross); T
  command teammates (Circle); R reload (R3); B fire rate (L3); Tab weapon selection (R2); 1 / 2 primary /
  secondary (L1 / L2); Q / E peek (D-pad Left / Right); middle click and Z zoom scope / NVG (D-pad Up); X zoom out
  (D-pad Down); M TacMap (SELECT); Esc pause (START); arrows the d-pad; IJKL the right stick.
  (menu): arrows and WASD the d-pad; wheel up/down the d-pad; Enter, Space, F and left click select (Cross);
  Backspace and right click back (Triangle); Esc START; Q / E L1 / R1 (text scrolling); Z Square; C Circle.
- **Wheel speed.** Steps of left-stick push (default 0.43, 0.53, 0.60, 0.69, 1.00 -- ~13/26/39/52/64 units/s
  standing, evenly spaced from the measurement; start at the last). WASD
  writes `128 -/+ round(push x 127)` on the held axes; a diagonal scales both so its length is the step's push.
  A wheel notch is one step, clamped (no wrap). The lowest step is refused below 0.36 (the dead zone plus a
  margin) when the file is read.
- **Indicator.** A line drawn by the present loop (beside the FPS overlay): "SPEED 3/5  60%", for 2.5 s after a
  change (`indicator = change`), always (`always`) or never (`off`).
- **Menu mouse.** Wheel and buttons in the menu context are edge pulses (two pad reads down, two up, the mouse
  module's `PulseQueue` timing), so one notch is one menu step. No pointing at items: the game's menus have no
  hit areas to find.
- **The remap file.** `controls.ini` next to the game executable (`PS2X_KBM_FILE` names another). Absent: the
  game writes the defaults there, commented, once. Sections `[game]`, `[menu]`, `[speed]` (`steps`, `start`,
  `indicator`), `[mouse]` (`sensitivity`, `aim_sensitivity`, `invert_y`; the knobs win when set). A line it
  cannot read is reported by number and ignored; the rest loads.
- **Trace.** `PS2X_KBM_TRACE`: `[kbm]` lines on a step change and every second while moving -- the step, the push,
  the axes written, and the actor's measured ground speed (position words +0x1c/+0x24 over the last second).

## Play-test 1 (2026-10-13, owner)

The movement scheme works. Asked for: the right button should zoom into the scope. Added `aim_zoom` (`[mouse]`,
default `scope`): the aim-hold sends UP until the view mode reaches a zoom level (research/87: 2 and up; the default
rifle goes 0 -> 1 -> 5), at most three presses, a dropped UP re-sent once, and a weapon with no zoom stays in first
person. The release restores the stored mode as before. `first_person` keeps the one-step behaviour (upstream's).

## Play-test 2 feedback (2026-10-13, owner, relayed; on hold: diagnosis only)

1. **Wheel speed misses notches / changes slowly.** Read-only causes, most likely first:
   - raylib 5.5's GLFW scroll callback *assigns* `currentWheelMove` (rcore_desktop_glfw.c `MouseScrollCallback`),
     so every scroll event but the last in a frame is lost. macOS sends several per notch from smooth-scrolling
     mice and the trackpad, and GLFW scales precise deltas by 0.1 (cocoa_window.m `scrollWheel`): a notch can
     arrive as fractions that mostly vanish before `socom2KbmFrame` sums them to 1.0.
   - `socom2KbmFrame`'s fraction carry never resets: residue in one direction eats the first notch back.
   - Notches count only in the game context and with focus; in a popup they are menu d-pad pulses.
   - Crouched, steps 1-4 are one speed (play-test 1), so a registered step can look like a missed one.
   Fix: chain our own GLFW scroll callback (`GetWindowHandle()`, `glfwSetScrollCallback` keeping raylib's) that
   adds every event into an atomic; one step per non-precise event (sign only, no acceleration overshoot);
   precise deltas accumulate to a threshold, the carry cleared on a direction change or 250 ms idle. Trace raw
   events per frame under `PS2X_KBM_TRACE`. Then the stance item (stand on a wheel step while crouched, or show it).
2. **Some clicks do not fire.** Read-only causes:
   - The left button is a *level* sampled twice: `IsMouseButtonDown` once per presented frame (~54/s) into an
     atomic, then the game thread reads it once per pad read (~28/s). A click whose down time falls between two
     pad reads (short taps, ~30-50 ms) never reaches R1; two quick clicks with no read in the gap merge into one
     press (a semi-auto weapon fires once).
   - Recapture swallows: after focus loss or Esc, the click that recaptures is eaten, and `m_swallow` holds until
     *both* buttons are up, so left clicks while right is held after a recapture are dropped.
   - Not the zoom: aim-hold pulses only the D-pad. The game may refuse fire during the zoom-in transition
     (unmeasured).
   Fix: the main thread counts press edges (`IsMouseButtonPressed`) per button into atomics; the game thread
   turns each counted press into at least 2 reads held + 1 released (a per-button stretcher), on top of the live
   level for held fire. Swallow only the recapture click's own button. Trace presses vs R1 reads.

Built 2026-10-13 (owner's go): `WheelCounter` and `ClickStretch` in the core; an NSEvent scroll monitor
(`socom2ScrollMonitorStart`, the GCMouse file; raylib's wheel elsewhere); counted presses per button and context;
`Capture` swallows only the recapturing button (`pressedMask` / `downMask`); `[speed] crouched = stand | keep`
with the stance byte (1 = crouched, measured: the spawn and every crouch walk read 1, also while the top step runs).
Traces: `[kbm] scroll ...` per event and `[kbm] click left presses= level= sent=` per change.

Play-test 3 (2026-10-13, e5daa95a): mouse look dead; owner: fix later, on hold (no diagnosis yet).

## Tests (failing first, `ps2x_tests`, no window)

The pure core (`runtime/socom2_kbm_core.h`): the ini parser (defaults, every section, bad lines kept out, unknown
names, round trip of the written defaults); the speed selector (clamp, start, the floor); the WASD axes
(cardinal, diagonal, opposite keys cancel, run); stance (crouch light, prone firm, a full Triangle wins); the
contexts (menu bindings only in menu; wheel and clicks as pulses); the enable rule (scripted off, knob wins).

## Out of scope

Mouse pointing at menu items; launcher UI for the remap; Windows/Linux (the layer is portable, untested there);
upstream.
