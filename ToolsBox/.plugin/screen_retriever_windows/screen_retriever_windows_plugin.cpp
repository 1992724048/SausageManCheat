#include "screen_retriever_windows_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <flutter/ephemeral/method_channel.h>
#include <flutter/ephemeral/plugin_registrar_windows.h>
#include <flutter/ephemeral/standard_method_codec.h>

#include <codecvt>
#include <map>
#include <memory>
#include <sstream>

constexpr double kBaseDpi = 96.0;

namespace screen_retriever_windows {
    std::unique_ptr<flutter::EventSink<>> event_sink_;

    // static
    auto ScreenRetrieverWindowsPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar) -> void {
        const auto channel = std::make_unique<flutter::MethodChannel<>>(registrar->messenger(), "dev.leanflutter.plugins/screen_retriever", &flutter::StandardMethodCodec::GetInstance());

        auto plugin = std::make_unique<ScreenRetrieverWindowsPlugin>(registrar);

        channel->SetMethodCallHandler([plugin_pointer = plugin.get()](const auto& call, auto result) {
            plugin_pointer->HandleMethodCall(call, std::move(result));
        });

        const auto event_channel = std::make_unique<flutter::EventChannel<>>(registrar->messenger(), "dev.leanflutter.plugins/screen_retriever_event", &flutter::StandardMethodCodec::GetInstance());
        auto streamHandler = std::make_unique<flutter::StreamHandlerFunctions<>>(
                [plugin_pointer = plugin.get()](const flutter::EncodableValue* arguments, std::unique_ptr<flutter::EventSink<>>&& events) -> std::unique_ptr<flutter::StreamHandlerError<>> {
                    return plugin_pointer->OnListen(arguments, std::move(events));
                },
                [plugin_pointer = plugin.get()](const flutter::EncodableValue* arguments) -> std::unique_ptr<flutter::StreamHandlerError<>> {
                    return plugin_pointer->OnCancel(arguments);
                });
        event_channel->SetStreamHandler(std::move(streamHandler));

        registrar->AddPlugin(std::move(plugin));
    }

    ScreenRetrieverWindowsPlugin::ScreenRetrieverWindowsPlugin(flutter::PluginRegistrarWindows* registrar) {
        registrar_ = registrar;
        display_count_ = GetMonitorCount();
        window_proc_id_ = registrar->RegisterTopLevelWindowProcDelegate([this](const HWND hwnd, const UINT message, const WPARAM wparam, const LPARAM lparam) {
            return HandleWindowProc(hwnd, message, wparam, lparam);
        });
    }

    ScreenRetrieverWindowsPlugin::~ScreenRetrieverWindowsPlugin() {}

    auto ScreenRetrieverWindowsPlugin::GetMonitorCount() -> int {
        int monitorCount = 0;
        EnumDisplayMonitors(nullptr,
                            nullptr,
                            [](HMONITOR, HDC, LPRECT, const LPARAM lParam) -> BOOL {
                                const auto count = (int*)lParam;
                                (*count)++;
                                return TRUE;
                            },
                            (LPARAM)&monitorCount);
        return monitorCount;
    }

    auto ScreenRetrieverWindowsPlugin::HandleWindowProc(HWND hwnd, const UINT message, WPARAM wparam, LPARAM lparam) -> std::optional<LRESULT> {
        switch (message) {
            case WM_DISPLAYCHANGE: {
                const int currentMonitorCount = GetMonitorCount();
                auto args = flutter::EncodableMap();
                if (currentMonitorCount > display_count_) {
                    args[flutter::EncodableValue("type")] = "display-added";
                } else if (currentMonitorCount < display_count_) {
                    args[flutter::EncodableValue("type")] = "display-removed";
                }
                display_count_ = currentMonitorCount;
                if (event_sink_) {
                    event_sink_->Success(flutter::EncodableValue(args));
                }
                break;
            }
        }
        return std::nullopt;
    }

    auto MonitorToEncodableMap(const HMONITOR monitor) -> flutter::EncodableMap {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

        MONITORINFOEX info;
        info.cbSize = sizeof(MONITORINFOEX);
        ::GetMonitorInfo(monitor, &info);
        const UINT dpi = FlutterDesktopGetDpiForMonitor(monitor);

        wchar_t display_name[std::size(info.szDevice) + 1] = {};
        memcpy(display_name, info.szDevice, sizeof(info.szDevice));

        double scale_factor = dpi / kBaseDpi;

        double visibleWidth = round((info.rcWork.right - info.rcWork.left) / scale_factor);
        double visibleHeight = round((info.rcWork.bottom - info.rcWork.top) / scale_factor);

        double visibleX = round((info.rcWork.left) / scale_factor);
        double visibleY = round((info.rcWork.top) / scale_factor);

        auto size = flutter::EncodableMap();
        auto visibleSize = flutter::EncodableMap();
        auto visiblePosition = flutter::EncodableMap();

        size[flutter::EncodableValue("width")] = flutter::EncodableValue(round(info.rcMonitor.right / scale_factor - visibleX));
        size[flutter::EncodableValue("height")] = flutter::EncodableValue(round(info.rcMonitor.bottom / scale_factor - visibleY));

        visibleSize[flutter::EncodableValue("width")] = flutter::EncodableValue(visibleWidth);
        visibleSize[flutter::EncodableValue("height")] = flutter::EncodableValue(visibleHeight);

        visiblePosition[flutter::EncodableValue("dx")] = flutter::EncodableValue(visibleX);
        visiblePosition[flutter::EncodableValue("dy")] = flutter::EncodableValue(visibleY);

        auto display = flutter::EncodableMap();

        display[flutter::EncodableValue("id")] = flutter::EncodableValue("");
        DISPLAY_DEVICE displayDevice;
        displayDevice.cb = sizeof(DISPLAY_DEVICE);
        int deviceIndex = 0;
        while (EnumDisplayDevices(info.szDevice, deviceIndex, &displayDevice, 0)) {
            if (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE && (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
                std::wstring deviceName(displayDevice.DeviceName);
                if (deviceName.starts_with(info.szDevice)) {
                    display[flutter::EncodableValue("id")] = flutter::EncodableValue(converter.to_bytes(displayDevice.DeviceID).c_str());
                }
            }
            deviceIndex++;
        }

        display[flutter::EncodableValue("name")] = flutter::EncodableValue(converter.to_bytes(display_name).c_str());
        display[flutter::EncodableValue("size")] = flutter::EncodableValue(size);
        display[flutter::EncodableValue("visibleSize")] = flutter::EncodableValue(visibleSize);
        display[flutter::EncodableValue("visiblePosition")] = flutter::EncodableValue(visiblePosition);
        display[flutter::EncodableValue("scaleFactor")] = flutter::EncodableValue(scale_factor);

        return display;
    }

    static auto CALLBACK MonitorRepresentationEnumProc(const HMONITOR monitor, HDC hdc, LPRECT clip, const LPARAM list_ref) -> BOOL {
        const auto monitors = reinterpret_cast<flutter::EncodableValue*>(list_ref);
        flutter::EncodableMap display = MonitorToEncodableMap(monitor);
        monitors->get<flutter::EncodableList>().emplace_back(display);
        return TRUE;
    }

    auto ScreenRetrieverWindowsPlugin::GetCursorScreenPoint(const flutter::MethodCall<>& method_call, std::unique_ptr<flutter::MethodResult<>> result) -> void {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();

        const double device_pixel_ratio = args.at(flutter::EncodableValue("devicePixelRatio")).get<double>();

        POINT cursorPos;
        GetCursorPos(&cursorPos);
        double x = cursorPos.x / device_pixel_ratio * 1.0f;
        double y = cursorPos.y / device_pixel_ratio * 1.0f;

        auto result_data = flutter::EncodableMap();
        result_data[flutter::EncodableValue("dx")] = flutter::EncodableValue(x);
        result_data[flutter::EncodableValue("dy")] = flutter::EncodableValue(y);

        result->Success(flutter::EncodableValue(result_data));
    }

    auto ScreenRetrieverWindowsPlugin::GetPrimaryDisplay(const flutter::MethodCall<>& method_call, std::unique_ptr<flutter::MethodResult<>> result) -> void {
        constexpr POINT pt_zero = {0, 0};
        const HMONITOR monitor = MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
        flutter::EncodableMap display = MonitorToEncodableMap(monitor);
        result->Success(flutter::EncodableValue(display));
    }

    auto ScreenRetrieverWindowsPlugin::GetAllDisplays(const flutter::MethodCall<>& method_call, std::unique_ptr<flutter::MethodResult<>> result) -> void {
        flutter::EncodableValue displays(std::in_place_type<flutter::EncodableList>);

        EnumDisplayMonitors(nullptr, nullptr, MonitorRepresentationEnumProc, reinterpret_cast<LPARAM>(&displays));

        auto result_data = flutter::EncodableMap();
        result_data[flutter::EncodableValue("displays")] = displays;

        result->Success(flutter::EncodableValue(result_data));
    }

    auto ScreenRetrieverWindowsPlugin::HandleMethodCall(const flutter::MethodCall<>& method_call, std::unique_ptr<flutter::MethodResult<>> _result) -> void {
        if (method_call.method_name() == "getCursorScreenPoint") {
            GetCursorScreenPoint(method_call, std::move(_result));
        } else if (method_call.method_name() == "getPrimaryDisplay") {
            GetPrimaryDisplay(method_call, std::move(_result));
        } else if (method_call.method_name() == "getAllDisplays") {
            GetAllDisplays(method_call, std::move(_result));
        } else {
            _result->NotImplemented();
        }
    }

    auto ScreenRetrieverWindowsPlugin::OnListenInternal(const flutter::EncodableValue* arguments, std::unique_ptr<flutter::EventSink<>>&& events) -> std::unique_ptr<flutter::StreamHandlerError<>> {
        event_sink_ = std::move(events);
        return nullptr;
    }

    auto ScreenRetrieverWindowsPlugin::OnCancelInternal(const flutter::EncodableValue* arguments) -> std::unique_ptr<flutter::StreamHandlerError<>> {
        event_sink_ = nullptr;
        return nullptr;
    }
} // namespace screen_retriever_windows
