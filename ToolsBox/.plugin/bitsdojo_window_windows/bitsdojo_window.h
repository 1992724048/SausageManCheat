#ifndef BITSDOJO_WINDOW_H_
#define BITSDOJO_WINDOW_H_
#include <windows.h>

namespace bitsdojo_window {
    using TIsBitsdojoWindowLoaded = bool(*)();
    auto isBitsdojoWindowLoaded() -> bool;

    using TSetWindowCanBeShown = void(*)(bool);
    auto setWindowCanBeShown(bool value) -> void;

    using TDragAppWindow = bool(*)();
    auto dragAppWindow() -> bool;

    using TGetAppWindow = HWND(*)();
    auto getAppWindow() -> HWND;

    using TSetMinSize = void(*)(int, int);
    auto setMinSize(int width, int height) -> void;

    using TSetMaxSize = void(*)(int, int);
    auto setMaxSize(int width, int height) -> void;

    using TSetWindowCutOnMaximize = void(*)(int);
    auto setWindowCutOnMaximize(int value) -> void;

    using TIsDPIAware = bool(*)();
    auto isDPIAware() -> bool;
}
#endif
