#pragma once

#include <windows.h>

#include <flutter/ephemeral/method_channel.h>
#include <flutter/ephemeral/plugin_registrar_windows.h>
#include <flutter/ephemeral/standard_method_codec.h>

#include <codecvt>
#include <map>
#include <memory>
#include <sstream>

#include "FlutterPluginRegistry.h"
#include "window_manager.cpp"

class WindowManagerPlugin : public flutter::Plugin, FlutterPlugin<WindowManagerPlugin> {
public:
    static constexpr auto PluginName = "WindowManagerPlugin";
    static auto RegisterWithRegistrar(flutter::PluginRegistrarWindows* _registrar) -> void;

    WindowManagerPlugin();
    WindowManagerPlugin(flutter::PluginRegistrarWindows* _registrar);

    ~WindowManagerPlugin() override;

    inline static WindowManager* window_manager;
private:
    std::unique_ptr<flutter::MethodChannel<>> channel = nullptr;
    flutter::PluginRegistrarWindows* registrar;

    // The ID of the WindowProc delegate registration.
    int window_proc_id = -1;

    auto _EmitEvent(std::string eventName) const -> void;
    // Called for top-level WindowProc delegation.
    auto HandleWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> std::optional<LRESULT>;
    // Called when a method is called on this plugin's channel from Dart.
    auto HandleMethodCall(const flutter::MethodCall<>& method_call, const std::unique_ptr<flutter::MethodResult<>>& _result) const -> void;

    static auto adjust_nccalcsize(HWND hwnd, NCCALCSIZE_PARAMS* sz) -> void {
        LONG l = 8;
        LONG t = 8;

        const HMONITOR monitor = MonitorFromRect(&sz->rgrc[0], MONITOR_DEFAULTTONEAREST);
        if (monitor != nullptr) {
            MONITORINFO monitorInfo;
            monitorInfo.cbSize = sizeof(MONITORINFO);
            if (TRUE == GetMonitorInfo(monitor, &monitorInfo)) {
                l = sz->rgrc[0].left - monitorInfo.rcWork.left;
                t = sz->rgrc[0].top - monitorInfo.rcWork.top;
            } else {
                // GetMonitorInfo failed, use (8, 8) as default value
            }
        } else {
            // unreachable code
        }

        sz->rgrc[0].left -= l;
        sz->rgrc[0].top -= t;
        sz->rgrc[0].right += l;
        sz->rgrc[0].bottom += t;
    }
};
