// Keyboard-and-mouse play (macOS fork, spec docs/superpowers/specs/2026-10-12-kbm-play-design.md).
#include "MiniTest.h"
#include "runtime/socom2_kbm_core.h"

#include <set>
#include <string>

using namespace socom2_kbm;
using launcher::mapping::kPs2Select;
using launcher::mapping::kPs2L3;
using launcher::mapping::kPs2R3;
using launcher::mapping::kPs2Start;
using launcher::mapping::kPs2Up;
using launcher::mapping::kPs2Right;
using launcher::mapping::kPs2Down;
using launcher::mapping::kPs2Left;
using launcher::mapping::kPs2L2;
using launcher::mapping::kPs2R2;
using launcher::mapping::kPs2L1;
using launcher::mapping::kPs2R1;
using launcher::mapping::kPs2Triangle;
using launcher::mapping::kPs2Circle;
using launcher::mapping::kPs2Cross;
using launcher::mapping::kPs2Square;

namespace
{
    std::set<int> g_down;   // the keys the predicate reports down
    bool keyDown(int key) { return g_down.count(key) != 0; }

    Inputs keys(std::initializer_list<int> down, uint8_t mouse = 0, int wheelUp = 0, int wheelDown = 0)
    {
        g_down = std::set<int>(down);
        Inputs in;
        in.isKeyDown = keyDown;
        in.mouseButtons = mouse;
        in.wheelUp = wheelUp;
        in.wheelDown = wheelDown;
        return in;
    }

    State freshState(const Settings &s)
    {
        State st;
        st.speed.reset(s);
        return st;
    }

    bool hasBinding(const std::vector<Binding> &table, int input, uint8_t action)
    {
        for (const Binding &b : table)
            if (b.input == input && b.action == action)
                return true;
        return false;
    }

    constexpr uint8_t kLeftClick = 1u << 0;
    constexpr uint8_t kRightClick = 1u << 1;
    constexpr uint8_t kMiddleClick = 1u << 2;
}

void register_socom2_kbm_tests()
{
    MiniTest::Case("Socom2KbmNames", [](TestCase &tc)
    {
        tc.Run("inputs round-trip through their names: keys, modifiers, the mouse and the wheel", [](TestCase &t)
        {
            for (int input : {static_cast<int>('W'), 32, 257, 256, 340, 341, 290, kMouseLeft, kMouseRight, kMouseMiddle,
                              kMouse4, kMouse5, kWheelUp, kWheelDown})
                t.Equals(inputFromName(inputName(input)), input, "round trip of " + inputName(input));
            t.Equals(inputFromName("W"), static_cast<int>('W'), "a capital letter names the key too");
            t.Equals(inputFromName(" left_shift "), 340, "spaces around a name are ignored");
            t.Equals(inputFromName("nonsense"), kUnknownInput, "an unknown name is refused");
        });

        tc.Run("actions: the sixteen PS2 buttons by libpad2 name, then the layer's own", [](TestCase &t)
        {
            t.Equals(actionFromName("r1"), static_cast<int>(kPs2R1), "r1");
            t.Equals(actionFromName("triangle"), static_cast<int>(kPs2Triangle), "triangle");
            t.Equals(actionFromName("speed_up"), static_cast<int>(kSpeedUp), "speed_up");
            t.Equals(actionFromName("aim_hold"), static_cast<int>(kAimHold), "aim_hold");
            for (int a = 0; a < kActionCount; ++a)
                t.Equals(actionFromName(actionName(static_cast<uint8_t>(a))), a, std::string("round trip of ") + actionName(static_cast<uint8_t>(a)));
            t.Equals(actionFromName("fly"), -1, "unknown action");
        });
    });

    MiniTest::Case("Socom2KbmDefaults", [](TestCase &tc)
    {
        tc.Run("every PS2 button and every layer action has a game binding (the owner's 'every control'; Triangle by crouch/prone)", [](TestCase &t)
        {
            const Settings s = defaults();
            for (int a = 0; a < kActionCount; ++a)
            {
                bool found = false;
                for (const Binding &b : s.game)
                    found = found || b.action == a || (a == kPs2Triangle && (b.action == kCrouch || b.action == kProne));
                t.IsTrue(found, std::string("no game binding for ") + actionName(static_cast<uint8_t>(a)));
            }
        });

        tc.Run("the preset-1 layout: fire, aim, jump, stance, reload, use, pause, TacMap", [](TestCase &t)
        {
            const Settings s = defaults();
            t.IsTrue(hasBinding(s.game, kMouseLeft, kPs2R1), "left click fires");
            t.IsTrue(hasBinding(s.game, kMouseRight, kAimHold), "right click aims");
            t.IsTrue(hasBinding(s.game, kWheelUp, kSpeedUp) && hasBinding(s.game, kWheelDown, kSpeedDown), "the wheel steps speed");
            t.IsTrue(hasBinding(s.game, ' ', kPs2Square), "space jumps");
            t.IsTrue(hasBinding(s.game, 'C', kCrouch) && hasBinding(s.game, 341, kProne), "C crouches, ctrl goes prone");
            t.IsTrue(hasBinding(s.game, 'R', kPs2R3), "R reloads");
            t.IsTrue(hasBinding(s.game, 'F', kPs2Cross) && hasBinding(s.game, 257, kPs2Cross), "F and Enter use");
            t.IsTrue(hasBinding(s.game, 256, kPs2Start), "Escape pauses");
            t.IsTrue(hasBinding(s.game, 'M', kPs2Select), "M opens the TacMap");
            t.IsTrue(hasBinding(s.menu, kMouseLeft, kPs2Cross) && hasBinding(s.menu, kMouseRight, kPs2Triangle), "menu clicks select and go back");
            t.IsTrue(hasBinding(s.menu, kWheelUp, kPs2Up) && hasBinding(s.menu, kWheelDown, kPs2Down), "the menu wheel scrolls");
        });

        tc.Run("speed steps start at full and every step clears the dead zone", [](TestCase &t)
        {
            const Settings s = defaults();
            t.Equals(s.start, static_cast<int>(s.steps.size()) - 1, "starts at the top step");
            t.Equals(s.steps.back(), 1.0f, "the top step is full push");
            for (float f : s.steps)
                t.IsTrue(f >= kMinPush, "a step inside the dead zone");
        });
    });

    MiniTest::Case("Socom2KbmFile", [](TestCase &tc)
    {
        tc.Run("the written defaults parse back to the defaults, with no error", [](TestCase &t)
        {
            const ParseResult r = parse(write(defaults()));
            t.IsTrue(r.errors.empty(), "errors: " + (r.errors.empty() ? std::string() : r.errors.front()));
            t.IsTrue(r.settings == defaults(), "the round trip changed the settings");
            t.Equals(write(r.settings), write(defaults()), "and writes the same file again");
        });

        tc.Run("an empty file is the defaults", [](TestCase &t)
        {
            const ParseResult r = parse("");
            t.IsTrue(r.errors.empty() && r.settings == defaults(), "empty file");
        });

        tc.Run("a line replaces one action's inputs and leaves the rest; an empty value unbinds", [](TestCase &t)
        {
            const ParseResult r = parse("[game]\nr1 = mouse_left, key:70\njump_is_square = x\nsquare = \n");
            t.IsTrue(hasBinding(r.settings.game, kMouseLeft, kPs2R1) && hasBinding(r.settings.game, 70, kPs2R1), "r1 rebound to two inputs");
            bool square = false;
            for (const Binding &b : r.settings.game)
                square = square || b.action == kPs2Square;
            t.IsFalse(square, "square unbound");
            t.IsTrue(hasBinding(r.settings.game, 'R', kPs2R3), "an action the file does not name keeps its default");
            t.Equals(r.errors.size(), static_cast<size_t>(1), "the unknown action is one error");
            t.IsTrue(!r.errors.empty() && r.errors[0].rfind("line 3:", 0) == 0, "the error names its line");
        });

        tc.Run("a line with an unknown input is ignored whole and reported", [](TestCase &t)
        {
            const ParseResult r = parse("[game]\nr1 = mouse_left, warp_drive\n");
            t.Equals(r.errors.size(), static_cast<size_t>(1), "one error");
            t.IsTrue(r.settings.game == defaults().game, "r1 kept its default");
        });

        tc.Run("speed: steps sorted, the start 1-based and clamped, a step in the dead zone raised to the floor", [](TestCase &t)
        {
            ParseResult r = parse("[speed]\nsteps = 1.0, 0.5, 0.2\nstart = 2\nindicator = always\n");
            t.Equals(r.settings.steps.size(), static_cast<size_t>(3), "three steps");
            t.Equals(r.settings.steps.front(), kMinPush, "0.2 raised to the floor");
            t.Equals(r.settings.steps.back(), 1.0f, "sorted");
            t.Equals(r.settings.start, 1, "start 2 is index 1");
            t.IsTrue(r.settings.indicator == Indicator::Always, "indicator");
            t.Equals(r.errors.size(), static_cast<size_t>(1), "the raised step is reported");
            r = parse("[speed]\nstart = 9\n");
            t.Equals(r.settings.start, static_cast<int>(defaults().steps.size()) - 1, "start clamped to the top");
            r = parse("[speed]\nsteps = 0.5, fast\n");
            t.IsTrue(r.settings.steps == defaults().steps && r.errors.size() == 1, "a bad list keeps the default steps");
        });

        tc.Run("mouse settings are carried only when set; bad sections and lines are reported", [](TestCase &t)
        {
            ParseResult r = parse("[mouse]\nsensitivity = 1.5\ninvert_y = 1\n[nope]\nx = y\nstray\n");
            t.IsTrue(r.settings.hasSens && r.settings.sens == 1.5f, "sensitivity");
            t.IsTrue(r.settings.hasInvertY && r.settings.invertY, "invert");
            t.IsFalse(r.settings.hasAimSens, "aim sensitivity unset");
            t.Equals(r.errors.size(), static_cast<size_t>(2), "unknown section + the line without '='");
            r = parse("r1 = mouse_left\n");
            t.Equals(r.errors.size(), static_cast<size_t>(1), "a setting before any section");
        });

        tc.Run("[speed] crouched: stand by default, keep on request, written and read back", [](TestCase &t)
        {
            t.IsTrue(defaults().crouchStand, "stand");
            ParseResult r = parse("[speed]\ncrouched = keep\n");
            t.IsTrue(r.errors.empty() && !r.settings.crouchStand, "keep");
            t.IsFalse(parse(write(r.settings)).settings.crouchStand, "round trip");
            r = parse("[speed]\ncrouched = sometimes\n");
            t.IsTrue(r.errors.size() == 1 && r.settings.crouchStand, "a bad value is reported");
        });

        tc.Run("aim_zoom: scope by default, first_person on request, written and read back", [](TestCase &t)
        {
            t.IsTrue(defaults().aimScope, "the right button zooms into the scope by default");
            ParseResult r = parse("[mouse]\naim_zoom = first_person\n");
            t.IsTrue(r.errors.empty() && !r.settings.aimScope, "first_person");
            t.IsFalse(parse(write(r.settings)).settings.aimScope, "round trip");
            t.IsTrue(parse("[mouse]\naim_zoom = scope\n").settings.aimScope, "scope");
            r = parse("[mouse]\naim_zoom = max\n");
            t.IsTrue(r.errors.size() == 1 && r.settings.aimScope, "a bad value is reported and keeps the default");
        });
    });

    MiniTest::Case("Socom2KbmEnable", [](TestCase &tc)
    {
        tc.Run("on unless scripted; the knob wins either way", [](TestCase &t)
        {
            t.IsTrue(enabledFor(nullptr, false), "interactive: on");
            t.IsFalse(enabledFor(nullptr, true), "scripted: off");
            t.IsTrue(enabledFor("1", true), "scripted with PS2X_KBM=1: on");
            t.IsFalse(enabledFor("0", false), "PS2X_KBM=0: off");
            t.IsFalse(enabledFor("off", false), "the one flag rule: off is off");
            t.IsTrue(enabledFor("", false), "empty is unset");
        });
    });

    MiniTest::Case("Socom2KbmSpeed", [](TestCase &tc)
    {
        tc.Run("the wheel steps the walk speed down and up, clamped at both ends", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            PadOut o = apply(s, Context::Game, keys({'W'}), st);
            t.Equals(o.axis[3], static_cast<uint8_t>(0x00), "full forward at the top step");
            o = apply(s, Context::Game, keys({'W'}, 0, 0, 2), st);
            t.IsTrue(o.stepChanged, "two notches down change the step");
            t.Equals(st.speed.step(), 2, "step 3 of 5");
            t.Equals(o.axis[3], axisByte(-s.steps[2]), "W at the third step");
            o = apply(s, Context::Game, keys({'W'}, 0, 0, 9), st);
            t.Equals(st.speed.step(), 0, "clamped at the bottom");
            t.Equals(o.axis[3], axisByte(-s.steps[0]), "W at the lowest step");
            o = apply(s, Context::Game, keys({'W'}, 0, 0, 1), st);
            t.IsFalse(o.stepChanged, "no change below the bottom");
            o = apply(s, Context::Game, keys({'S'}, 0, 1, 0), st);
            t.Equals(o.axis[3], axisByte(s.steps[1]), "one notch up: S at the second step");
        });

        tc.Run("crouched: a wheel step below the top stands the SEAL up (light Triangle, two reads), once a second", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            Inputs in = keys({}, 0, 0, 1);
            in.stance = kStanceCrouched;
            PadOut o = apply(s, Context::Game, in, st);
            t.IsTrue(o.stepChanged, "the step changed");
            t.IsTrue(o.button[kPs2Triangle] != 0 && o.trianglePressure == kLightPressure, "light Triangle");
            in = keys({});
            in.stance = kStanceCrouched;
            o = apply(s, Context::Game, in, st);
            t.IsTrue(o.button[kPs2Triangle] != 0, "held a second read");
            o = apply(s, Context::Game, in, st);
            t.IsTrue(o.button[kPs2Triangle] == 0, "then released");
            in = keys({}, 0, 0, 1);
            in.stance = kStanceCrouched;   // the game has not caught up yet
            o = apply(s, Context::Game, in, st);
            t.IsTrue(o.stepChanged && o.button[kPs2Triangle] == 0, "a second notch inside the guard sends nothing");
            in = keys({});
            in.stance = kStanceCrouched;
            for (int i = 0; i < kStandGuardReads; ++i)
                apply(s, Context::Game, in, st);
            in = keys({}, 0, 0, 1);
            in.stance = kStanceCrouched;
            t.IsTrue(apply(s, Context::Game, in, st).button[kPs2Triangle] != 0, "after the guard it may stand again");
        });

        tc.Run("crouched: no stand for the top step, for a stance not crouched or unknown, or with crouched = keep", [](TestCase &t)
        {
            Settings s = defaults();
            State st = freshState(s);
            Inputs in = keys({}, 0, 0, 2);
            in.stance = kStanceCrouched;
            apply(s, Context::Game, in, st);   // stands (step 3)
            for (int i = 0; i < kStandGuardReads + 2; ++i)
                apply(s, Context::Game, keys({}), st);
            in = keys({}, 0, 2, 0);
            in.stance = kStanceCrouched;
            t.IsTrue(apply(s, Context::Game, in, st).button[kPs2Triangle] == 0, "back to the top step: no stand (it runs)");
            in = keys({}, 0, 0, 1);
            in.stance = 0;
            t.IsTrue(apply(s, Context::Game, in, st).button[kPs2Triangle] == 0, "standing: nothing");
            in.stance = -1;
            t.IsTrue(apply(s, Context::Game, in, st).button[kPs2Triangle] == 0, "unknown: nothing");
            s.crouchStand = false;
            State st2 = freshState(s);
            in = keys({}, 0, 0, 1);
            in.stance = kStanceCrouched;
            t.IsTrue(apply(s, Context::Game, in, st2).button[kPs2Triangle] == 0, "crouched = keep");
        });

        tc.Run("the lowest step still moves: its push clears the measured dead zone (0.33)", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            apply(s, Context::Game, keys({}, 0, 0, 9), st);
            const PadOut o = apply(s, Context::Game, keys({'W'}), st);
            t.IsTrue(128 - o.axis[3] >= 46, "deflection at least 46 of 128 (0.36)");
        });

        tc.Run("run holds full push whatever the step; a diagonal keeps the step's length", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            apply(s, Context::Game, keys({}, 0, 0, 2), st);   // the third step
            PadOut o = apply(s, Context::Game, keys({'W', 340}), st);
            t.Equals(o.axis[3], static_cast<uint8_t>(0x00), "shift: full");
            o = apply(s, Context::Game, keys({'W', 'D'}), st);
            const float d = s.steps[2] / std::sqrt(2.0f);
            t.Equals(o.axis[3], axisByte(-d), "diagonal forward component");
            t.Equals(o.axis[2], axisByte(d), "diagonal right component");
            o = apply(s, Context::Game, keys({'A', 'D'}), st);
            t.Equals(o.axis[2], static_cast<uint8_t>(0x80), "opposite keys cancel");
            t.IsFalse(o.moving, "nothing moves");
        });

        tc.Run("a speed key steps once per press, not once per read", [](TestCase &t)
        {
            Settings s = parse("[game]\nspeed_down = minus\n").settings;
            State st = freshState(s);
            apply(s, Context::Game, keys({45}), st);
            apply(s, Context::Game, keys({45}), st);
            apply(s, Context::Game, keys({45}), st);
            t.Equals(st.speed.step(), s.start - 1, "held for three reads: one step");
            apply(s, Context::Game, keys({}), st);
            apply(s, Context::Game, keys({45}), st);
            t.Equals(st.speed.step(), s.start - 2, "released and pressed again: a second step");
        });

        tc.Run("speed never steps in a menu, even when the file binds it there", [](TestCase &t)
        {
            const Settings s = parse("[menu]\nspeed_down = wheel_down, minus\n").settings;
            State st = freshState(s);
            apply(s, Context::Menu, keys({45}, 0, 0, 3), st);
            t.Equals(st.speed.step(), s.start, "unchanged");
        });

        tc.Run("the indicator shows the step for 2.5 s after a change, always, or never", [](TestCase &t)
        {
            Settings s = defaults();
            t.Equals(indicatorLine(s, 2, 1.0), std::string("SPEED 3/5  60%"), "shown after a change");
            t.Equals(indicatorLine(s, 2, 3.0), std::string(), "hidden later");
            s.indicator = Indicator::Always;
            t.Equals(indicatorLine(s, 4, 99.0), std::string("SPEED 5/5  100%"), "always");
            s.indicator = Indicator::Off;
            t.Equals(indicatorLine(s, 4, 0.0), std::string(), "off");
        });
    });

    MiniTest::Case("Socom2KbmWheel", [](TestCase &tc)
    {
        tc.Run("a notched wheel: every event is one step, however large its delta (no acceleration overshoot)", [](TestCase &t)
        {
            WheelCounter w;
            t.Equals(w.add({1.0, false, false, 0.00}), 1, "one notch up");
            t.Equals(w.add({0.1, false, false, 0.05}), 1, "a slow notch's small delta is still a notch");
            t.Equals(w.add({4.5, false, false, 0.06}), 1, "an accelerated notch is one step");
            t.Equals(w.add({-0.1, false, false, 0.07}), -1, "down");
            t.Equals(w.add({0.0, false, false, 0.08}), 0, "a zero delta is nothing");
        });

        tc.Run("precise deltas add up to a step; the carry clears on a direction change or when idle", [](TestCase &t)
        {
            WheelCounter w;
            const double s = kPreciseStep;
            t.Equals(w.add({s * 0.6, true, false, 0.00}), 0, "not yet");
            t.Equals(w.add({s * 0.6, true, false, 0.02}), 1, "the sum crosses a step");
            t.Equals(w.add({s * 2.0, true, false, 0.04}), 2, "a large precise delta is several steps");
            t.Equals(w.add({s * 0.7, true, false, 0.06}), 0, "0.2 left over + 0.7: not yet");
            t.Equals(w.add({-s * 0.5, true, false, 0.08}), 0, "a change of direction drops the carry");
            t.Equals(w.add({-s * 0.6, true, false, 0.10}), 1 * -1, "and the way back counts from zero");
            t.Equals(w.add({s * 0.9, true, false, 0.12}), 0, "carry up");
            t.Equals(w.add({s * 0.2, true, false, 0.12 + kWheelIdle + 0.01}), 0, "after an idle gap the carry is gone");
        });

        tc.Run("momentum (after the finger lifts) is ignored", [](TestCase &t)
        {
            WheelCounter w;
            t.Equals(w.add({kPreciseStep * 3.0, true, true, 0.0}), 0, "momentum");
            t.Equals(w.add({5.0, false, true, 0.1}), 0, "momentum on a non-precise event too");
        });
    });

    MiniTest::Case("Socom2KbmClicks", [](TestCase &tc)
    {
        // One pad read: the live level and the presses counted since the last read; the output is what the game sees.
        tc.Run("a tap shorter than a pad read still reaches the game for two reads, then one up", [](TestCase &t)
        {
            ClickStretch c;
            t.IsTrue(c.tick(false, 1), "the counted press: down");
            t.IsTrue(c.tick(false, 0), "held a second read");
            t.IsFalse(c.tick(false, 0), "then up");
            t.IsFalse(c.tick(false, 0), "and stays up");
        });

        tc.Run("two clicks between two reads are two presses with a release between them", [](TestCase &t)
        {
            ClickStretch c;
            std::string seen;
            seen += c.tick(false, 2) ? '1' : '0';
            for (int i = 0; i < 6; ++i)
                seen += c.tick(false, 0) ? '1' : '0';
            t.Equals(seen, std::string("1101100"), "down 2, up 1, down 2, up");
        });

        tc.Run("a held button keeps firing after its press (automatic fire)", [](TestCase &t)
        {
            ClickStretch c;
            t.IsTrue(c.tick(true, 1), "press");
            for (int i = 0; i < 10; ++i)
                t.IsTrue(c.tick(true, 0), "held");
            t.IsFalse(c.tick(false, 0), "released");
        });

        tc.Run("a new click while held gets its own edge", [](TestCase &t)
        {
            ClickStretch c;
            c.tick(true, 1);
            c.tick(true, 0);
            c.tick(true, 0);
            t.IsFalse(c.tick(true, 1), "a re-press (release and press between reads): one read up first");
            t.IsTrue(c.tick(true, 0), "then down again");
        });

        tc.Run("a level with no counted press still reaches the game", [](TestCase &t)
        {
            ClickStretch c;
            t.IsTrue(c.tick(true, 0), "down");
            t.IsFalse(c.tick(false, 0), "up");
        });

        tc.Run("presses pile up to a cap", [](TestCase &t)
        {
            ClickStretch c;
            int downs = 0;
            c.tick(false, 50);
            for (int i = 0; i < 200; ++i)
                downs += c.tick(false, 0) ? 1 : 0;
            t.Equals(downs + 1, 2 * kClickCap, "kClickCap presses of two reads each");
        });
    });

    MiniTest::Case("Socom2KbmScript", [](TestCase &tc)
    {
        tc.Run("timed holds and wheel notches; a bad event is reported and skipped", [](TestCase &t)
        {
            std::vector<std::string> errors;
            const std::vector<ScriptEvent> ev = parseScript("10:w:4, 15:wheel_down, 15.5:wheel_down, 20:mouse_left, 21:nope, x:w", &errors);
            t.Equals(ev.size(), static_cast<size_t>(4), "four events");
            t.Equals(errors.size(), static_cast<size_t>(2), "two refused");
            t.IsFalse(scriptHolds(ev, 'W', 9.9), "before");
            t.IsTrue(scriptHolds(ev, 'W', 12.0), "during");
            t.IsFalse(scriptHolds(ev, 'W', 14.0), "after the hold");
            t.IsTrue(scriptHolds(ev, kMouseLeft, 20.1), "default hold 0.25 s");
            t.IsFalse(scriptHolds(ev, kWheelDown, 15.0), "a wheel is never held");
            t.Equals(scriptWheel(ev, kWheelDown, 14.0, 16.0), 2, "two notches in the window");
            t.Equals(scriptWheel(ev, kWheelDown, 15.0, 16.0), 1, "the window is (from, to]");
        });
    });

    MiniTest::Case("Socom2KbmButtons", [](TestCase &tc)
    {
        tc.Run("game: keys and mouse buttons are held as long as they are down", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            for (int i = 0; i < 3; ++i)
            {
                const PadOut o = apply(s, Context::Game, keys({'R', ' '}, kLeftClick), st);
                t.IsTrue(o.button[kPs2R1] && o.button[kPs2R3] && o.button[kPs2Square], "R1, R3 and Square held");
            }
            const PadOut o = apply(s, Context::Game, keys({}, kRightClick | kMiddleClick), st);
            t.IsTrue(o.aimHold, "right click is aim-hold, not a button");
            t.IsTrue(o.button[kPs2Up], "middle click is D-pad Up");
            t.IsFalse(o.button[kPs2R1], "R1 released");
        });

        tc.Run("stance: crouch is a light Triangle, prone a full one, and full wins", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            PadOut o = apply(s, Context::Game, keys({'C'}), st);
            t.IsTrue(o.button[kPs2Triangle], "crouch presses Triangle");
            t.Equals(o.trianglePressure, kLightPressure, "light");
            o = apply(s, Context::Game, keys({341}), st);
            t.Equals(o.trianglePressure, kFullPressure, "prone is full");
            o = apply(s, Context::Game, keys({'C', 341}), st);
            t.Equals(o.trianglePressure, kFullPressure, "both: full wins");
            o = apply(s, Context::Game, keys({}), st);
            t.IsFalse(o.button[kPs2Triangle], "released");
        });

        tc.Run("menu: a click or a notch is one press -- held two reads, released two", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            std::vector<int> cross;
            cross.push_back(apply(s, Context::Menu, keys({}, kLeftClick), st).button[kPs2Cross]);
            for (int i = 0; i < 5; ++i)
                cross.push_back(apply(s, Context::Menu, keys({}, kLeftClick), st).button[kPs2Cross]);   // still held
            t.IsTrue(cross == std::vector<int>({1, 1, 0, 0, 0, 0}), "one press for one click held down");
            std::vector<int> down;
            down.push_back(apply(s, Context::Menu, keys({}, 0, 0, 2), st).button[kPs2Down]);
            for (int i = 0; i < 8; ++i)
                down.push_back(apply(s, Context::Menu, keys({}), st).button[kPs2Down]);
            t.IsTrue(down == std::vector<int>({1, 1, 0, 0, 1, 1, 0, 0, 0}), "two notches: two presses");
        });

        tc.Run("menu: keys are held as before, and the right click goes back", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            PadOut o = apply(s, Context::Menu, keys({264, 257}), st);
            t.IsTrue(o.button[kPs2Down] && o.button[kPs2Cross], "arrow down and Enter");
            o = apply(s, Context::Menu, keys({}, kRightClick), st);
            t.IsTrue(o.button[kPs2Triangle], "right click: Triangle (back)");
            t.IsFalse(o.aimHold, "no aim-hold in a menu");
        });

        tc.Run("a button held across a context change is not a fresh press", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            apply(s, Context::Game, keys({}, kLeftClick), st);   // firing
            const PadOut o = apply(s, Context::Menu, keys({}, kLeftClick), st);   // the pause menu opens
            t.IsFalse(o.button[kPs2Cross], "no select from the held fire button");
        });

        tc.Run("an input bound only in the menu, held across the change, is not a fresh press either", [](TestCase &t)
        {
            const Settings s = parse("[menu]\nsquare = mouse_4\n").settings;
            State st = freshState(s);
            apply(s, Context::Game, keys({}, 1u << 3), st);   // mouse_4 held in the game, where it is unbound
            const PadOut o = apply(s, Context::Menu, keys({}, 1u << 3), st);
            t.IsFalse(o.button[kPs2Square], "no Square from the held button");
        });

        tc.Run("a pulse queued in a menu does not land in the game", [](TestCase &t)
        {
            const Settings s = defaults();
            State st = freshState(s);
            apply(s, Context::Menu, keys({}, 0, 0, 3), st);   // three notches queued
            const PadOut o = apply(s, Context::Game, keys({}), st);
            t.IsFalse(o.button[kPs2Down], "cleared on the context change");
        });
    });
}
