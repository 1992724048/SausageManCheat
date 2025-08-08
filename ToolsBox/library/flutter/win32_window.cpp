#include "win32_window.h"

#include <dwmapi.h>
#include "flutter_window.h"

#include "resource.h"

namespace {
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

    // The number of Win32Window objects that currently exist.
    int g_active_window_count = 0;

    using EnableNonClientDpiScaling = BOOL __stdcall(HWND hwnd);

    // Scale helper to convert logical scaler values to physical using passed in
    // scale factor
    auto Scale(const int source, const double scale_factor) -> int {
        return static_cast<int>(source * scale_factor);
    }

    // Dynamically loads the |EnableNonClientDpiScaling| from the User32 module.
    // This API is only needed for PerMonitor V1 awareness mode.
    auto EnableFullDpiSupportIfAvailable(const HWND hwnd) -> void {
        const HMODULE user32_module = LoadLibraryA("User32.dll");
        if (!user32_module) {
            return;
        }
        const auto enable_non_client_dpi_scaling = reinterpret_cast<EnableNonClientDpiScaling*>(GetProcAddress(user32_module, "EnableNonClientDpiScaling"));
        if (enable_non_client_dpi_scaling != nullptr) {
            enable_non_client_dpi_scaling(hwnd);
        }
        FreeLibrary(user32_module);
    }
} // namespace

// Manages the Win32Window's window class registration.
class WindowClassRegistrar {
public:
    ~WindowClassRegistrar() = default;

    // Returns the singleton registrar instance.
    static auto GetInstance() -> WindowClassRegistrar* {
        if (!instance_) {
            instance_ = new WindowClassRegistrar();
        }
        return instance_;
    }

    // Returns the name of the window class, registering the class if it hasn't
    // previously been registered.
    auto GetWindowClass() -> const wchar_t*;

    // Unregisters the window class. Should only be called if there are no
    // instances of the window.
    auto UnregisterWindowClass() -> void;

private:
    WindowClassRegistrar() = default;

    static WindowClassRegistrar* instance_;

    bool class_registered_ = false;
};

WindowClassRegistrar* WindowClassRegistrar::instance_ = nullptr;

auto WindowClassRegistrar::GetWindowClass() -> const wchar_t* {
    if (!class_registered_) {
        WNDCLASS window_class;
        window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
        window_class.lpszClassName = kWindowClassName;
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.cbClsExtra = 0;
        window_class.cbWndExtra = 0;
        window_class.hInstance = GetModuleHandle(nullptr);
        window_class.hIcon = LoadIcon(window_class.hInstance, MAKEINTRESOURCE(IDI_ICON1));
        window_class.hbrBackground = nullptr;
        window_class.lpszMenuName = nullptr;
        window_class.lpfnWndProc = Win32Window::WndProc;
        RegisterClass(&window_class);
        class_registered_ = true;
    }
    return kWindowClassName;
}

auto WindowClassRegistrar::UnregisterWindowClass() -> void {
    UnregisterClass(kWindowClassName, nullptr);
    class_registered_ = false;
}

Win32Window::Win32Window() {
    ++g_active_window_count;
}

Win32Window::~Win32Window() {
    --g_active_window_count;
    Destroy();
}

auto Win32Window::Create(const std::wstring& _title, const Point& _origin, const Size& _size) -> bool {
    Destroy();

    const wchar_t* window_class = WindowClassRegistrar::GetInstance()->GetWindowClass();

    const POINT target_point = {static_cast<LONG>(_origin.x), static_cast<LONG>(_origin.y)};
    HMONITOR monitor = MonitorFromPoint(target_point, MONITOR_DEFAULTTONEAREST);
    UINT dpi = FlutterDesktopGetDpiForMonitor(monitor);
    double scale_factor = dpi / 96.0;

    HWND window = CreateWindow(window_class,
                               _title.c_str(),
                               WS_OVERLAPPEDWINDOW,
                               Scale(_origin.x, scale_factor),
                               Scale(_origin.y, scale_factor),
                               Scale(_size.width, scale_factor),
                               Scale(_size.height, scale_factor),
                               nullptr,
                               nullptr,
                               GetModuleHandle(nullptr),
                               this);

    if (!window) {
        return false;
    }

    UpdateTheme(window);

    return OnCreate();
}

auto Win32Window::Show() const -> bool {
    return ShowWindow(window_handle_, SW_SHOWNORMAL);
}

auto Win32Window::Show(bool show) const -> bool {
    return ShowWindow(window_handle_, show ? SW_SHOW : SW_HIDE);
}

// static
auto CALLBACK Win32Window::WndProc(const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam) noexcept -> LRESULT {
    if (message == WM_NCCREATE) {
        const auto window_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window_struct->lpCreateParams));

        const auto that = static_cast<Win32Window*>(window_struct->lpCreateParams);
        EnableFullDpiSupportIfAvailable(window);
        that->window_handle_ = window;
    } else if (Win32Window* that = GetThisFromHandle(window)) {
        return that->MessageHandler(window, message, wparam, lparam);
    }

    return DefWindowProc(window, message, wparam, lparam);
}

auto Win32Window::MessageHandler(const HWND hwnd, const UINT message, const WPARAM wparam, const LPARAM lparam) noexcept -> LRESULT {
    switch (message) {
        case WM_DESTROY:
            window_handle_ = nullptr;
            Destroy();
            if (quit_on_close_) {
                PostQuitMessage(0);
            }
            return 0;

        case WM_DPICHANGED: {
            const auto new_rect_size = reinterpret_cast<RECT*>(lparam);
            const LONG new_width = new_rect_size->right - new_rect_size->left;
            const LONG new_height = new_rect_size->bottom - new_rect_size->top;

            SetWindowPos(hwnd, nullptr, new_rect_size->left, new_rect_size->top, new_width, new_height, SWP_NOZORDER | SWP_NOACTIVATE);

            return 0;
        }
        case WM_SIZE: {
            const auto [left, top, right, bottom] = GetClientArea();
            if (child_content_ != nullptr) {
                MoveWindow(child_content_, left, top, right - left, bottom - top, TRUE);
            }
            return 0;
        }

        case WM_ACTIVATE:
            if (child_content_ != nullptr) {
                SetFocus(child_content_);
            }
            return 0;

        case WM_DWMCOLORIZATIONCOLORCHANGED:
            UpdateTheme(hwnd);
            return 0;
        case WM_SYSCOMMAND:
            if ((wparam & 0xFFF0) == SC_CLOSE) {
                return 0;
            }
            break;
        default: ;
    }

    return DefWindowProc(window_handle_, message, wparam, lparam);
}

auto Win32Window::Destroy() -> void {
    OnDestroy();

    if (window_handle_) {
        DestroyWindow(window_handle_);
        window_handle_ = nullptr;
    }
    if (g_active_window_count == 0) {
        WindowClassRegistrar::GetInstance()->UnregisterWindowClass();
    }
}

auto Win32Window::GetThisFromHandle(const HWND window) noexcept -> Win32Window* {
    return reinterpret_cast<Win32Window*>(GetWindowLongPtr(window, GWLP_USERDATA));
}

auto Win32Window::SetChildContent(const HWND content) -> void {
    child_content_ = content;
    SetParent(content, window_handle_);
    const RECT frame = GetClientArea();

    MoveWindow(content, frame.left, frame.top, frame.right - frame.left, frame.bottom - frame.top, true);

    SetFocus(child_content_);
}

auto Win32Window::GetClientArea() const -> RECT {
    RECT frame;
    GetClientRect(window_handle_, &frame);
    return frame;
}

auto Win32Window::GetHandle() const -> HWND {
    return window_handle_;
}

auto Win32Window::SetQuitOnClose(const bool quit_on_close) -> void {
    quit_on_close_ = quit_on_close;
}

auto Win32Window::OnCreate() -> bool {
    // No-op; provided for subclasses.
    return true;
}

auto Win32Window::OnDestroy() -> void {
    // No-op; provided for subclasses.
}

auto Win32Window::UpdateTheme(const HWND window) -> void {
    DWORD light_mode;
    DWORD light_mode_size = sizeof(light_mode);
    const LSTATUS result = RegGetValue(HKEY_CURRENT_USER, kGetPreferredBrightnessRegKey, kGetPreferredBrightnessRegValue, RRF_RT_REG_DWORD, nullptr, &light_mode, &light_mode_size);

    if (result == ERROR_SUCCESS) {
        const BOOL enable_dark_mode = light_mode == 0;
        DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable_dark_mode, sizeof(enable_dark_mode));
    }
}
