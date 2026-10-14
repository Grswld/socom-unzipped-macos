// Keyboard-and-mouse play -- the macOS fork's glue. See socom2_kbm.h.
#include "socom2_kbm.h"
#include "socom2_mouse.h"

#include "ps2x/exe_dir.h"
#include "ps2x/knobs.h"
#include "runtime/socom2_addresses.h"
#include "runtime/socom2_mouse_core.h"
#include "raylib.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace ps2_stubs
{
    namespace
    {
        // Main thread -> game thread.
        std::atomic<uint8_t> g_buttonsGame{0};   // the five mouse buttons, gated by the mouse module's capture
        std::atomic<uint8_t> g_buttonsMenu{0};   // the same, ungated but for focus (no capture in a menu)
        std::atomic<int> g_wheelUp{0};
        std::atomic<int> g_wheelDown{0};
        // Press edges counted between pad reads, per button, per context (a tap can fall between two reads).
        std::atomic<int> g_pressGame[5] = {};
        std::atomic<int> g_pressMenu[5] = {};
        bool g_focused = false;   // main thread only: the scroll monitor runs there
        bool traceOn()
        {
            static const bool on = ps2x::knob("PS2X_KBM_TRACE") != nullptr;
            return on;
        }
        // Game thread -> main thread.
        std::atomic<int> g_step{0};
        std::atomic<int64_t> g_changedAtMs{-1000000};   // steady clock, ms

        // PS2X_KBM_SCRIPT (a self-run's hands): the events, and the time since the first pad read.
        std::vector<socom2_kbm::ScriptEvent> g_script;
        double g_scriptT = 0.0;

        int64_t nowMs()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        std::filesystem::path controlsPath()
        {
            if (const char *p = ps2x::knob("PS2X_KBM_FILE"); p != nullptr && *p != '\0')
                return p;
            return ExeDir::get() / "controls.ini";
        }
    }

    bool socom2KbmEnabled()
    {
        static const bool on = [] {
            const bool scripted = ps2x::knob("PS2X_SOCOM2_INPUT_SCRIPT") != nullptr || ps2x::knob("PS2X_SOCOM2_INPUT_FILE") != nullptr;
            return ps2x::knobOn("PS2X_KBM", !scripted);   // == socom2_kbm::enabledFor, the tested rule
        }();
        return on;
    }

    const socom2_kbm::Settings &socom2KbmSettings()
    {
        static const socom2_kbm::Settings settings = [] {
            const std::filesystem::path path = controlsPath();
            std::error_code ec;
            if (!std::filesystem::exists(path, ec))
            {
                const socom2_kbm::Settings d = socom2_kbm::defaults();
                std::ofstream out(path, std::ios::binary);
                if (out && (out << socom2_kbm::write(d)))
                    std::cout << "[kbm] wrote the default controls to " << path.string() << std::endl;
                else
                    std::cout << "[kbm] no controls file and none could be written at " << path.string() << "; playing the defaults" << std::endl;
                return d;
            }
            std::ifstream in(path, std::ios::binary);
            std::stringstream text;
            text << in.rdbuf();
            const socom2_kbm::ParseResult r = socom2_kbm::parse(text.str());
            for (const std::string &e : r.errors)
                std::cout << "[kbm] " << path.filename().string() << " " << e << " (ignored)" << std::endl;
            std::cout << "[kbm] controls from " << path.string() << ": " << r.settings.game.size() << " game and "
                      << r.settings.menu.size() << " menu bindings, " << r.settings.steps.size() << " speed steps" << std::endl;
            return r.settings;
        }();
        return settings;
    }

    void socom2KbmApply(uint8_t *rdram, Socom2PadState &next, bool &aimHold, bool &triangleLight)
    {
        using namespace socom2_kbm;
        const Settings &s = socom2KbmSettings();
        static State st = [&] {
            State fresh;
            fresh.speed.reset(s);
            g_step.store(fresh.speed.step(), std::memory_order_relaxed);
            return fresh;
        }();

        // The game context: an actor whose controller validates with m_menustate 0 (the mouse module's predicate).
        socom2_mouse::ModeRead mode;
        if (rdram != nullptr)
            mode = socom2_mouse::readMode(rdram, socom2_mouse::revisionFor(socom2_addresses::current().revision));
        const bool game = mode.ok && socom2_mouse::interactive(rdram, mode.actor);
        const Context ctx = game ? Context::Game : Context::Menu;

        static const bool s_scripted = [] {
            if (const char *v = ps2x::knob("PS2X_KBM_SCRIPT"))
            {
                std::vector<std::string> errors;
                g_script = socom2_kbm::parseScript(v, &errors);
                for (const std::string &e : errors)
                    std::cout << "[kbm] PS2X_KBM_SCRIPT: " << e << std::endl;
                std::cout << "[kbm] PS2X_KBM_SCRIPT: " << g_script.size() << " events" << std::endl;
            }
            return !g_script.empty();
        }();
        static const int64_t s_startMs = nowMs();
        const double lastT = g_scriptT;
        g_scriptT = double(nowMs() - s_startMs) / 1000.0;

        Inputs in;
        in.isKeyDown = [](int key) { return IsKeyDown(key) || (!g_script.empty() && socom2_kbm::scriptHolds(g_script, key, g_scriptT)); };
        in.mouseButtons = (game ? g_buttonsGame : g_buttonsMenu).load(std::memory_order_relaxed);
        in.wheelUp = g_wheelUp.exchange(0, std::memory_order_relaxed);
        in.wheelDown = g_wheelDown.exchange(0, std::memory_order_relaxed);
        if (s_scripted)
        {
            for (int b = 0; b < 5; ++b)
                if (socom2_kbm::scriptHolds(g_script, kMouseLeft - b, g_scriptT))
                    in.mouseButtons |= static_cast<uint8_t>(1u << b);
            in.wheelUp += socom2_kbm::scriptWheel(g_script, kWheelUp, lastT, g_scriptT);
            in.wheelDown += socom2_kbm::scriptWheel(g_script, kWheelDown, lastT, g_scriptT);
        }
        // The mouse buttons, stretched: every counted press reaches the game as an edge (play-test 2).
        {
            static ClickStretch s_stretch[5];
            const uint8_t level = in.mouseButtons;
            uint8_t stretched = 0;
            for (int b = 0; b < 5; ++b)
            {
                const int game_ = g_pressGame[b].exchange(0, std::memory_order_relaxed);
                const int menu_ = g_pressMenu[b].exchange(0, std::memory_order_relaxed);
                const int presses = game ? game_ : menu_;
                const bool down = s_stretch[b].tick(((level >> b) & 1u) != 0, presses);
                if (down)
                    stretched |= static_cast<uint8_t>(1u << b);
                if (traceOn() && b == 0)
                {
                    static bool s_was = false;
                    if (presses > 0 || down != s_was)
                        std::cout << "[kbm] click left presses=" << presses << " level=" << ((level & 1u) != 0)
                                  << " sent=" << down << " ctx=" << (game ? "game" : "menu") << std::endl;
                    s_was = down;
                }
            }
            in.mouseButtons = stretched;
        }
        // The wished stance (r0001 actor+0x374, research/79): a wheel step stands a crouched SEAL up.
        static const bool s_r0001 = std::strcmp(socom2_addresses::current().revision, "r0001") == 0;
        in.stance = game && s_r0001 ? int(rdram[mode.actor + 0x374u]) : -1;
        const PadOut out = apply(s, ctx, in, st);

        for (int i = 0; i < 16; ++i)
            if (out.button[i])
                next.button[i] = 1u;
        for (int i = 0; i < 4; ++i)
            if (out.axis[i] != 0x80u)
                next.axis[i] = out.axis[i];
        aimHold = out.aimHold;
        triangleLight = out.button[kPadTriangle] && out.trianglePressure != kFullPressure;
        if (out.stepChanged)
        {
            g_step.store(st.speed.step(), std::memory_order_relaxed);
            g_changedAtMs.store(nowMs(), std::memory_order_relaxed);
        }

        // PS2X_KBM_TRACE: the step, the push and the axes written, and the actor's ground speed over the last
        // second (position words +0x1c / +0x24), on a step change and every second while a move key is down.
        static const bool s_trace = ps2x::knob("PS2X_KBM_TRACE") != nullptr;
        if (s_trace)
        {
            static int64_t s_windowMs = nowMs();
            static float s_x = 0.0f, s_z = 0.0f;
            static uint32_t s_actor = 0;
            static bool s_allGame = false;   // every read of this window was in the game context
            s_allGame = s_allGame && game;
            const int64_t t = nowMs();
            float x = 0.0f, z = 0.0f;
            if (game)
            {
                std::memcpy(&x, rdram + mode.actor + 0x1cu, sizeof(x));
                std::memcpy(&z, rdram + mode.actor + 0x24u, sizeof(z));
            }
            const bool windowDone = t - s_windowMs >= 1000;
            if (out.stepChanged || (windowDone && out.moving))
            {
                std::ostringstream line;
                line << std::fixed << std::setprecision(2) << "[kbm] ctx=" << (game ? "game" : "menu") << " step="
                     << st.speed.step() + 1 << "/" << st.speed.count() << " push=" << std::lround(out.push * 100.0f) << "% lx=" << int(out.axis[2])
                     << " ly=" << int(out.axis[3]);
                if (game)
                    line << " stance=" << int(rdram[mode.actor + 0x374u]);   // the wished stance byte (research/79)
                if (game && s_allGame && s_actor == mode.actor && windowDone)
                    line << " speed=" << std::hypot(x - s_x, z - s_z) * 1000.0 / double(t - s_windowMs) << "/s";
                std::cout << line.str() << std::endl;
            }
            if (windowDone || s_actor != mode.actor)
            {
                s_windowMs = t;
                s_x = x;
                s_z = z;
                s_actor = mode.actor;
                s_allGame = game;
            }
        }
    }

    void socom2KbmFrame(uint8_t liveMask, bool focused)
    {
        g_focused = focused;
        static const int kButtons[5] = {MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT, MOUSE_BUTTON_MIDDLE, MOUSE_BUTTON_SIDE,
                                        MOUSE_BUTTON_EXTRA};
        uint8_t down = 0;
        for (int i = 0; i < 5; ++i)
            if (IsMouseButtonDown(kButtons[i]))
                down |= static_cast<uint8_t>(1u << i);
        g_buttonsGame.store(down & liveMask, std::memory_order_relaxed);
        g_buttonsMenu.store(focused ? down : 0, std::memory_order_relaxed);
        for (int i = 0; i < 5; ++i)
            if (IsMouseButtonPressed(kButtons[i]))
            {
                if ((liveMask >> i) & 1u)
                    g_pressGame[i].fetch_add(1, std::memory_order_relaxed);
                if (focused)
                    g_pressMenu[i].fetch_add(1, std::memory_order_relaxed);
            }

        // The wheel: every scroll event (the monitor), else raylib's last event of the frame.
        static socom2_kbm::WheelCounter s_counter;
        static const bool s_monitor = socom2ScrollMonitorStart([](double dy, bool precise, bool momentum, double t) {
            if (!g_focused)
                return;
            const int n = s_counter.add({dy, precise, momentum, t});
            if (n > 0)
                g_wheelUp.fetch_add(n, std::memory_order_relaxed);
            else if (n < 0)
                g_wheelDown.fetch_add(-n, std::memory_order_relaxed);
            if (traceOn())
                std::cout << "[kbm] scroll dy=" << dy << " precise=" << precise << " momentum=" << momentum << " steps=" << n
                          << std::endl;
        });
        if (!s_monitor && focused)
        {
            const float v = GetMouseWheelMove();
            const int n = s_counter.add({v, false, false, GetTime()});
            if (n > 0)
                g_wheelUp.fetch_add(n, std::memory_order_relaxed);
            else if (n < 0)
                g_wheelDown.fetch_add(-n, std::memory_order_relaxed);
        }
    }

    void socom2KbmOverlay()
    {
        if (!socom2KbmEnabled())
            return;
        const socom2_kbm::Settings &s = socom2KbmSettings();
        const double since = double(nowMs() - g_changedAtMs.load(std::memory_order_relaxed)) / 1000.0;
        const std::string line = socom2_kbm::indicatorLine(s, g_step.load(std::memory_order_relaxed), since);
        if (line.empty())
            return;
        const int size = 20;
        const int w = MeasureText(line.c_str(), size);
        const int x = (GetScreenWidth() - w) / 2;
        const int y = GetScreenHeight() - size - 40;
        DrawRectangle(x - 8, y - 4, w + 16, size + 8, Color{0, 0, 0, 170});
        DrawText(line.c_str(), x, y, size, Color{230, 200, 90, 255});
    }
}
