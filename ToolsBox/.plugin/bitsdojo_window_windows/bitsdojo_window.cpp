#include <cstdio>
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <cmath>
#include "bitsdojo_window_common.h"
#include "bitsdojo_window.h"
#include "window_util.h"
#include "bitsdojo_window_plugin.h"

namespace bitsdojo_window {
    static UINT (*GetDpiForWindow)(HWND) = [](HWND) {
        return 96u;
    };
    static int (*GetSystemMetricsForDpi)(int, UINT) = [](int nIndex, UINT) {
        return GetSystemMetrics(nIndex);
    };

    static HWND flutter_window = nullptr;
    static HWND flutter_child_window = nullptr;
    static HHOOK flutterWindowMonitor = nullptr;
    static BOOL is_bitsdojo_window_loaded = FALSE;
    static BOOL during_minimize = FALSE;
    static BOOL during_maximize = FALSE;
    static BOOL during_restore = FALSE;
    static BOOL bypass_wm_size = FALSE;
    static BOOL has_custom_frame = FALSE;
    static BOOL visible_on_startup = TRUE;
    static BOOL window_can_be_shown = FALSE;
    static BOOL restore_by_moving = FALSE;
    static BOOL during_size_move = FALSE;
    static BOOL dpi_changed_during_size_move = FALSE;
    static BOOL is_dpi_aware = FALSE;
    static SIZE min_size = {0, 0};
    static SIZE max_size = {0, 0};
    // Amount to cut when window is maximized
    static int window_cut_on_maximize = 0;

    // Forward declarations
    static auto init() -> int;
    static auto monitorFlutterWindows() -> void;

    static auto bdw_init = init();

    auto isBitsdojoWindowLoaded() -> bool {
        return is_bitsdojo_window_loaded;
    }

    auto init() -> int {
        is_bitsdojo_window_loaded = true;
        if (auto user32 = LoadLibraryA("User32.dll")) {
            if (auto fn = GetProcAddress(user32, "GetDpiForWindow")) {
                is_dpi_aware = true;
                GetDpiForWindow = reinterpret_cast<decltype(GetDpiForWindow)>(fn);
                GetSystemMetricsForDpi = reinterpret_cast<decltype(GetSystemMetricsForDpi)>(GetProcAddress(user32, "GetSystemMetricsForDpi"));
            }
        }
        monitorFlutterWindows();
        return 1;
    }

    static auto configure(unsigned int flags) -> int {
        has_custom_frame = (flags & BDW_CUSTOM_FRAME);
        visible_on_startup = !(flags & BDW_HIDE_ON_STARTUP);
        return 1;
    }

    auto setWindowCutOnMaximize(int value) -> void {
        window_cut_on_maximize = value;
    }

    auto setMinSize(int width, int height) -> void {
        min_size.cx = width;
        min_size.cy = height;
    }

    auto setMaxSize(int width, int height) -> void {
        max_size.cx = width;
        max_size.cy = height;
    }

    auto setWindowCanBeShown(bool value) -> void {
        window_can_be_shown = value;
    }

    auto getAppWindow() -> HWND {
        return flutter_window;
    }

    auto isDPIAware() -> bool {
        return is_dpi_aware;
    }

    static auto CALLBACK main_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR subclassID, DWORD_PTR refData) -> LRESULT;
    static auto CALLBACK child_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR subclassID, DWORD_PTR refData) -> LRESULT;

    static auto CALLBACK monitorFlutterWindowsProc(_In_ int code, _In_ WPARAM wparam, _In_ LPARAM lparam) -> LRESULT {
        if (code == HCBT_CREATEWND) {
            auto createParams = reinterpret_cast<CBT_CREATEWND*>(lparam);
            if (!createParams->lpcs->lpCreateParams) {
                return 0;
            }
            if (wcscmp(createParams->lpcs->lpszClass, L"FLUTTER_RUNNER_WIN32_WINDOW") == 0) {
                flutter_window = (HWND)wparam;
                SetWindowSubclass(flutter_window, main_window_proc, 1, NULL);
            } else if (wcscmp(createParams->lpcs->lpszClass, L"FLUTTERVIEW") == 0) {
                flutter_child_window = (HWND)wparam;
                SetWindowSubclass(flutter_child_window, child_window_proc, 1, NULL);
            }
        }
        if ((flutter_window != nullptr) && (flutter_child_window != nullptr)) {
            UnhookWindowsHookEx(flutterWindowMonitor);
        }
        return 0;
    }

    auto monitorFlutterWindows() -> void {
        DWORD threadID = GetCurrentThreadId();
        flutterWindowMonitor = SetWindowsHookEx(WH_CBT, monitorFlutterWindowsProc, nullptr, threadID);
    }

    auto CALLBACK main_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR subclassID, DWORD_PTR refData) -> LRESULT;

    static auto forceChildRefresh() -> void {
        if (flutter_child_window == nullptr) {
            return;
        }

        RECT rc;
        GetClientRect(flutter_window, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        SetWindowPos(flutter_child_window, nullptr, 0, 0, width + 1, height + 1, SWP_NOMOVE | SWP_NOACTIVATE);
        SetWindowPos(flutter_child_window, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOACTIVATE);
    }

    static auto getResizeMargin(HWND window) -> int {
        UINT currentDpi = GetDpiForWindow(window);
        int resizeBorder = GetSystemMetricsForDpi(SM_CXSIZEFRAME, currentDpi);
        int borderPadding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, currentDpi);
        bool isMaximized = IsZoomed(window);
        if (isMaximized) {
            return borderPadding;
        }
        return resizeBorder + borderPadding;
    }

    static auto extendIntoClientArea(HWND hwnd) -> void {
        MARGINS margins = {0, 0, 1, 0};
        DwmExtendFrameIntoClientArea(hwnd, &margins);
    }

    static auto handle_nchittest(HWND window, WPARAM wparam, LPARAM lparam) -> LRESULT {
        bool isMaximized = IsZoomed(flutter_window);
        if (isMaximized) {
            return HTCLIENT;
        }
        POINT pt = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        ScreenToClient(window, &pt);
        RECT rc;
        GetClientRect(window, &rc);
        int resizeMargin = getResizeMargin(window);
        if (pt.y < resizeMargin) {
            if (pt.x < resizeMargin) {
                return HTTOPLEFT;
            }
            if (pt.x > (rc.right - resizeMargin)) {
                return HTTOPRIGHT;
            }
            return HTTOP;
        }
        if (pt.y > (rc.bottom - resizeMargin)) {
            if (pt.x < resizeMargin) {
                return HTBOTTOMLEFT;
            }
            if (pt.x > (rc.right - resizeMargin)) {
                return HTBOTTOMRIGHT;
            }
            return HTBOTTOM;
        }
        if (pt.x < resizeMargin) {
            return HTLEFT;
        }
        if (pt.x > (rc.right - resizeMargin)) {
            return HTRIGHT;
        }
        return HTCLIENT;
    }


    static auto getScreenRectForWindow(HWND window) -> RECT {
        MONITORINFO monitorInfo = {};
        monitorInfo.cbSize = static_cast<DWORD>(sizeof(MONITORINFO));
        auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
        GetMonitorInfoW(monitor, &monitorInfo);
        return monitorInfo.rcMonitor;
    }

    static auto getWorkingScreenRectForWindow(HWND window) -> RECT {
        MONITORINFO monitorInfo = {};
        monitorInfo.cbSize = static_cast<DWORD>(sizeof(MONITORINFO));
        auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
        GetMonitorInfoW(monitor, &monitorInfo);
        return monitorInfo.rcWork;
    }

    static auto adjustPositionOnRestoreByMove(HWND window, WINDOWPOS* winPos) -> void {
        if (restore_by_moving == FALSE) {
            return;
        }

        auto screenRect = getWorkingScreenRectForWindow(window);

        if (winPos->y < screenRect.top) {
            winPos->y = screenRect.top;
        }
    }

    static auto adjustMaximizedSize(HWND window, WINDOWPOS* winPos) -> void {
        auto screenRect = getWorkingScreenRectForWindow(window);
        if ((winPos->x < screenRect.left) && (winPos->y < screenRect.top) && (winPos->cx > (screenRect.right - screenRect.left)) && (winPos->cy > (screenRect.bottom - screenRect.top))) {
            winPos->x = screenRect.left;
            winPos->y = screenRect.top;
            winPos->cx = screenRect.right - screenRect.left;
            winPos->cy = screenRect.bottom - screenRect.top;
        }
    }

    static auto adjustMaximizedRects(HWND window, NCCALCSIZE_PARAMS* params) -> void {
        auto screenRect = getWorkingScreenRectForWindow(window);
        for (auto& [left, top, right, bottom] : params->rgrc) {
            if ((left < screenRect.left) && (top < screenRect.top) && (right > screenRect.right) && (bottom > screenRect.bottom)) {
                left = screenRect.left;
                top = screenRect.top;
                right = screenRect.right;
                bottom = screenRect.bottom;
            }
        }
    }

    static auto getScaleFactor(HWND window) -> double {
        UINT dpi = GetDpiForWindow(window);
        return dpi / 96.0;
    }

    static auto handle_nccalcsize(HWND window, WPARAM wparam, LPARAM lparam) -> LRESULT {
        if (!wparam) {
            return 0;
        }

        auto params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
        if (params->lppos) {
            adjustMaximizedSize(window, params->lppos);
        }
        adjustMaximizedRects(window, params);

        auto initialRect = params->rgrc[0];
        auto defaultResult = DefSubclassProc(window, WM_NCCALCSIZE, wparam, lparam);

        if (defaultResult != 0) {
            return defaultResult;
        }

        bool isMaximized = IsZoomed(window);
        params->rgrc[0] = initialRect;
        double scaleFactor = getScaleFactor(window);
        int scaleFactorInt = static_cast<int>(ceil(scaleFactor));

        if (isMaximized) {
            int sidesCut = static_cast<int>(ceil(scaleFactor * window_cut_on_maximize));
            int topCut = static_cast<int>(ceil(scaleFactor * window_cut_on_maximize)) + scaleFactorInt + 1;

            params->rgrc[0].top -= topCut;
            params->rgrc[0].left -= sidesCut;
            params->rgrc[0].right += sidesCut;
            params->rgrc[0].bottom += sidesCut;
        } else {
            params->rgrc[0].top -= 1;
        }

        return 0;
    }

    static constexpr long kWmDpiChangedBeforeParent = 0x02E2;

    static auto fixDPIScaling() -> void {
        SendMessage(flutter_child_window, kWmDpiChangedBeforeParent, 0, 0);
        forceChildRefresh();
    }

    auto CALLBACK child_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR subclassID, DWORD_PTR refData) -> LRESULT {
        switch (message) {
            case WM_CREATE: {
                LRESULT result = DefSubclassProc(window, message, wparam, lparam);
                fixDPIScaling();
                return result;
            }
            case WM_NCHITTEST: {
                if (has_custom_frame == FALSE) {
                    break;
                }
                LRESULT result = handle_nchittest(window, wparam, lparam);
                if (result != HTCLIENT) {
                    return HTTRANSPARENT;
                }
                break;
            }
        }
        return DefSubclassProc(window, message, wparam, lparam);
    }

    static auto adjustChildWindowSize() -> void {
        RECT clientRect;
        GetClientRect(flutter_window, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        SetWindowPos(flutter_child_window, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOACTIVATE);
    }

    static auto getSizeOnScreen(SIZE* _size) -> void {
        UINT dpi = GetDpiForWindow(flutter_window);
        double scale_factor = dpi / 96.0;
        _size->cx = static_cast<int>(_size->cx * scale_factor);
        _size->cy = static_cast<int>(_size->cy * scale_factor);
    }

    static auto centerOnMonitorContainingMouse(HWND window, int width, int height) -> bool {
        MONITORINFO monitorInfo = {};
        monitorInfo.cbSize = static_cast<DWORD>(sizeof(MONITORINFO));

        POINT mousePosition;
        if (GetCursorPos(&mousePosition) == FALSE) {
            return false;
        }
        auto monitor = MonitorFromPoint(mousePosition, MONITOR_DEFAULTTONEAREST);
        if (GetMonitorInfoW(monitor, &monitorInfo) == FALSE) {
            return false;
        }
        auto monitorWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
        auto monitorHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
        auto x = (monitorWidth - width) / 2;
        auto y = (monitorHeight - height) / 2;
        x += monitorInfo.rcWork.left;
        y += monitorInfo.rcWork.top;
        SetWindowPos(window, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
        return true;
    }

    static auto handle_bdw_action(HWND window, WPARAM action, LPARAM actionData) -> void {
        switch (action) {
            case BDW_SETWINDOWPOS: {
                auto param = (SWPParam*)(actionData);
                SetWindowPos(window, nullptr, param->x, param->y, param->cx, param->cy, param->uFlags);
                HeapFree(GetProcessHeap(), 0, param);
                break;
            }
            case BDW_SETWINDOWTEXT: {
                auto param = (SWTParam*)(actionData);
                SetWindowText(window, param->text);
                HeapFree(GetProcessHeap(), 0, (LPVOID)param->text);
                HeapFree(GetProcessHeap(), 0, param);
                break;
            }
            case BDW_FORCECHILDREFRESH: {
                forceChildRefresh();
                break;
            }
        }
    }


    auto CALLBACK main_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR subclssID, DWORD_PTR refData) -> LRESULT {
        switch (message) {
            case WM_ERASEBKGND: {
                return 1;
            }
            case WM_NCCREATE: {
                flutter_window = window;
                auto style = GetWindowLongPtr(window, GWL_STYLE);
                style = style | WS_CLIPCHILDREN;
                SetWindowLongPtr(window, GWL_STYLE, style);
                SetProp(window, L"BitsDojoWindow", (HANDLE)(1));
                break;
            }
            case WM_NCHITTEST: {
                if (has_custom_frame == FALSE) {
                    break;
                }
                return handle_nchittest(window, wparam, lparam);
            }
            case WM_NCCALCSIZE: {
                if (has_custom_frame == FALSE) {
                    break;
                }
                return handle_nccalcsize(window, wparam, lparam);
            }
            case WM_CREATE: {
                auto createStruct = reinterpret_cast<CREATESTRUCT*>(lparam);
                LRESULT result = DefSubclassProc(window, message, wparam, lparam);
                if (has_custom_frame == TRUE) {
                    extendIntoClientArea(window);
                    SetWindowPos(window, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);
                }
                centerOnMonitorContainingMouse(window, createStruct->cx, createStruct->cy);
                if (visible_on_startup == TRUE) {
                    setWindowCanBeShown(TRUE);
                    forceChildRefresh();
                }
                return result;
            }
            case WM_DPICHANGED: {
                if (during_size_move == TRUE) {
                    dpi_changed_during_size_move = TRUE;
                }
                forceChildRefresh();
                break;
            }
            case WM_SIZE: {
                if (during_minimize == TRUE) {
                    return 0;
                }
                if (bypass_wm_size == TRUE) {
                    return DefWindowProc(window, message, wparam, lparam);
                }
                break;
            }
            case WM_SYSCOMMAND: {
                if (wparam == SC_MINIMIZE) {
                    during_minimize = TRUE;
                }
                if (wparam == SC_MAXIMIZE) {
                    during_maximize = TRUE;
                }
                if (wparam == SC_RESTORE) {
                    during_restore = TRUE;
                }
                LRESULT result = DefSubclassProc(window, message, wparam, lparam);
                during_minimize = FALSE;
                during_maximize = FALSE;
                during_restore = FALSE;
                return result;
            }
            case WM_WINDOWPOSCHANGING: {
                auto winPos = reinterpret_cast<WINDOWPOS*>(lparam);
                bool isResize = !(winPos->flags & SWP_NOSIZE);
                if (has_custom_frame && isResize) {
                    adjustMaximizedSize(window, winPos);
                    adjustPositionOnRestoreByMove(window, winPos);
                }
                BOOL isShowWindow = ((winPos->flags & SWP_SHOWWINDOW) == SWP_SHOWWINDOW);

                if ((isShowWindow == TRUE) && (window_can_be_shown == FALSE) && (visible_on_startup == FALSE)) {
                    winPos->flags &= ~SWP_SHOWWINDOW;
                }

                break;
            }
            case WM_WINDOWPOSCHANGED: {
                auto winPos = reinterpret_cast<WINDOWPOS*>(lparam);
                bool isResize = !(winPos->flags & SWP_NOSIZE);
                if (has_custom_frame && isResize) {
                    adjustMaximizedSize(window, winPos);
                    adjustPositionOnRestoreByMove(window, winPos);
                }

                if (false == window_can_be_shown) {
                    break;
                }

                if (bypass_wm_size == TRUE) {
                    if (isResize && (!during_minimize) && (winPos->cx != 0)) {
                        adjustChildWindowSize();
                    }
                }
                break;
            }
            case WM_GETMINMAXINFO: {
                auto info = reinterpret_cast<MINMAXINFO*>(lparam);
                if ((min_size.cx != 0) && (min_size.cy != 0)) {
                    SIZE minSize = min_size;
                    getSizeOnScreen(&minSize);
                    info->ptMinTrackSize.x = minSize.cx;
                    info->ptMinTrackSize.y = minSize.cy;
                }
                if ((max_size.cx != 0) && (max_size.cy != 0)) {
                    SIZE maxSize = max_size;
                    getSizeOnScreen(&maxSize);
                    info->ptMaxTrackSize.x = maxSize.cx;
                    info->ptMaxTrackSize.y = maxSize.cy;
                }
                return 0;
            }
            case WM_ENTERSIZEMOVE: {
                bool isMaximized = IsMaximized(window);
                during_size_move = TRUE;
                if (isMaximized) {
                    restore_by_moving = TRUE;
                }
                break;
            }
            case WM_EXITSIZEMOVE: {
                during_size_move = FALSE;
                if (dpi_changed_during_size_move) {
                    forceChildRefresh();
                }
                dpi_changed_during_size_move = FALSE;
                restore_by_moving = FALSE;
                break;
            }
            case WM_BDW_ACTION: {
                handle_bdw_action(window, wparam, lparam);
                break;
            }
            default:
                break;
        }
        return DefSubclassProc(window, message, wparam, lparam);
    }

    auto dragAppWindow() -> bool {
        if (flutter_window == nullptr) {
            return false;
        }
        ReleaseCapture();
        SendMessage(flutter_window, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
        return true;
    }
} // bitsdojo_window namespace

auto bitsdojo_window_configure(unsigned int flags) -> int {
    return bitsdojo_window::configure(flags);
}
