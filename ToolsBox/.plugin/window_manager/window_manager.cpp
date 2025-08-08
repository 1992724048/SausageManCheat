// This must be included before many other Windows headers.
#pragma once

#include <Windows.h>

#include <shobjidl_core.h>

#include <flutter/ephemeral/method_channel.h>
#include <flutter/ephemeral/plugin_registrar_windows.h>
#include <flutter/ephemeral/standard_method_codec.h>

#include <dwmapi.h>
#include <codecvt>
#include <map>
#include <memory>
#include <sstream>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "Gdi32.lib")

#define STATE_NORMAL 0
#define STATE_MAXIMIZED 1
#define STATE_MINIMIZED 2
#define STATE_FULLSCREEN_ENTERED 3
#define STATE_DOCKED 4

/// Window attribute that enables dark mode window decorations.
///
/// Redefined in case the developer's machine has a Windows SDK older than
/// version 10.0.22000.0.
/// See:
/// <url id="d1vo2h2vtfem27tm20e0" type="url" status="parsed" title="DWMWINDOWATTRIBUTE （dwmapi.h） - Win32 apps" wc="9823">https://docs.microsoft.com/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute</url>   
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

constexpr wchar_t kWindowClassName[] = L"FLUTTER_RUNNER_WIN32_WINDOW";

/// Registry key for app theme preference.
///
/// A value of 0 indicates apps should use dark mode. A non-zero or missing
/// value indicates apps should use light mode.
constexpr wchar_t kGetPreferredBrightnessRegKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
constexpr wchar_t kGetPreferredBrightnessRegValue[] = L"AppsUseLightTheme";

#define APPBAR_CALLBACK WM_USER + 0x01;

namespace {
    auto ValueOrNull(const flutter::EncodableMap& map, const char* key) -> const flutter::EncodableValue* {
        const auto it = map.find(flutter::EncodableValue(key));
        if (it == map.end()) {
            return nullptr;
        }
        return &(it->second);
    }

    class WindowManager {
    public:
        WindowManager();

        virtual ~WindowManager();

        HWND native_window;

        int last_state = STATE_NORMAL;

        bool has_shadow_ = false;
        bool is_always_on_bottom_ = false;
        bool is_frameless_ = false;
        bool is_prevent_close_ = false;
        double aspect_ratio_ = 0;
        POINT minimum_size_ = {0, 0};
        POINT maximum_size_ = {-1, -1};
        double pixel_ratio_ = 1;
        bool is_resizable_ = true;
        int is_docked_ = 0;
        bool is_registered_for_docking_ = false;
        bool is_skip_taskbar_ = true;
        std::string title_bar_style_ = "normal";
        double opacity_ = 1;

        bool is_resizing_ = false;
        bool is_moving_ = false;

        auto GetMainWindow() const -> HWND;
        auto ForceRefresh() const -> void;
        auto ForceChildRefresh() const -> void;
        auto SetAsFrameless() -> void;
        auto WaitUntilReadyToShow() -> void;
        static auto Destroy() -> void;
        auto Close() const -> void;
        auto IsPreventClose() const -> bool;
        auto SetPreventClose(const flutter::EncodableMap& args) -> void;
        auto Focus() -> void;
        auto Blur() const -> void;
        auto IsFocused() const -> bool;
        auto Show() const -> void;
        auto Hide() const -> void;
        auto IsVisible() const -> bool;
        auto IsMaximized() const -> bool;
        auto Maximize(const flutter::EncodableMap& args) const -> void;
        auto Unmaximize() const -> void;
        auto IsMinimized() const -> bool;
        auto Minimize() const -> void;
        auto Restore() const -> void;
        auto Dock(const flutter::EncodableMap& args) -> void;
        auto Undock() -> bool;
        auto IsFullScreen() const -> bool;
        auto SetFullScreen(const flutter::EncodableMap& args) -> void;
        auto SetAspectRatio(const flutter::EncodableMap& args) -> void;
        auto SetBackgroundColor(const flutter::EncodableMap& args) const -> void;
        auto GetBounds(const flutter::EncodableMap& args) const -> flutter::EncodableMap;
        auto SetBounds(const flutter::EncodableMap& args) const -> void;
        auto SetMinimumSize(const flutter::EncodableMap& args) -> void;
        auto SetMaximumSize(const flutter::EncodableMap& args) -> void;
        auto IsResizable() const -> bool;
        auto SetResizable(const flutter::EncodableMap& args) -> void;
        auto IsMinimizable() const -> bool;
        auto SetMinimizable(const flutter::EncodableMap& args) const -> void;
        auto IsMaximizable() const -> bool;
        auto SetMaximizable(const flutter::EncodableMap& args) const -> void;
        auto IsClosable() const -> bool;
        auto SetClosable(const flutter::EncodableMap& args) const -> void;
        auto IsAlwaysOnTop() const -> bool;
        auto SetAlwaysOnTop(const flutter::EncodableMap& args) const -> void;
        auto IsAlwaysOnBottom() const -> bool;
        auto SetAlwaysOnBottom(const flutter::EncodableMap& args) -> void;
        auto GetTitle() const -> std::string;
        auto SetTitle(const flutter::EncodableMap& args) const -> void;
        auto SetTitleBarStyle(const flutter::EncodableMap& args) -> void;
        auto GetTitleBarHeight() const -> int;
        auto IsSkipTaskbar() const -> bool;
        auto SetSkipTaskbar(const flutter::EncodableMap& args) -> void;
        auto SetProgressBar(const flutter::EncodableMap& args) const -> void;
        auto SetIcon(const flutter::EncodableMap& args) const -> void;
        auto HasShadow() const -> bool;
        auto SetHasShadow(const flutter::EncodableMap& args) -> void;
        auto GetOpacity() const -> double;
        auto SetOpacity(const flutter::EncodableMap& args) -> void;
        auto SetBrightness(const flutter::EncodableMap& args) const -> void;
        auto SetIgnoreMouseEvents(const flutter::EncodableMap& args) const -> void;
        auto PopUpWindowMenu(const flutter::EncodableMap& args) const -> void;
        auto StartDragging() -> void;
        auto StartResizing(const flutter::EncodableMap& args) -> void;

    private:
        static constexpr auto kFlutterViewWindowClassName = L"FLUTTERVIEW";
        bool g_is_window_fullscreen = false;
        std::string g_title_bar_style_before_fullscreen;
        RECT g_frame_before_fullscreen;
        bool g_maximized_before_fullscreen;
        LONG g_style_before_fullscreen;
        ITaskbarList3* taskbar_ = nullptr;
        static auto GetDpiForHwnd(HWND hWnd) -> double;
        auto RegisterAccessBar(HWND hwnd, BOOL fRegister) -> BOOL;
        static auto PASCAL AppBarQuerySetPos(HWND hwnd, UINT uEdge, LPRECT lprc, PAPPBARDATA pabd) -> void;
        auto DockAccessBar(HWND hwnd, UINT edge, UINT windowWidth) -> void;
    };

    WindowManager::WindowManager() {}

    WindowManager::~WindowManager() {}

    auto WindowManager::GetMainWindow() const -> HWND {
        return native_window;
    }

    auto WindowManager::ForceRefresh() const -> void {
        const HWND hWnd = GetMainWindow();

        RECT rect;

        GetWindowRect(hWnd, &rect);
        SetWindowPos(hWnd, nullptr, rect.left, rect.top, rect.right - rect.left + 1, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);
        SetWindowPos(hWnd, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);
    }

    auto WindowManager::ForceChildRefresh() const -> void {
        const HWND hWnd = GetWindow(GetMainWindow(), GW_CHILD);

        RECT rect;

        GetWindowRect(hWnd, &rect);
        SetWindowPos(hWnd, nullptr, rect.left, rect.top, rect.right - rect.left + 1, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);
        SetWindowPos(hWnd, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);
    }

    auto WindowManager::SetAsFrameless() -> void {
        is_frameless_ = true;
        const HWND hWnd = GetMainWindow();

        RECT rect;

        GetWindowRect(hWnd, &rect);
        SetWindowPos(hWnd, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    }

    auto WindowManager::WaitUntilReadyToShow() -> void {
        CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&taskbar_));
    }

    auto WindowManager::Destroy() -> void {
        PostQuitMessage(0);
    }

    auto WindowManager::Close() const -> void {
        const HWND hWnd = GetMainWindow();
        PostMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
    }

    auto WindowManager::SetPreventClose(const flutter::EncodableMap& args) -> void {
        is_prevent_close_ = args.at(flutter::EncodableValue("isPreventClose")).get<bool>();
    }

    auto WindowManager::IsPreventClose() const -> bool {
        return is_prevent_close_;
    }

    auto WindowManager::Focus() -> void {
        const HWND hWnd = GetMainWindow();
        if (IsMinimized()) {
            Restore();
        }

        SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        SetForegroundWindow(hWnd);
    }

    auto WindowManager::Blur() const -> void {
        const HWND hWnd = GetMainWindow();
        HWND next_hwnd = ::GetNextWindow(hWnd, GW_HWNDNEXT);
        while (next_hwnd) {
            if (IsWindowVisible(next_hwnd)) {
                SetForegroundWindow(next_hwnd);
                return;
            }
            next_hwnd = ::GetNextWindow(next_hwnd, GW_HWNDNEXT);
        }
    }

    auto WindowManager::IsFocused() const -> bool {
        return GetMainWindow() == GetForegroundWindow();
    }

    auto WindowManager::Show() const -> void {
        const HWND hWnd = GetMainWindow();
        DWORD gwlStyle = GetWindowLong(hWnd, GWL_STYLE);
        gwlStyle = gwlStyle | WS_VISIBLE;
        if ((gwlStyle & WS_VISIBLE) == 0) {
            SetWindowLong(hWnd, GWL_STYLE, gwlStyle);
            SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        }

        ShowWindow(GetMainWindow(), SW_SHOW);
        SetForegroundWindow(GetMainWindow());
    }

    auto WindowManager::Hide() const -> void {
        ShowWindow(GetMainWindow(), SW_HIDE);
    }

    auto WindowManager::IsVisible() const -> bool {
        const bool isVisible = IsWindowVisible(GetMainWindow());
        return isVisible;
    }

    auto WindowManager::IsMaximized() const -> bool {
        const HWND mainWindow = GetMainWindow();
        WINDOWPLACEMENT windowPlacement;
        GetWindowPlacement(mainWindow, &windowPlacement);

        return windowPlacement.showCmd == SW_MAXIMIZE;
    }

    auto WindowManager::Maximize(const flutter::EncodableMap& args) const -> void {
        const bool vertically = args.at(flutter::EncodableValue("vertically")).get<bool>();

        const HWND hwnd = GetMainWindow();
        WINDOWPLACEMENT windowPlacement;
        GetWindowPlacement(hwnd, &windowPlacement);

        if (vertically) {
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            PostMessage(hwnd, WM_NCLBUTTONDBLCLK, HTTOP, MAKELPARAM(cursorPos.x, cursorPos.y));
        } else {
            if (windowPlacement.showCmd != SW_MAXIMIZE) {
                PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
            }
        }
    }

    auto WindowManager::Unmaximize() const -> void {
        const HWND mainWindow = GetMainWindow();
        WINDOWPLACEMENT windowPlacement;
        GetWindowPlacement(mainWindow, &windowPlacement);

        if (windowPlacement.showCmd != SW_NORMAL) {
            PostMessage(mainWindow, WM_SYSCOMMAND, SC_RESTORE, 0);
        }
    }

    auto WindowManager::IsMinimized() const -> bool {
        const HWND mainWindow = GetMainWindow();
        WINDOWPLACEMENT windowPlacement;
        GetWindowPlacement(mainWindow, &windowPlacement);

        return windowPlacement.showCmd == SW_SHOWMINIMIZED;
    }

    auto WindowManager::Minimize() const -> void {
        if (IsFullScreen()) {
            // Like chromium, we don't want to minimize fullscreen
            // windows
            return;
        }
        const HWND mainWindow = GetMainWindow();
        WINDOWPLACEMENT windowPlacement;
        GetWindowPlacement(mainWindow, &windowPlacement);

        if (windowPlacement.showCmd != SW_SHOWMINIMIZED) {
            PostMessage(mainWindow, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }
    }

    auto WindowManager::Restore() const -> void {
        const HWND mainWindow = GetMainWindow();
        WINDOWPLACEMENT windowPlacement;
        GetWindowPlacement(mainWindow, &windowPlacement);

        if (windowPlacement.showCmd != SW_NORMAL) {
            PostMessage(mainWindow, WM_SYSCOMMAND, SC_RESTORE, 0);
        }
    }

    auto WindowManager::GetDpiForHwnd(HWND hWnd) -> double {
        const auto monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        UINT newDpiX = 96; // Default values
        UINT newDpiY = 96;

        // Dynamically load shcore.dll and get the GetDpiForMonitor function address
        // We need to do this to ensure Windows 7 support
        const HMODULE shcore = LoadLibrary(TEXT("shcore.dll"));
        if (shcore) {
            using GetDpiForMonitor = HRESULT(*)(HMONITOR, int, UINT*, UINT*);

            const auto GetDpiForMonitorFunc = (GetDpiForMonitor)GetProcAddress(shcore, "GetDpiForMonitor");

            if (GetDpiForMonitorFunc) {
                // Use the loaded function if available
                constexpr int MDT_EFFECTIVE_DPI = 0;
                if (FAILED(GetDpiForMonitorFunc(monitor, MDT_EFFECTIVE_DPI, &newDpiX, &newDpiY))) {
                    // If it fails, set the default values again
                    newDpiX = 96;
                    newDpiY = 96;
                }
            }
            FreeLibrary(shcore);
        }
        return newDpiX;
    }

    auto WindowManager::Dock(const flutter::EncodableMap& args) -> void {
        const HWND mainWindow = GetMainWindow();

        const double dpi = GetDpiForHwnd(mainWindow);
        const double scalingFactor = dpi / 96.0;

        const bool left = args.at(flutter::EncodableValue("left")).get<bool>();
        const bool right = args.at(flutter::EncodableValue("right")).get<bool>();
        const int width = args.at(flutter::EncodableValue("width")).get<int>();

        // first register bar
        RegisterAccessBar(mainWindow, true);

        //
        UINT edge = ABE_LEFT;
        if (right && !left) {
            edge = ABE_RIGHT;
        }

        const UINT uw = static_cast<UINT>(width * scalingFactor + 0.5);

        // dock window
        DockAccessBar(mainWindow, edge, uw);
    }

    auto WindowManager::Undock() -> bool {
        const HWND mainWindow = GetMainWindow();
        const bool result = RegisterAccessBar(mainWindow, false);
        is_docked_ = 0;
        return result;
    }

    auto PASCAL WindowManager::AppBarQuerySetPos(HWND hwnd, UINT uEdge, LPRECT lprc, PAPPBARDATA pabd) -> void {
        int iHeight = 0;
        int iWidth = 0;

        pabd->hWnd = hwnd;
        pabd->rc = *lprc;
        pabd->uEdge = uEdge;

        // Copy the screen coordinates of the appbar's bounding
        // rectangle into the APPBARDATA structure.
        if ((uEdge == ABE_LEFT) || (uEdge == ABE_RIGHT)) {
            iWidth = pabd->rc.right - pabd->rc.left;
            pabd->rc.top = 0;
            pabd->rc.bottom = GetSystemMetrics(SM_CYSCREEN);
        } else {
            iHeight = pabd->rc.bottom - pabd->rc.top;
            pabd->rc.left = 0;
            pabd->rc.right = GetSystemMetrics(SM_CXSCREEN);
        }

        // Query the system for an approved size and position.
        SHAppBarMessage(ABM_QUERYPOS, pabd);

        // Adjust the rectangle, depending on the edge to which the appbar is
        // anchored.
        switch (uEdge) {
            case ABE_LEFT:
                pabd->rc.right = pabd->rc.left + iWidth;
                break;

            case ABE_RIGHT:
                pabd->rc.left = pabd->rc.right - iWidth;
                break;

            case ABE_TOP:
                pabd->rc.bottom = pabd->rc.top + iHeight;
                break;

            case ABE_BOTTOM:
                pabd->rc.top = pabd->rc.bottom - iHeight;
                break;
        }

        // Pass the final bounding rectangle to the system.
        SHAppBarMessage(ABM_SETPOS, pabd);

        // Move and size the appbar so that it conforms to the
        // bounding rectangle passed to the system.
        constexpr UINT uFlags = NULL;
        SetWindowPos(hwnd, HWND_TOP, pabd->rc.left, pabd->rc.top, pabd->rc.right - pabd->rc.left, pabd->rc.bottom - pabd->rc.top, uFlags);
    }

    auto WindowManager::RegisterAccessBar(HWND hwnd, BOOL fRegister) -> BOOL {
        APPBARDATA abd;

        // Specify the structure size and handle to the appbar.
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hwnd;

        if (fRegister && is_registered_for_docking_) {
            return false;
        }

        if (!fRegister && !is_registered_for_docking_) {
            return false;
        }

        if (!fRegister && is_registered_for_docking_) {
            // Unregister the appbar.
            SHAppBarMessage(ABM_REMOVE, &abd);
            is_registered_for_docking_ = false;
            is_docked_ = 0;
            return true;
        }

        if (fRegister && !is_registered_for_docking_) {
            // Provide an identifier for notification messages.
            abd.uCallbackMessage = APPBAR_CALLBACK;

            // Register the appbar.
            if (!SHAppBarMessage(ABM_NEW, &abd)) {
                return false;
            }

            is_docked_ = 1; // default edge
            is_registered_for_docking_ = true;
            return false;
        }

        return false;
    }

    auto WindowManager::DockAccessBar(HWND hwnd, UINT edge, UINT windowWidth) -> void {
        APPBARDATA abd;
        RECT lprc;

        lprc.top = 0;
        lprc.bottom = 0;

        if (edge == ABE_LEFT) {
            lprc.left = 0;
            lprc.right = windowWidth;
        } else {
            lprc.left = GetSystemMetrics(SM_CXSCREEN) - windowWidth;
            lprc.right = GetSystemMetrics(SM_CXSCREEN);
        }

        // Specify the structure size and handle to the appbar.
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hwnd;
        abd.uCallbackMessage = APPBAR_CALLBACK;

        AppBarQuerySetPos(hwnd, edge, &lprc, &abd);

        if (edge == ABE_LEFT) {
            is_docked_ = 1;
        } else if (edge == ABE_RIGHT) {
            is_docked_ = 2;
        }
    }

    auto WindowManager::IsFullScreen() const -> bool {
        return g_is_window_fullscreen;
    }

    auto WindowManager::SetFullScreen(const flutter::EncodableMap& args) -> void {
        const bool isFullScreen = args.at(flutter::EncodableValue("isFullScreen")).get<bool>();

        const HWND mainWindow = GetMainWindow();

        // Previously inspired by how Chromium does this
        // <url id="d1vo2h2vtfem27tm20eg" type="url" status="parsed" title="src.chromium.org SVN" wc="287">https://src.chromium.org/viewvc/chrome/trunk/src/ui/views/win/fullscreen_handler.cc?revision=247204&view=markup</url>   
        // Instead, we use a modified implementation of how the media_kit package
        // implements this (we got permission from the author, I believe)
        // <url id="d1vo2h2vtfem27tm20f0" type="url" status="parsed" title="media-kit/media_kit_video/windows/utils.cc at 1226bcff36eab27cb17d60c33e9c15ca489c1f06 · media-kit/media-kit" wc="16">https://github.com/alexmercerind/media_kit/blob/1226bcff36eab27cb17d60c33e9c15ca489c1f06/media_kit_video/windows/utils.cc</url>   

        // Save current window state if not already fullscreen.
        if (!g_is_window_fullscreen) {
            // Save current window information.
            g_maximized_before_fullscreen = ::IsZoomed(mainWindow);
            g_style_before_fullscreen = GetWindowLong(mainWindow, GWL_STYLE);
            GetWindowRect(mainWindow, &g_frame_before_fullscreen);
            g_title_bar_style_before_fullscreen = title_bar_style_;
        }

        g_is_window_fullscreen = isFullScreen;

        if (isFullScreen) {
            // Set to fullscreen
            // ::SendMessage(mainWindow, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
            if (!is_frameless_) {
                auto monitor = MONITORINFO{};
                auto placement = WINDOWPLACEMENT{};
                monitor.cbSize = sizeof(MONITORINFO);
                placement.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(mainWindow, &placement);
                ::GetMonitorInfo(MonitorFromWindow(mainWindow, MONITOR_DEFAULTTONEAREST), &monitor);
                if (!g_maximized_before_fullscreen) {
                    SetAsFrameless();
                }
                ::SetWindowLongPtr(mainWindow, GWL_STYLE, g_style_before_fullscreen & ~(WS_THICKFRAME | WS_MAXIMIZEBOX));
                SetWindowPos(mainWindow, HWND_TOP, monitor.rcMonitor.left, monitor.rcMonitor.top, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                SetWindowPos(mainWindow,
                             HWND_TOP,
                             0,
                             0,
                             monitor.rcMonitor.right - monitor.rcMonitor.left,
                             monitor.rcMonitor.bottom - monitor.rcMonitor.top,
                             SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
        } else {
            // Restore from fullscreen
            // if (!g_maximized_before_fullscreen)
            //   Restore();
            ::SetWindowLongPtr(mainWindow, GWL_STYLE, g_style_before_fullscreen | (WS_THICKFRAME | WS_MAXIMIZEBOX));
            if (::IsZoomed(mainWindow)) {
                // Refresh the parent mainWindow.
                SetWindowPos(mainWindow, nullptr, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                auto rect = RECT{};
                GetClientRect(mainWindow, &rect);
                const auto flutter_view = ::FindWindowEx(mainWindow, nullptr, kFlutterViewWindowClassName, nullptr);
                SetWindowPos(flutter_view, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER);
                if (g_maximized_before_fullscreen) {
                    PostMessage(mainWindow, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                }
            } else {
                SetWindowPos(mainWindow,
                             nullptr,
                             g_frame_before_fullscreen.left,
                             g_frame_before_fullscreen.top,
                             g_frame_before_fullscreen.right - g_frame_before_fullscreen.left,
                             g_frame_before_fullscreen.bottom - g_frame_before_fullscreen.top,
                             SWP_NOACTIVATE | SWP_NOZORDER);

                // restore titlebar style
                title_bar_style_ = g_title_bar_style_before_fullscreen;
                is_frameless_ = false;
                constexpr MARGINS margins = {0, 0, 0, 0};
                RECT rect1;
                GetWindowRect(mainWindow, &rect1);
                DwmExtendFrameIntoClientArea(mainWindow, &margins);
                SetWindowPos(mainWindow, nullptr, rect1.left, rect1.top, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
            }
        }
    }

    auto WindowManager::SetAspectRatio(const flutter::EncodableMap& args) -> void {
        aspect_ratio_ = args.at(flutter::EncodableValue("aspectRatio")).get<double>();
    }

    auto WindowManager::SetBackgroundColor(const flutter::EncodableMap& args) const -> void {
        const int backgroundColorA = args.at(flutter::EncodableValue("backgroundColorA")).get<int>();
        const int backgroundColorR = args.at(flutter::EncodableValue("backgroundColorR")).get<int>();
        const int backgroundColorG = args.at(flutter::EncodableValue("backgroundColorG")).get<int>();
        const int backgroundColorB = args.at(flutter::EncodableValue("backgroundColorB")).get<int>();

        const bool isTransparent = backgroundColorA == 0 && backgroundColorR == 0 && backgroundColorG == 0 && backgroundColorB == 0;

        const HWND hWnd = GetMainWindow();
        const HINSTANCE hModule = LoadLibrary(TEXT("user32.dll"));
        if (hModule) {
            using ACCENT_STATE = enum _ACCENT_STATE {
                ACCENT_DISABLED                   = 0,
                ACCENT_ENABLE_GRADIENT            = 1,
                ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
                ACCENT_ENABLE_BLURBEHIND          = 3,
                ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4,
                ACCENT_ENABLE_HOSTBACKDROP        = 5,
                ACCENT_INVALID_STATE              = 6
            };
            struct ACCENTPOLICY {
                int nAccentState;
                int nFlags;
                int nColor;
                int nAnimationId;
            };
            struct WINCOMPATTRDATA {
                int nAttribute;
                PVOID pData;
                ULONG ulDataSize;
            };
            using pSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINCOMPATTRDATA*);
            const auto SetWindowCompositionAttribute = reinterpret_cast<pSetWindowCompositionAttribute>(GetProcAddress(hModule, "SetWindowCompositionAttribute"));
            if (SetWindowCompositionAttribute) {
                const int32_t accent_state = isTransparent ? ACCENT_ENABLE_TRANSPARENTGRADIENT : ACCENT_ENABLE_GRADIENT;
                ACCENTPOLICY policy = {accent_state, 2, ((backgroundColorA << 24) + (backgroundColorB << 16) + (backgroundColorG << 8) + (backgroundColorR)), 0};
                WINCOMPATTRDATA data = {19, &policy, sizeof(policy)};
                SetWindowCompositionAttribute(hWnd, &data);
            }
            FreeLibrary(hModule);
        }
    }

    auto WindowManager::GetBounds(const flutter::EncodableMap& args) const -> flutter::EncodableMap {
        const HWND hwnd = GetMainWindow();
        const double devicePixelRatio = args.at(flutter::EncodableValue("devicePixelRatio")).get<double>();

        auto resultMap = flutter::EncodableMap();
        RECT rect;
        if (GetWindowRect(hwnd, &rect)) {
            double x = rect.left / devicePixelRatio * 1.0f;
            double y = rect.top / devicePixelRatio * 1.0f;
            double width = (rect.right - rect.left) / devicePixelRatio * 1.0f;
            double height = (rect.bottom - rect.top) / devicePixelRatio * 1.0f;

            resultMap[flutter::EncodableValue("x")] = flutter::EncodableValue(x);
            resultMap[flutter::EncodableValue("y")] = flutter::EncodableValue(y);
            resultMap[flutter::EncodableValue("width")] = flutter::EncodableValue(width);
            resultMap[flutter::EncodableValue("height")] = flutter::EncodableValue(height);
        }
        return resultMap;
    }

    auto WindowManager::SetBounds(const flutter::EncodableMap& args) const -> void {
        const HWND hwnd = GetMainWindow();

        const double devicePixelRatio = args.at(flutter::EncodableValue("devicePixelRatio")).get<double>();

        auto* null_or_x = std::get_if<double>(&ValueOrNull(args, "x")->value());
        auto* null_or_y = std::get_if<double>(&ValueOrNull(args, "y")->value());
        auto* null_or_width = std::get_if<double>(&ValueOrNull(args, "width")->value());
        auto* null_or_height = std::get_if<double>(&ValueOrNull(args, "height")->value());

        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        UINT uFlags = NULL;

        if (null_or_x != nullptr && null_or_y != nullptr) {
            x = static_cast<int>(*null_or_x * devicePixelRatio);
            y = static_cast<int>(*null_or_y * devicePixelRatio);
        }
        if (null_or_width != nullptr && null_or_height != nullptr) {
            width = static_cast<int>(*null_or_width * devicePixelRatio);
            height = static_cast<int>(*null_or_height * devicePixelRatio);
        }

        if (null_or_x == nullptr || null_or_y == nullptr) {
            uFlags = SWP_NOMOVE;
        }
        if (null_or_width == nullptr || null_or_height == nullptr) {
            uFlags = SWP_NOSIZE;
        }

        SetWindowPos(hwnd, HWND_TOP, x, y, width, height, uFlags);
    }

    auto WindowManager::SetMinimumSize(const flutter::EncodableMap& args) -> void {
        const double devicePixelRatio = args.at(flutter::EncodableValue("devicePixelRatio")).get<double>();
        const double width = args.at(flutter::EncodableValue("width")).get<double>();
        const double height = args.at(flutter::EncodableValue("height")).get<double>();

        if (width >= 0 && height >= 0) {
            pixel_ratio_ = devicePixelRatio;
            POINT point = {};
            point.x = static_cast<LONG>(width);
            point.y = static_cast<LONG>(height);
            minimum_size_ = point;
        }
    }

    auto WindowManager::SetMaximumSize(const flutter::EncodableMap& args) -> void {
        const double devicePixelRatio = args.at(flutter::EncodableValue("devicePixelRatio")).get<double>();
        const double width = args.at(flutter::EncodableValue("width")).get<double>();
        const double height = args.at(flutter::EncodableValue("height")).get<double>();

        if (width >= 0 && height >= 0) {
            pixel_ratio_ = devicePixelRatio;
            POINT point = {};
            point.x = static_cast<LONG>(width);
            point.y = static_cast<LONG>(height);
            maximum_size_ = point;
        }
    }

    auto WindowManager::IsResizable() const -> bool {
        return is_resizable_;
    }

    auto WindowManager::SetResizable(const flutter::EncodableMap& args) -> void {
        const HWND hWnd = GetMainWindow();
        is_resizable_ = args.at(flutter::EncodableValue("isResizable")).get<bool>();
        DWORD gwlStyle = GetWindowLong(hWnd, GWL_STYLE);
        if (is_resizable_) {
            gwlStyle |= WS_THICKFRAME;
        } else {
            gwlStyle &= ~WS_THICKFRAME;
        }
        ::SetWindowLong(hWnd, GWL_STYLE, gwlStyle);
    }

    auto WindowManager::IsMinimizable() const -> bool {
        const HWND hWnd = GetMainWindow();
        const DWORD gwlStyle = GetWindowLong(hWnd, GWL_STYLE);
        return (gwlStyle & WS_MINIMIZEBOX) != 0;
    }

    auto WindowManager::SetMinimizable(const flutter::EncodableMap& args) const -> void {
        const HWND hWnd = GetMainWindow();
        const bool isMinimizable = args.at(flutter::EncodableValue("isMinimizable")).get<bool>();
        DWORD gwlStyle = GetWindowLong(hWnd, GWL_STYLE);
        gwlStyle = isMinimizable ? gwlStyle | WS_MINIMIZEBOX : gwlStyle & ~WS_MINIMIZEBOX;
        SetWindowLong(hWnd, GWL_STYLE, gwlStyle);
    }

    auto WindowManager::IsMaximizable() const -> bool {
        const HWND hWnd = GetMainWindow();
        const DWORD gwlStyle = GetWindowLong(hWnd, GWL_STYLE);
        return (gwlStyle & WS_MAXIMIZEBOX) != 0;
    }

    auto WindowManager::SetMaximizable(const flutter::EncodableMap& args) const -> void {
        const HWND hWnd = GetMainWindow();
        const bool isMaximizable = args.at(flutter::EncodableValue("isMaximizable")).get<bool>();
        DWORD gwlStyle = GetWindowLong(hWnd, GWL_STYLE);
        gwlStyle = isMaximizable ? gwlStyle | WS_MAXIMIZEBOX : gwlStyle & ~WS_MAXIMIZEBOX;
        SetWindowLong(hWnd, GWL_STYLE, gwlStyle);
    }

    auto WindowManager::IsClosable() const -> bool {
        const HWND hWnd = GetMainWindow();
        const DWORD gclStyle = GetClassLong(hWnd, GCL_STYLE);
        return !((gclStyle & CS_NOCLOSE) != 0);
    }

    auto WindowManager::SetClosable(const flutter::EncodableMap& args) const -> void {
        const HWND hWnd = GetMainWindow();
        const bool isClosable = args.at(flutter::EncodableValue("isClosable")).get<bool>();
        DWORD gclStyle = GetClassLong(hWnd, GCL_STYLE);
        gclStyle = isClosable ? gclStyle & ~CS_NOCLOSE : gclStyle | CS_NOCLOSE;
        SetClassLong(hWnd, GCL_STYLE, gclStyle);
    }

    auto WindowManager::IsAlwaysOnTop() const -> bool {
        const DWORD dwExStyle = GetWindowLong(GetMainWindow(), GWL_EXSTYLE);
        return (dwExStyle & WS_EX_TOPMOST) != 0;
    }

    auto WindowManager::SetAlwaysOnTop(const flutter::EncodableMap& args) const -> void {
        const bool isAlwaysOnTop = args.at(flutter::EncodableValue("isAlwaysOnTop")).get<bool>();
        SetWindowPos(GetMainWindow(), isAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    auto WindowManager::IsAlwaysOnBottom() const -> bool {
        return is_always_on_bottom_;
    }

    auto WindowManager::SetAlwaysOnBottom(const flutter::EncodableMap& args) -> void {
        is_always_on_bottom_ = args.at(flutter::EncodableValue("isAlwaysOnBottom")).get<bool>();

        SetWindowPos(GetMainWindow(), is_always_on_bottom_ ? HWND_BOTTOM : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    auto WindowManager::GetTitle() const -> std::string {
        const int bufferSize = 1 + GetWindowTextLength(GetMainWindow());
        std::wstring title(bufferSize, L'\0');
        GetWindowText(GetMainWindow(), title.data(), bufferSize);

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(title);
    }

    auto WindowManager::SetTitle(const flutter::EncodableMap& args) const -> void {
        const auto title = args.at(flutter::EncodableValue("title")).get<std::string>();

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        SetWindowText(GetMainWindow(), converter.from_bytes(title).c_str());
    }

    auto WindowManager::SetTitleBarStyle(const flutter::EncodableMap& args) -> void {
        title_bar_style_ = args.at(flutter::EncodableValue("titleBarStyle")).get<std::string>();
        // Enables the ability to go from setAsFrameless() to
        // TitleBarStyle.normal/hidden
        is_frameless_ = false;

        constexpr MARGINS margins = {0, 0, 0, 0};
        const HWND hWnd = GetMainWindow();
        RECT rect;
        GetWindowRect(hWnd, &rect);
        DwmExtendFrameIntoClientArea(hWnd, &margins);
        SetWindowPos(hWnd, nullptr, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    }

    auto WindowManager::GetTitleBarHeight() const -> int {
        const HWND hWnd = GetMainWindow();

        auto ptinfo = static_cast<TITLEBARINFOEX*>(malloc(sizeof(TITLEBARINFOEX)));
        ptinfo->cbSize = sizeof(TITLEBARINFOEX);
        SendMessage(hWnd, WM_GETTITLEBARINFOEX, 0, (LPARAM)ptinfo);
        const int height = ptinfo->rcTitleBar.bottom == 0 ? 0 : ptinfo->rcTitleBar.bottom - ptinfo->rcTitleBar.top;
        free(ptinfo);

        return height;
    }

    auto WindowManager::IsSkipTaskbar() const -> bool {
        return is_skip_taskbar_;
    }

    auto WindowManager::SetSkipTaskbar(const flutter::EncodableMap& args) -> void {
        is_skip_taskbar_ = args.at(flutter::EncodableValue("isSkipTaskbar")).get<bool>();

        const HWND hWnd = GetMainWindow();

        const LPVOID lp = nullptr;
        CoInitialize(lp);

        taskbar_->HrInit();
        if (!is_skip_taskbar_) {
            taskbar_->AddTab(hWnd);
        } else {
            taskbar_->DeleteTab(hWnd);
        }
    }

    auto WindowManager::SetProgressBar(const flutter::EncodableMap& args) const -> void {
        const double progress = args.at(flutter::EncodableValue("progress")).get<double>();

        const HWND hWnd = GetMainWindow();
        taskbar_->SetProgressState(hWnd, TBPF_INDETERMINATE);
        taskbar_->SetProgressValue(hWnd, static_cast<int32_t>(progress * 100), 100);

        if (progress < 0) {
            taskbar_->SetProgressState(hWnd, TBPF_NOPROGRESS);
            taskbar_->SetProgressValue(hWnd, 0, 0);
        } else if (progress > 1) {
            taskbar_->SetProgressState(hWnd, TBPF_INDETERMINATE);
            taskbar_->SetProgressValue(hWnd, 100, 100);
        } else {
            taskbar_->SetProgressState(hWnd, TBPF_INDETERMINATE);
            taskbar_->SetProgressValue(hWnd, static_cast<int32_t>(progress * 100), 100);
        }
    }

    auto WindowManager::SetIcon(const flutter::EncodableMap& args) const -> void {
        const auto iconPath = args.at(flutter::EncodableValue("iconPath")).get<std::string>();

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

        auto hIconSmall = static_cast<HICON>((LoadImage(nullptr, converter.from_bytes(iconPath).c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE)));

        auto hIconLarge = static_cast<HICON>((LoadImage(nullptr, converter.from_bytes(iconPath).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE)));

        const HWND hWnd = GetMainWindow();

        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);
    }

    auto WindowManager::HasShadow() const -> bool {
        if (is_frameless_) {
            return has_shadow_;
        }
        return true;
    }

    auto WindowManager::SetHasShadow(const flutter::EncodableMap& args) -> void {
        if (is_frameless_) {
            has_shadow_ = args.at(flutter::EncodableValue("hasShadow")).get<bool>();

            const HWND hWnd = GetMainWindow();

            constexpr MARGINS margins[2]{{0, 0, 0, 0}, {0, 0, 1, 0}};

            DwmExtendFrameIntoClientArea(hWnd, &margins[has_shadow_]);
        }
    }

    auto WindowManager::GetOpacity() const -> double {
        return opacity_;
    }

    auto WindowManager::SetOpacity(const flutter::EncodableMap& args) -> void {
        opacity_ = args.at(flutter::EncodableValue("opacity")).get<double>();
        const HWND hWnd = GetMainWindow();
        const long gwlExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        SetWindowLong(hWnd, GWL_EXSTYLE, gwlExStyle | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hWnd, 0, static_cast<int8_t>(255 * opacity_), 0x02);
    }

    auto WindowManager::SetBrightness(const flutter::EncodableMap& args) const -> void {
        DWORD light_mode;
        DWORD light_mode_size = sizeof(light_mode);
        const LSTATUS result = RegGetValue(HKEY_CURRENT_USER, kGetPreferredBrightnessRegKey, kGetPreferredBrightnessRegValue, RRF_RT_REG_DWORD, nullptr, &light_mode, &light_mode_size);

        if (result == ERROR_SUCCESS) {
            const auto brightness = args.at(flutter::EncodableValue("brightness")).get<std::string>();
            const HWND hWnd = GetMainWindow();
            const BOOL enable_dark_mode = light_mode == 0 && brightness == "dark";
            DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable_dark_mode, sizeof(enable_dark_mode));
        }
    }

    auto WindowManager::SetIgnoreMouseEvents(const flutter::EncodableMap& args) const -> void {
        const bool ignore = args.at(flutter::EncodableValue("ignore")).get<bool>();

        const HWND hwnd = GetMainWindow();
        LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        if (ignore) {
            ex_style |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
        } else {
            ex_style &= ~(WS_EX_TRANSPARENT | WS_EX_LAYERED);
        }

        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
    }

    auto WindowManager::PopUpWindowMenu(const flutter::EncodableMap& args) const -> void {
        const HWND hWnd = GetMainWindow();
        const HMENU hMenu = GetSystemMenu(hWnd, false);

        double x, y;

        POINT cursorPos;
        GetCursorPos(&cursorPos);
        x = cursorPos.x;
        y = cursorPos.y;

        const int cmd = TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, static_cast<int>(x), static_cast<int>(y), 0, hWnd, nullptr);

        if (cmd) {
            PostMessage(hWnd, WM_SYSCOMMAND, cmd, 0);
        }
    }

    auto WindowManager::StartDragging() -> void {
        ReleaseCapture();
        Undock();
        SendMessage(GetMainWindow(), WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
    }

    auto WindowManager::StartResizing(const flutter::EncodableMap& args) -> void {
        const bool top = args.at(flutter::EncodableValue("top")).get<bool>();
        const bool bottom = args.at(flutter::EncodableValue("bottom")).get<bool>();
        const bool left = args.at(flutter::EncodableValue("left")).get<bool>();
        const bool right = args.at(flutter::EncodableValue("right")).get<bool>();

        const HWND hWnd = GetMainWindow();
        Undock();
        ReleaseCapture();
        LONG command;
        if (top && !bottom && !right && !left) {
            command = HTTOP;
        } else if (top && left && !bottom && !right) {
            command = HTTOPLEFT;
        } else if (left && !top && !bottom && !right) {
            command = HTLEFT;
        } else if (right && !top && !left && !bottom) {
            command = HTRIGHT;
        } else if (top && right && !left && !bottom) {
            command = HTTOPRIGHT;
        } else if (bottom && !top && !right && !left) {
            command = HTBOTTOM;
        } else if (bottom && left && !top && !right) {
            command = HTBOTTOMLEFT;
        } else {
            command = HTBOTTOMRIGHT;
        }
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        PostMessage(hWnd, WM_NCLBUTTONDOWN, command, MAKELPARAM(cursorPos.x, cursorPos.y));
    }
} // namespace
