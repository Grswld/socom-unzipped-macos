# Keyboard and mouse controls (macOS fork)

> **Experimental: work in progress, to be continued over time** (branch `feat-kbm-play`, not for upstream). Known issues: mouse look did not work
> in play-test 3 (e5daa95a), not yet diagnosed; untested in play: the stand-up on a crouched wheel step, and the
> scroll monitor on a real wheel mouse. Also pending: faster scope zoom, the scope staying on after a popup, hold
> breath in the scope.

The macOS fork plays SOCOM II with a keyboard and a mouse, with no gamepad needed. The layer is on by default
(`PS2X_KBM=1`). It stays off in a scripted or replayed run unless the knob turns it on, so harness runs are
unchanged. `PS2X_KBM=0` gives upstream's keyboard (`launcher/mapping.h`, developer mode). Design:
`docs/superpowers/specs/2026-10-12-kbm-play-design.md`.

The functions are the game's own preset 1, "Precision Shooter" (the default, read from its Controller Presets
screen). Another preset moves the functions between the PS2 buttons; rebind in `controls.ini` to match.

## In a mission

| Control | Keyboard / mouse | PS2 button |
|---|---|---|
| Move / strafe | W A S D | left stick |
| Walk speed | mouse wheel (5 steps), Left Shift = full speed while held | left stick push |
| Aim / rotate | mouse (direct look); I J K L also turn | right stick |
| Fire weapon / throw grenade | left click | R1 |
| Aim through the scope while held (back on release) | right click | D-pad Up / Down |
| Jump | Space | Square |
| Crouch / stand | C | Triangle, light |
| Prone / stand from prone | Left Ctrl | Triangle, firm |
| Execute command / pick up weapon / "press X" | F, Enter | Cross |
| Command teammates | T | Circle |
| Force reload | R | R3 |
| Change fire rate | B | L3 |
| Open / close weapon selection | Tab | R2 |
| Quick-select primary / secondary weapon | 1 / 2 | L1 / L2 |
| Peek left / right | Q / E (and the left and right arrows) | D-pad Left / Right |
| Zoom sniper scope / NVG | middle click, Z (and the up arrow) | D-pad Up |
| Zoom out | X (and the down arrow) | D-pad Down |
| TacMap / objectives | M | SELECT |
| Pause | Esc | START |

## In menus (the front end, pause and every other menu)

| Control | Keyboard / mouse | PS2 button |
|---|---|---|
| Move the selection | arrows, W A S D, mouse wheel | D-pad |
| Select | Enter, Space, F, left click | Cross |
| Back | Backspace, right click | Triangle |
| Start / resume | Esc | START |
| Scroll text | Q / E | L1 / R1 |
| Other prompts | Z (Square), C (Circle), M (SELECT) | |

The mouse in a menu is one step per wheel notch and one press per click; it does not point at items. Every click
reaches the game, however short: a tap between two of the game's pad reads (about 28 a second) is held for two reads.

## Walk speed on the wheel

The wheel steps the left stick's push, so W A S D move at the chosen speed, the way the Splinter Cell PC port works.
A notch is one step, and the steps stop at both ends. The game reports the change at the bottom of the screen
("SPEED 3/5  60%") for 2.5 s.

Measured standing on r0001 (2026-10-12, `PS2X_KBM_TRACE`):

| Step | Push | Speed (units/s) |
|---|---|---|
| 1 | 0.43 | ~13 |
| 2 | 0.53 | ~26 |
| 3 | 0.60 | ~39 |
| 4 | 0.69 | ~52 |
| 5 | 1.00 | ~64 (run) |

- **Dead zone:** the game ignores a push under about 0.33. A step below 0.36 is raised to 0.36 when the file is
  read, so the lowest step always moves.
- **Crouched:** the game has a single crouch-walk speed (about 10-14 units/s), and the top step, like a full stick
  on the console, stands the SEAL up and runs. So a wheel step below the top stands a crouched SEAL up first
  (`crouched = stand` in `[speed]`, the default); `crouched = keep` leaves the stance alone.
- **Every notch counts:** the game reads each scroll event (macOS), one step per notch of a wheel however fast it
  turns; a trackpad or Magic Mouse steps once per 30 points of scrolling, and its glide after the finger lifts is
  ignored.

## controls.ini

The game reads `controls.ini` beside the executable (`dist-macos/controls.ini`), or the file `PS2X_KBM_FILE`
names. When the file is absent, the game writes it with the defaults above and comments.

- **Bindings:** `[game]` and `[menu]` hold one line per action: `action = input, input`. A line replaces that
  action's inputs, an empty value unbinds it, and an action the file does not name keeps its default.
- **Inputs:** letters and digits as themselves; `space enter escape backspace tab up down left right`;
  `left_shift left_ctrl left_alt left_cmd` (and `right_*`); `f1`-`f12`; `kp_0`-`kp_9`; `mouse_left mouse_right
  mouse_middle mouse_4 mouse_5 wheel_up wheel_down`; or `key:<GLFW code>`.
- **Actions:**
  - the PS2 buttons: `up down left right cross circle square triangle l1 l2 l3 r1 r2 r3 start select`;
  - movement and look: `move_forward move_back move_left move_right look_up look_down look_left look_right`;
  - the layer's own: `crouch prone aim_hold speed_up speed_down run`.
- **`[speed]`:** `steps` is 1 to 10 pushes in (0, 1], `start` is the step a session begins on (1-based),
  `indicator` is `change`, `always` or `off`, and `crouched` is `stand` or `keep`.
- **`[mouse]`:** `sensitivity`, `aim_sensitivity` and `invert_y`; the `PS2X_MOUSE_*` knobs win when they are set.
  `aim_zoom = scope` (the default) makes the held right button zoom into the weapon's first scope level;
  `first_person` steps in only to first person. A weapon with no scope stops at first person.
- **Errors:** a line the game cannot read is logged as `[kbm] controls.ini line <n>: ...` and ignored; the rest
  of the file still loads.

A natural-scrolling trackpad or Magic Mouse turns the wheel the other way. To make "up" speed up, swap `wheel_up`
and `wheel_down` in `[game]`.

## Knobs

| Knob | What |
|---|---|
| `PS2X_KBM` | 1 (default) the layer; 0 upstream's keyboard |
| `PS2X_KBM_FILE` | another controls file |
| `PS2X_KBM_TRACE` | `[kbm]` lines: the step, the push, the axes and the measured ground speed every second |
| `PS2X_KBM_SCRIPT` | `t:input[:hold],...`: timed inputs for a self-run (macOS does not let a script post keys) |
| `PS2X_MOUSE` | mouse look; it follows `PS2X_KBM`. With `PS2X_MOUSE=0` nothing captures the cursor, so the mouse buttons work in menus only |
