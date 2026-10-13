#pragma once
// Keyboard-and-mouse play -- the macOS fork's glue (spec docs/superpowers/specs/2026-10-12-kbm-play-design.md).
// The pure core is runtime/socom2_kbm_core.h. Threads: socom2KbmApply on the game thread (the pad poll),
// socom2KbmFrame and socom2KbmOverlay on the main thread. Shared state is atomics only.
#include "runtime/socom2_kbm_core.h"
#include "socom2_host_input.h"

#include <cstdint>
#include <string>

namespace ps2_stubs
{
    // PS2X_KBM (fork default on), off for a scripted or replayed run unless the knob says 1. Decided once.
    bool socom2KbmEnabled();
    // controls.ini (PS2X_KBM_FILE, else beside the executable), read once; written with the defaults when absent.
    const socom2_kbm::Settings &socom2KbmSettings();
    // One pad read: the bindings of the context the game is in, into `next` (buttons OR-ed, axes written when
    // moved). `aimHold`: the aim-hold binding is down (the mouse module's right-click behaviour). `triangleLight`:
    // Triangle is pressed by crouch alone, at light pressure.
    void socom2KbmApply(uint8_t *rdram, Socom2PadState &next, bool &aimHold, bool &triangleLight);
    // Main thread, once a frame: the mouse buttons (`liveMask` bit i: the mouse module's capture lets button i
    // through in a mission), their press edges, and the wheel.
    void socom2KbmFrame(uint8_t liveMask, bool focused);
    // Main thread, inside the frame's drawing: the speed indicator.
    void socom2KbmOverlay();
}
