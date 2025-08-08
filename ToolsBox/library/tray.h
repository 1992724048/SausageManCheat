#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <functional>
#include <stdexcept>
#include <unordered_map>

class Tray {
public:
    using Callback = std::function<void()>;

    static auto create_tray(const std::wstring& tip, const std::wstring& title, const std::wstring& info) -> void {
        notifyicon_data.cbSize = sizeof(notifyicon_data);
        notifyicon_data.hWnd = window;
        notifyicon_data.uID = 1;
        notifyicon_data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        notifyicon_data.uCallbackMessage = WM_USER + 1;
        notifyicon_data.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wcsncpy_s(notifyicon_data.szTip, tooltip.data(), _TRUNCATE);

        wcscpy_s(notifyicon_data.szInfo, info.data());
        wcscpy_s(notifyicon_data.szInfoTitle, title.data());
        notifyicon_data.dwInfoFlags = NIIF_INFO;
        notifyicon_data.uTimeout = 5000;

        wcscpy_s(notifyicon_data.szTip, tip.data());


        if (!Shell_NotifyIcon(NIM_ADD, &notifyicon_data)) {
            throw std::runtime_error("无法创建托盘图标!");
        }

        Shell_NotifyIcon(NIM_MODIFY, &notifyicon_data);
        notifyicon_data.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIcon(NIM_SETVERSION, &notifyicon_data);
    }

    static auto set_tray_icon(const HICON icon) -> void {
        notifyicon_data.hIcon = icon;
        Shell_NotifyIcon(NIM_MODIFY, &notifyicon_data);
        Shell_NotifyIcon(NIM_DELETE, &notifyicon_data);
        Shell_NotifyIcon(NIM_ADD, &notifyicon_data);
    }

    static auto set_tray_icon(const UINT resource_id) -> void {
        if (const HICON icon = LoadIcon(instance, MAKEINTRESOURCE(resource_id))) {
            set_tray_icon(icon);
        }
    }

    static auto add_menu_item(const std::wstring& name, const Callback& callback) -> void {
        const UINT menu_id = next_menu_id++;
        callbacks[menu_id] = { name, callback };
    }

    static auto show_tray_notification(const std::wstring& title,
        const std::wstring& message,
        const DWORD flags = NIIF_INFO,
        const UINT timeout = 5000) -> void {
        notifyicon_data.uFlags = NIF_INFO;
        wcscpy_s(notifyicon_data.szInfo, message.data());
        wcscpy_s(notifyicon_data.szInfoTitle, title.data());
        notifyicon_data.dwInfoFlags = flags;
        notifyicon_data.uTimeout = timeout;

        Shell_NotifyIcon(NIM_MODIFY, &notifyicon_data);
        notifyicon_data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    }

    static auto init(const std::wstring& title, const UINT resource_id) {
        instance = ::GetModuleHandle(nullptr);
        if (const HICON icon = LoadIcon(instance, MAKEINTRESOURCE(resource_id))) {
            return init(title, icon);
        }
    }

    static auto init(const std::wstring& title, const HICON icon) -> void {
        instance = ::GetModuleHandle(nullptr);

        wc = {
            .cbSize = sizeof(wc),
            .style = CS_CLASSDC,
            .lpfnWndProc = window_proc,
            .cbClsExtra = 0L,
            .cbWndExtra = 0L,
            .hInstance = instance,
            .hIcon = icon,
            .hCursor = nullptr,
            .hbrBackground = nullptr,
            .lpszMenuName = title.data(),
            .lpszClassName = L"tips",
            .hIconSm = icon
        };
        RegisterClassExW(&wc);
        window = ::CreateWindowW(wc.lpszClassName, title.data(), WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
    }

    static auto quit() -> void {
        PostQuitMessage(0);
    }

    static auto run(std::function<void()> callback) -> void {
        bool done = false;
        while (!done) {
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                callback();
                if (msg.message == WM_QUIT) {
                    done = true;
                    break;
                }
            }
      
            if (!IsWindowVisible(window)) {
                WaitMessage();
            }
        }

        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
    }
private:
    inline static HINSTANCE instance{ nullptr };
    inline static HWND window{ nullptr };
    inline static HMENU menu{ nullptr };
    inline static NOTIFYICONDATA notifyicon_data{};
    inline static std::unordered_map<UINT, std::pair<std::wstring, Callback>> callbacks;
    inline static UINT next_menu_id{ 0 };
    inline static std::wstring tooltip;

    inline static WNDCLASSEXW wc;


    static auto CALLBACK window_proc(const HWND wnd, const UINT message, const WPARAM w_param, const LPARAM param) -> LRESULT {
        if (message == WM_CREATE) {
            const auto p_create = reinterpret_cast<CREATESTRUCT*>(param);
            SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p_create->lpCreateParams));
        }

        return handle_message(wnd, message, w_param, param);
    }

    static auto handle_message(const HWND wnd, const UINT message, const WPARAM w_param, const LPARAM l_param) -> LRESULT {
        switch (message) {
        case WM_USER + 1:
            if (l_param == WM_RBUTTONUP) {
                show_context_menu();
            }
            return 0;
        case WM_COMMAND: {
            if (const UINT menu_id = LOWORD(w_param); callbacks.contains(menu_id)) {
                callbacks[menu_id].second();
            }
            break;
        }
        case WM_SYSCOMMAND:
            if ((w_param & 0xfff0) == SC_KEYMENU) {
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:;
        }
        return DefWindowProc(wnd, message, w_param, l_param);
    }

    static auto show_context_menu() -> void {
        POINT pt;
        GetCursorPos(&pt);

        menu = CreatePopupMenu();
        if (menu) {
            for (const auto& [fst, snd] : callbacks) {
                InsertMenu(menu, -1, MF_BYPOSITION | MF_STRING, fst, snd.first.data());
            }

            SetForegroundWindow(window);
            TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, window, nullptr);
            DestroyMenu(menu);
            menu = nullptr;
        }
    }
};