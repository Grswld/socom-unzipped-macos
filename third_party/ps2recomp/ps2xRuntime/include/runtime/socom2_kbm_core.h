#pragma once
// Keyboard-and-mouse play -- the macOS fork's layer between the host's keys, mouse buttons and wheel and the pad
// state (spec: docs/superpowers/specs/2026-10-12-kbm-play-design.md). Upstream's key table (launcher/mapping.h) is
// frozen on purpose -- the harness posts its keys and gates pin its hash -- so this layer sits beside it and,
// when on, replaces the keyboard step for interactive play. Off is the path as it was.
//
// Pure: no raylib, no OS call, no environment read. socom2_kbm.cpp is the glue.
#include "launcher/mapping.h"
#include "ps2x/knobs.h"
#include "runtime/host_crouch_shortcut.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace socom2_kbm
{
    // ---- Inputs: a keyboard key is its raylib (== GLFW) code, > 0; the mouse is negative -----------------------
    constexpr int kMouseLeft = -1;
    constexpr int kMouseRight = -2;
    constexpr int kMouseMiddle = -3;
    constexpr int kMouse4 = -4;
    constexpr int kMouse5 = -5;
    constexpr int kWheelUp = -10;
    constexpr int kWheelDown = -11;
    inline bool isMouseButton(int input) { return input <= kMouseLeft && input >= kMouse5; }
    inline bool isWheel(int input) { return input == kWheelUp || input == kWheelDown; }

    // ---- Actions: 0-15 are the PS2 buttons (libpad2 ids); the rest are the layer's own --------------------------
    enum Action : uint8_t
    {
        kMoveForward = 16, kMoveBack, kMoveLeft, kMoveRight,
        kLookUp, kLookDown, kLookLeft, kLookRight,
        kCrouch,     // Triangle at light pressure: stand <-> crouch (host_crouch_shortcut.h)
        kProne,      // Triangle at full pressure: prone, or stand from prone
        kAimHold,    // the mouse module's right-click aim-hold (socom2_mouse_core.h AimHold)
        kSpeedUp,    // one wheel-speed step, per press or notch
        kSpeedDown,
        kRun,        // full push while held, whatever the step
        kActionCount
    };

    enum class Context : uint8_t { Game, Menu };
    enum class Indicator : uint8_t { Change, Always, Off };

    struct Binding
    {
        int input;
        uint8_t action;
        bool operator==(const Binding &) const = default;
    };

    // The lowest push a speed step may have: the measured dead zone (~0.33, spec "Measured") plus a margin.
    constexpr float kMinPush = 0.36f;
    constexpr int kMaxSteps = 10;
    constexpr uint8_t kFullPressure = kFullButtonPressure;
    constexpr uint8_t kLightPressure = kCrouchShortcutPressure;

    struct Settings
    {
        std::vector<Binding> game;
        std::vector<Binding> menu;
        std::vector<float> steps;   // left-stick push per wheel step, ascending, each in [kMinPush, 1]
        int start = 0;              // the step a session begins at (0-based)
        Indicator indicator = Indicator::Change;
        // [mouse]: set only when the file says so; the PS2X_MOUSE_* knobs win over these.
        bool hasSens = false, hasAimSens = false, hasInvertY = false;
        float sens = 1.0f, aimSens = 0.6f;
        bool invertY = false;
        bool operator==(const Settings &) const = default;
    };

    // ---- Names --------------------------------------------------------------------------------------------------
    namespace detail
    {
        struct NamedInput { const char *name; int input; };
        // Keys launcher::mapping does not name, and the mouse. Codes are GLFW's.
        inline constexpr NamedInput kNamedInputs[] = {
            {"mouse_left", kMouseLeft}, {"mouse_right", kMouseRight}, {"mouse_middle", kMouseMiddle},
            {"mouse_4", kMouse4}, {"mouse_5", kMouse5}, {"wheel_up", kWheelUp}, {"wheel_down", kWheelDown},
            {"left_shift", 340}, {"left_ctrl", 341}, {"left_alt", 342}, {"left_cmd", 343},
            {"right_shift", 344}, {"right_ctrl", 345}, {"right_alt", 346}, {"right_cmd", 347},
            {"caps_lock", 280}, {"delete", 261}, {"insert", 260}, {"home", 268}, {"end", 269},
            {"page_up", 266}, {"page_down", 267}, {"grave", 96}, {"minus", 45}, {"equal", 61},
            {"left_bracket", 91}, {"right_bracket", 93}, {"backslash", 92}, {"semicolon", 59},
            {"apostrophe", 39}, {"comma", 44}, {"period", 46}, {"slash", 47},
            {"f1", 290}, {"f2", 291}, {"f3", 292}, {"f4", 293}, {"f5", 294}, {"f6", 295},
            {"f7", 296}, {"f8", 297}, {"f9", 298}, {"f10", 299}, {"f11", 300}, {"f12", 301},
            {"kp_0", 320}, {"kp_1", 321}, {"kp_2", 322}, {"kp_3", 323}, {"kp_4", 324}, {"kp_5", 325},
            {"kp_6", 326}, {"kp_7", 327}, {"kp_8", 328}, {"kp_9", 329}, {"kp_enter", 335},
        };
        inline constexpr const char *kActionNames[kActionCount - 16] = {
            "move_forward", "move_back", "move_left", "move_right",
            "look_up", "look_down", "look_left", "look_right",
            "crouch", "prone", "aim_hold", "speed_up", "speed_down", "run",
        };
        inline std::string trim(const std::string &s)
        {
            const size_t a = s.find_first_not_of(" \t\r");
            if (a == std::string::npos)
                return "";
            const size_t b = s.find_last_not_of(" \t\r");
            return s.substr(a, b - a + 1);
        }
        inline std::string lower(std::string s)
        {
            for (char &c : s)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return s;
        }
    }

    inline std::string inputName(int input)
    {
        for (const auto &n : detail::kNamedInputs)
            if (n.input == input)
                return n.name;
        return launcher::mapping::keyName(input);
    }

    // -1 for a name it does not know (no key or mouse code is -1 apart from kMouseLeft, which has a name).
    inline int inputFromName(const std::string &rawName)
    {
        const std::string name = detail::lower(detail::trim(rawName));
        if (name.empty())
            return -100;
        for (const auto &n : detail::kNamedInputs)
            if (name == n.name)
                return n.input;
        const int key = launcher::mapping::keyFromName(name);
        return key > 0 ? key : -100;
    }
    constexpr int kUnknownInput = -100;

    inline const char *actionName(uint8_t action)
    {
        if (action < 16)
            return launcher::mapping::ps2ButtonName(action);
        if (action < kActionCount)
            return detail::kActionNames[action - 16];
        return "";
    }

    inline int actionFromName(const std::string &rawName)
    {
        const std::string name = detail::lower(detail::trim(rawName));
        const int button = launcher::mapping::ps2ButtonFromName(name);
        if (button >= 0)
            return button;
        for (int i = 16; i < kActionCount; ++i)
            if (name == detail::kActionNames[i - 16])
                return i;
        return -1;
    }

    // ---- The defaults (spec "Defaults"): preset 1, "Precision Shooter" -----------------------------------------
    inline Settings defaults()
    {
        using namespace launcher::mapping;
        Settings s;
        s.game = {
            {'W', kMoveForward}, {'S', kMoveBack}, {'A', kMoveLeft}, {'D', kMoveRight},
            {'I', kLookUp}, {'K', kLookDown}, {'J', kLookLeft}, {'L', kLookRight},
            {kMouseLeft, kPs2R1},                       // fire weapon / throw grenade
            {kMouseRight, kAimHold},                    // zoom while held
            {kWheelUp, kSpeedUp}, {kWheelDown, kSpeedDown},
            {340, kRun},                                // left shift: full speed while held
            {' ', kPs2Square},                          // jump
            {'C', kCrouch}, {341, kProne},              // C crouch, left ctrl prone
            {'F', kPs2Cross}, {257, kPs2Cross},         // F / Enter: execute command, pick up weapon, "press X"
            {'T', kPs2Circle},                          // command teammates
            {'R', kPs2R3},                              // force reload
            {'B', kPs2L3},                              // change fire rate
            {258, kPs2R2},                              // Tab: open / close weapon selection
            {'1', kPs2L1}, {'2', kPs2L2},               // quick-select primary / secondary weapon
            // The d-pad, an action's inputs together (the written file groups them; parse keeps the place):
            {kMouseMiddle, kPs2Up}, {'Z', kPs2Up}, {265, kPs2Up},   // zoom sniper scope / NVG (and the up arrow)
            {'X', kPs2Down}, {264, kPs2Down},                       // zoom out
            {'Q', kPs2Left}, {263, kPs2Left},                       // peek left
            {'E', kPs2Right}, {262, kPs2Right},                     // peek right
            {'M', kPs2Select},                          // TacMap / objectives
            {256, kPs2Start},                           // Escape: pause
        };
        s.menu = {
            {265, kPs2Up}, {'W', kPs2Up}, {kWheelUp, kPs2Up},
            {264, kPs2Down}, {'S', kPs2Down}, {kWheelDown, kPs2Down},
            {263, kPs2Left}, {'A', kPs2Left},
            {262, kPs2Right}, {'D', kPs2Right},
            {257, kPs2Cross}, {' ', kPs2Cross}, {'F', kPs2Cross}, {kMouseLeft, kPs2Cross},   // select
            {259, kPs2Triangle}, {kMouseRight, kPs2Triangle},                               // back
            {256, kPs2Start},
            {'Q', kPs2L1}, {'E', kPs2R1},               // scroll text
            {'Z', kPs2Square}, {'C', kPs2Circle},
            {'M', kPs2Select},
        };
        // Standing speeds measured on r0001 (2026-10-12, PS2X_KBM_TRACE): push 0.36 ~7, 0.45 ~15, 0.50 ~19, 0.55 ~32,
        // 0.60 ~39, 0.65 ~45, 0.70 ~53, 0.78 and up ~64 units/s. These steps give ~13 / 26 / 39 / 52 / 64: even.
        // Crouched, every push under 0.838 is the one crouch walk; the top step stands the SEAL up and runs.
        s.steps = {0.43f, 0.53f, 0.60f, 0.69f, 1.00f};
        s.start = static_cast<int>(s.steps.size()) - 1;
        return s;
    }

    // ---- The file: controls.ini -------------------------------------------------------------------------------
    // [game] / [menu]: `<action> = <input>, <input>, ...` -- a line replaces that action's inputs in its section
    // (an empty value unbinds it); actions the file does not name keep their defaults. [speed]: `steps = 0.4, ...`,
    // `start = <1-based step>`, `indicator = change | always | off`. [mouse]: `sensitivity`, `aim_sensitivity`,
    // `invert_y = 0 | 1`. `#` and `;` start comments. A line that cannot be read is reported and ignored.
    struct ParseResult
    {
        Settings settings;
        std::vector<std::string> errors;   // "line <n>: <what>"
    };

    namespace detail
    {
        inline void replaceAction(std::vector<Binding> &table, uint8_t action, const std::vector<int> &inputs)
        {
            // Keep the action's place in the table (its first binding's index), so a written file round-trips.
            size_t at = table.size();
            for (size_t i = 0; i < table.size(); ++i)
                if (table[i].action == action)
                {
                    at = i;
                    break;
                }
            std::vector<Binding> out;
            for (size_t i = 0; i < table.size(); ++i)
            {
                if (i == at)
                    for (int in : inputs)
                        out.push_back({in, action});
                if (table[i].action != action)
                    out.push_back(table[i]);
            }
            if (at == table.size())
                for (int in : inputs)
                    out.push_back({in, action});
            table = std::move(out);
        }

        inline bool parseFloat(const std::string &v, float &out)
        {
            char *end = nullptr;
            const std::string t = trim(v);
            const float f = std::strtof(t.c_str(), &end);
            if (t.empty() || end == t.c_str() || *end != '\0' || !std::isfinite(f))
                return false;
            out = f;
            return true;
        }
    }

    inline ParseResult parse(const std::string &text)
    {
        ParseResult r;
        r.settings = defaults();
        Settings &s = r.settings;
        std::string section;
        std::istringstream in(text);
        std::string raw;
        int lineNo = 0;
        bool startGiven = false;
        int startOneBased = 0;
        auto error = [&](const std::string &what) { r.errors.push_back("line " + std::to_string(lineNo) + ": " + what); };
        while (std::getline(in, raw))
        {
            ++lineNo;
            std::string line = raw;
            const size_t hash = line.find_first_of("#;");
            if (hash != std::string::npos)
                line = line.substr(0, hash);
            line = detail::trim(line);
            if (line.empty())
                continue;
            if (line.front() == '[')
            {
                if (line.back() != ']')
                {
                    error("a section header without its ']'");
                    section.clear();
                    continue;
                }
                section = detail::lower(detail::trim(line.substr(1, line.size() - 2)));
                if (section != "game" && section != "menu" && section != "speed" && section != "mouse")
                {
                    error("unknown section [" + section + "]");
                    section = "?";
                }
                continue;
            }
            const size_t eq = line.find('=');
            if (eq == std::string::npos)
            {
                error("no '='");
                continue;
            }
            const std::string key = detail::lower(detail::trim(line.substr(0, eq)));
            const std::string value = detail::trim(line.substr(eq + 1));
            if (section == "game" || section == "menu")
            {
                const int action = actionFromName(key);
                if (action < 0)
                {
                    error("unknown action '" + key + "'");
                    continue;
                }
                std::vector<int> inputs;
                bool ok = true;
                std::stringstream list(value);
                std::string item;
                while (std::getline(list, item, ','))
                {
                    if (detail::trim(item).empty())
                        continue;
                    const int input = inputFromName(item);
                    if (input == kUnknownInput)
                    {
                        error("unknown input '" + detail::trim(item) + "'");
                        ok = false;
                        break;
                    }
                    inputs.push_back(input);
                }
                if (ok)
                    detail::replaceAction(section == "game" ? s.game : s.menu, static_cast<uint8_t>(action), inputs);
            }
            else if (section == "speed")
            {
                if (key == "steps")
                {
                    std::vector<float> steps;
                    bool ok = true;
                    std::stringstream list(value);
                    std::string item;
                    while (std::getline(list, item, ','))
                    {
                        float f = 0.0f;
                        if (!detail::parseFloat(item, f) || f <= 0.0f || f > 1.0f)
                        {
                            error("a step must be a number in (0, 1]: '" + detail::trim(item) + "'");
                            ok = false;
                            break;
                        }
                        if (f < kMinPush)
                        {
                            error("step " + detail::trim(item) + " is inside the stick's dead zone; raised to 0.36");
                            f = kMinPush;
                        }
                        steps.push_back(f);
                    }
                    if (ok && (steps.empty() || static_cast<int>(steps.size()) > kMaxSteps))
                    {
                        error("steps needs 1 to 10 values");
                        ok = false;
                    }
                    if (ok)
                    {
                        std::sort(steps.begin(), steps.end());
                        s.steps = steps;
                        if (!startGiven)
                            s.start = static_cast<int>(s.steps.size()) - 1;
                    }
                }
                else if (key == "start")
                {
                    char *end = nullptr;
                    const long n = std::strtol(value.c_str(), &end, 10);
                    if (value.empty() || *end != '\0' || n < 1)
                        error("start must be a step number from 1");
                    else
                    {
                        startGiven = true;
                        startOneBased = static_cast<int>(n);
                    }
                }
                else if (key == "indicator")
                {
                    const std::string v = detail::lower(value);
                    if (v == "change")
                        s.indicator = Indicator::Change;
                    else if (v == "always")
                        s.indicator = Indicator::Always;
                    else if (v == "off")
                        s.indicator = Indicator::Off;
                    else
                        error("indicator must be change, always or off");
                }
                else
                    error("unknown [speed] setting '" + key + "'");
            }
            else if (section == "mouse")
            {
                float f = 0.0f;
                if (key == "sensitivity" || key == "aim_sensitivity")
                {
                    if (!detail::parseFloat(value, f) || f <= 0.0f || f >= 100.0f)
                        error(key + " must be a number above 0");
                    else if (key == "sensitivity")
                    {
                        s.hasSens = true;
                        s.sens = f;
                    }
                    else
                    {
                        s.hasAimSens = true;
                        s.aimSens = f;
                    }
                }
                else if (key == "invert_y")
                {
                    if (value == "0" || value == "1")
                    {
                        s.hasInvertY = true;
                        s.invertY = value == "1";
                    }
                    else
                        error("invert_y must be 0 or 1");
                }
                else
                    error("unknown [mouse] setting '" + key + "'");
            }
            else if (section.empty())
                error("a setting before any [section]");
            // section "?": the unknown section's lines were reported with its header
        }
        if (startGiven)
            s.start = std::clamp(startOneBased - 1, 0, static_cast<int>(s.steps.size()) - 1);
        return r;
    }

    namespace detail
    {
        inline std::string bindingLines(const std::vector<Binding> &table)
        {
            std::vector<uint8_t> order;
            for (const Binding &b : table)
                if (std::find(order.begin(), order.end(), b.action) == order.end())
                    order.push_back(b.action);
            std::string out;
            for (uint8_t a : order)
            {
                out += actionName(a);
                out += " = ";
                bool first = true;
                for (const Binding &b : table)
                    if (b.action == a)
                    {
                        out += (first ? "" : ", ") + inputName(b.input);
                        first = false;
                    }
                out += "\n";
            }
            return out;
        }
        inline std::string floatText(float f)
        {
            std::ostringstream o;
            o << f;
            return o.str();
        }
    }

    // The file the game writes when there is none: every binding, the speed steps and the help, so `parse` of it
    // gives `s` back.
    inline std::string write(const Settings &s)
    {
        std::string out =
            "# SOCOM II keyboard and mouse controls (macOS fork). Edit and restart the game.\n"
            "# docs/CONTROLS_MACOS.md lists every action. A line is `action = input, input`; an empty value unbinds.\n"
            "# Inputs: letters and digits as themselves (a, 1), space enter escape backspace tab up down left right,\n"
            "#   left_shift left_ctrl left_alt left_cmd (and right_*), f1-f12, kp_0-kp_9, mouse_left mouse_right\n"
            "#   mouse_middle mouse_4 mouse_5 wheel_up wheel_down, or key:<GLFW code>.\n"
            "# PS2 buttons as actions: up down left right cross circle square triangle l1 l2 l3 r1 r2 r3 start select.\n"
            "# Layer actions: move_forward move_back move_left move_right look_up look_down look_left look_right,\n"
            "#   crouch (light Triangle), prone (firm Triangle), aim_hold (zoom while held), speed_up speed_down, run.\n"
            "\n# In a mission (preset 1, Precision Shooter).\n[game]\n";
        out += detail::bindingLines(s.game);
        out += "\n# Front end, pause and every other menu. The wheel and the mouse buttons step one item per notch or click.\n[menu]\n";
        out += detail::bindingLines(s.menu);
        out += "\n# Walk speed on the wheel: the left stick's push per step (the game ignores pushes under about 0.33).\n[speed]\nsteps = ";
        for (size_t i = 0; i < s.steps.size(); ++i)
            out += (i ? ", " : "") + detail::floatText(s.steps[i]);
        out += "\nstart = " + std::to_string(s.start + 1) + "\nindicator = ";
        out += s.indicator == Indicator::Always ? "always" : s.indicator == Indicator::Off ? "off" : "change";
        out += "\n\n# Mouse look. Uncomment to set; PS2X_MOUSE_SENS, PS2X_MOUSE_AIM_SENS and PS2X_MOUSE_INVERT_Y win when set.\n[mouse]\n";
        out += (s.hasSens ? "" : "# ") + std::string("sensitivity = ") + detail::floatText(s.sens) + "\n";
        out += (s.hasAimSens ? "" : "# ") + std::string("aim_sensitivity = ") + detail::floatText(s.aimSens) + "\n";
        out += (s.hasInvertY ? "" : "# ") + std::string("invert_y = ") + (s.invertY ? "1" : "0") + "\n";
        return out;
    }

    // ---- On or off ----------------------------------------------------------------------------------------------
    // PS2X_KBM's value (nullptr: unset) under the one flag rule (ps2x::knobs::flagValue). A scripted or replayed
    // run gets the layer only when the knob turns it on, the same rule as the mouse's (a hand on the desk must not
    // move a parity run).
    inline bool enabledFor(const char *knobValue, bool scripted)
    {
        return ps2x::knobs::flagValue(knobValue, !scripted);
    }

    // ---- Wheel speed --------------------------------------------------------------------------------------------
    class SpeedSelector
    {
    public:
        void reset(const Settings &s)
        {
            m_count = static_cast<int>(s.steps.size());
            m_step = std::clamp(s.start, 0, std::max(0, m_count - 1));
        }
        // +n steps up, -n down; clamped. True when the step changed.
        bool move(int delta)
        {
            const int next = std::clamp(m_step + delta, 0, std::max(0, m_count - 1));
            const bool changed = next != m_step;
            m_step = next;
            return changed;
        }
        int step() const { return m_step; }
        int count() const { return m_count; }

    private:
        int m_step = 0;
        int m_count = 0;
    };

    // The axis byte for a push in [-1, 1] on one stick axis: 0x00 at -1, 0x80 at 0, 0xFF at +1 (the keyboard's
    // full deflection, as before).
    inline uint8_t axisByte(float v)
    {
        v = std::clamp(v, -1.0f, 1.0f);
        const long b = v < 0.0f ? 128 - std::lround(-v * 128.0f) : 128 + std::lround(v * 127.0f);
        return static_cast<uint8_t>(std::clamp(b, 0L, 255L));
    }

    // ---- One pad read -------------------------------------------------------------------------------------------
    struct Inputs
    {
        bool (*isKeyDown)(int key) = nullptr;   // raylib's IsKeyDown in the game, the suite's predicate in tests
        uint8_t mouseButtons = 0;               // bit i: mouse button i (kMouseLeft = bit 0 ... kMouse5 = bit 4)
        int wheelUp = 0;                        // notches since the last read
        int wheelDown = 0;
    };

    struct PadOut
    {
        uint8_t axis[4] = {0x80u, 0x80u, 0x80u, 0x80u};   // RX, RY, LX, LY
        uint8_t button[16] = {0};
        uint8_t trianglePressure = kFullPressure;
        bool aimHold = false;
        bool moving = false;          // a move key is down
        bool stepChanged = false;
        float push = 1.0f;            // the left stick's push this read (the step's, or 1 with run)
    };

    // Buttons pressed by a click or a notch: held two reads, released two (socom2_mouse_core.h's pulse timing),
    // so the game sees each as one press.
    class ButtonPulses
    {
    public:
        static constexpr int kHeld = 2;
        static constexpr int kGap = 2;
        void push(uint8_t button)
        {
            if (m_queue.size() < 16)
                m_queue.push_back(button);
        }
        int tick()   // the button to hold this read, or -1
        {
            if (m_phase == 0)
            {
                if (m_queue.empty())
                    return -1;
                m_current = m_queue.front();
                m_queue.pop_front();
                m_phase = kHeld + kGap;
            }
            const int out = m_phase > kGap ? m_current : -1;
            --m_phase;
            return out;
        }
        void clear()
        {
            m_queue.clear();
            m_phase = 0;
        }

    private:
        std::deque<uint8_t> m_queue;
        int m_current = 0;
        int m_phase = 0;
    };

    struct State
    {
        SpeedSelector speed;
        ButtonPulses pulses;
        std::map<int, bool> wasDown;   // per input: down at the last read (edges for speed keys and menu clicks)
        Context lastContext = Context::Menu;
    };

    inline bool inputDown(const Inputs &in, int input)
    {
        if (isMouseButton(input))
            return (in.mouseButtons >> (kMouseLeft - input)) & 1u;
        if (input == kWheelUp)
            return in.wheelUp > 0;
        if (input == kWheelDown)
            return in.wheelDown > 0;
        return input > 0 && in.isKeyDown != nullptr && in.isKeyDown(input);
    }

    // How many presses an input gives this read: a wheel's notches, else 1 on its down edge.
    inline int pressesOf(const Inputs &in, int input, State &st)
    {
        if (input == kWheelUp)
            return in.wheelUp;
        if (input == kWheelDown)
            return in.wheelDown;
        const bool down = inputDown(in, input);
        const bool was = st.wasDown[input];
        return down && !was ? 1 : 0;
    }

    inline PadOut apply(const Settings &s, Context ctx, const Inputs &in, State &st)
    {
        PadOut out;
        if (ctx != st.lastContext)
        {
            st.pulses.clear();   // a click's pulse must not land in the other context
            st.lastContext = ctx;
        }
        const std::vector<Binding> &table = ctx == Context::Game ? s.game : s.menu;
        bool fwd = false, back = false, left = false, right = false, run = false;
        bool lookU = false, lookD = false, lookL = false, lookR = false;
        bool crouch = false, prone = false, fullTriangle = false;
        int speedDelta = 0;
        for (const Binding &b : table)
        {
            const uint8_t a = b.action;
            // A click or a notch in a menu, and every speed step, counts presses; everything else is held.
            if (a == kSpeedUp || a == kSpeedDown)
            {
                if (ctx == Context::Game)
                    speedDelta += (a == kSpeedUp ? 1 : -1) * pressesOf(in, b.input, st);
                continue;
            }
            if (a < 16 && ctx == Context::Menu && (isMouseButton(b.input) || isWheel(b.input)))
            {
                for (int n = pressesOf(in, b.input, st); n > 0; --n)
                    st.pulses.push(a);
                continue;
            }
            if (!inputDown(in, b.input))
                continue;
            switch (a)
            {
            case kMoveForward: fwd = true; break;
            case kMoveBack: back = true; break;
            case kMoveLeft: left = true; break;
            case kMoveRight: right = true; break;
            case kLookUp: lookU = true; break;
            case kLookDown: lookD = true; break;
            case kLookLeft: lookL = true; break;
            case kLookRight: lookR = true; break;
            case kCrouch: crouch = true; break;
            case kProne: prone = true; break;
            case kAimHold: out.aimHold = true; break;
            case kRun: run = true; break;
            default:
                if (a < 16)
                {
                    out.button[a] = 1u;
                    if (a == launcher::mapping::kPs2Triangle)
                        fullTriangle = true;
                }
                break;
            }
        }
        // Edges are taken after the whole table is read, so two bindings of one input see the same edge, and for
        // both tables, so a button held across a context change (fire held as the pause menu opens) is no press.
        for (const std::vector<Binding> *t : {&s.game, &s.menu})
            for (const Binding &b : *t)
                if (!isWheel(b.input))
                    st.wasDown[b.input] = inputDown(in, b.input);

        const int pulse = st.pulses.tick();
        if (pulse >= 0)
            out.button[pulse] = 1u;

        if (speedDelta != 0)
            out.stepChanged = st.speed.move(speedDelta);

        // Stance: prone and a plain Triangle are full; crouch alone is light (R139: full wins).
        if (crouch || prone || fullTriangle)
        {
            out.button[launcher::mapping::kPs2Triangle] = 1u;
            out.trianglePressure = (prone || fullTriangle) ? kFullPressure : kLightPressure;
        }

        // Left stick: the step's push (or full with run), a diagonal's length kept at the push.
        const float x = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
        const float y = (back ? 1.0f : 0.0f) - (fwd ? 1.0f : 0.0f);
        const bool speedApplies = ctx == Context::Game && !s.steps.empty();
        out.push = (run || !speedApplies) ? 1.0f : s.steps[static_cast<size_t>(st.speed.step())];
        if (x != 0.0f || y != 0.0f)
        {
            out.moving = true;
            const float norm = (x != 0.0f && y != 0.0f) ? out.push / std::sqrt(2.0f) : out.push;
            if (x != 0.0f)
                out.axis[2] = axisByte(x * norm);
            if (y != 0.0f)
                out.axis[3] = axisByte(y * norm);
        }
        // Right stick: full deflection, as the keyboard's IJKL always gave.
        const float rx = (lookR ? 1.0f : 0.0f) - (lookL ? 1.0f : 0.0f);
        const float ry = (lookD ? 1.0f : 0.0f) - (lookU ? 1.0f : 0.0f);
        if (rx != 0.0f)
            out.axis[0] = axisByte(rx);
        if (ry != 0.0f)
            out.axis[1] = axisByte(ry);
        return out;
    }

    // ---- PS2X_KBM_SCRIPT: timed inputs for a self-run (macOS will not let a script post keys without a permission) --
    // "t:input[:hold],..." -- at t seconds after the first pad read, hold `input` (a controls.ini input name) for
    // `hold` seconds (default 0.25); a wheel input is one notch at t.
    struct ScriptEvent
    {
        double t = 0.0;
        int input = 0;
        double hold = 0.25;
    };

    inline std::vector<ScriptEvent> parseScript(const std::string &text, std::vector<std::string> *errors = nullptr)
    {
        std::vector<ScriptEvent> out;
        std::stringstream list(text);
        std::string item;
        while (std::getline(list, item, ','))
        {
            item = detail::trim(item);
            if (item.empty())
                continue;
            std::vector<std::string> f;
            std::stringstream parts(item);
            std::string part;
            while (std::getline(parts, part, ':'))
                f.push_back(part);
            ScriptEvent e;
            char *end = nullptr;
            bool ok = f.size() >= 2 && f.size() <= 3;
            if (ok)
            {
                e.t = std::strtod(f[0].c_str(), &end);
                ok = end != f[0].c_str() && *end == '\0' && e.t >= 0.0;
            }
            if (ok)
            {
                e.input = inputFromName(f[1]);
                ok = e.input != kUnknownInput;
            }
            if (ok && f.size() == 3)
            {
                e.hold = std::strtod(f[2].c_str(), &end);
                ok = end != f[2].c_str() && *end == '\0' && e.hold > 0.0;
            }
            if (ok)
                out.push_back(e);
            else if (errors != nullptr)
                errors->push_back("unreadable script event '" + item + "'");
        }
        return out;
    }

    // Is `input` held by the script at time `t`? Wheel notches are counted once by scriptWheel.
    inline bool scriptHolds(const std::vector<ScriptEvent> &events, int input, double t)
    {
        for (const ScriptEvent &e : events)
            if (e.input == input && !isWheel(input) && t >= e.t && t < e.t + e.hold)
                return true;
        return false;
    }

    // Wheel notches the script gives in (from, to].
    inline int scriptWheel(const std::vector<ScriptEvent> &events, int wheel, double from, double to)
    {
        int n = 0;
        for (const ScriptEvent &e : events)
            if (e.input == wheel && e.t > from && e.t <= to)
                ++n;
        return n;
    }

    // The indicator's text ("SPEED 3/5  70%"), or "" when it is not shown at this moment.
    inline std::string indicatorLine(const Settings &s, int step, double secondsSinceChange)
    {
        if (s.indicator == Indicator::Off || s.steps.empty())
            return "";
        if (s.indicator == Indicator::Change && secondsSinceChange > 2.5)
            return "";
        const int pct = static_cast<int>(std::lround(s.steps[static_cast<size_t>(step)] * 100.0f));
        return "SPEED " + std::to_string(step + 1) + "/" + std::to_string(s.steps.size()) + "  " + std::to_string(pct) + "%";
    }
}
