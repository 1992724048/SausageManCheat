#ifndef FLUTTER_PLUGIN_SCREEN_RETRIEVER_WINDOWS_PLUGIN_H_
#define FLUTTER_PLUGIN_SCREEN_RETRIEVER_WINDOWS_PLUGIN_H_

#include <flutter/ephemeral/event_channel.h>
#include <flutter/ephemeral/event_stream_handler_functions.h>
#include <flutter/ephemeral/method_channel.h>
#include <flutter/ephemeral/plugin_registrar_windows.h>

#include <memory>

#include "FlutterPluginRegistry.h"

namespace screen_retriever_windows {
    class ScreenRetrieverWindowsPlugin : public flutter::Plugin, flutter::StreamHandler<>, FlutterPlugin<ScreenRetrieverWindowsPlugin> {
        flutter::PluginRegistrarWindows* registrar_;
        std::unique_ptr<flutter::EventSink<>> event_sink_;

        int32_t display_count_ = 0;
        int32_t window_proc_id_ = -1;
        static auto GetMonitorCount() -> int;
        auto HandleWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> std::optional<LRESULT>;

    public:
        static constexpr auto PluginName = "ScreenRetrieverWindowsPluginCApi";
        static auto RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar) -> void;

        ScreenRetrieverWindowsPlugin(): registrar_{nullptr} {} ;
        ScreenRetrieverWindowsPlugin(flutter::PluginRegistrarWindows* registrar);
        ~ScreenRetrieverWindowsPlugin() override;

        // Disallow copy and assign.
        ScreenRetrieverWindowsPlugin(const ScreenRetrieverWindowsPlugin&) = delete;
        auto operator=(const ScreenRetrieverWindowsPlugin&) -> ScreenRetrieverWindowsPlugin& = delete;

        static auto GetCursorScreenPoint(const flutter::MethodCall<>& method_call, std::unique_ptr<flutter::MethodResult<>> result) -> void;
        static auto GetPrimaryDisplay(const flutter::MethodCall<>& method_call, std::unique_ptr<flutter::MethodResult<>> result) -> void;
        static auto GetAllDisplays(const flutter::MethodCall<>& method_call, std::unique_ptr<flutter::MethodResult<>> result) -> void;

        // Called when a method is called on this plugin's channel from Dart.
        static auto HandleMethodCall(const flutter::MethodCall<>& method_call, std::unique_ptr<flutter::MethodResult<>> _result) -> void;

        auto OnListenInternal(const flutter::EncodableValue* arguments, std::unique_ptr<flutter::EventSink<>>&& events) -> std::unique_ptr<flutter::StreamHandlerError<>> override;

        auto OnCancelInternal(const flutter::EncodableValue* arguments) -> std::unique_ptr<flutter::StreamHandlerError<>> override;
    };
} // namespace screen_retriever_windows

#endif  // FLUTTER_PLUGIN_SCREEN_RETRIEVER_WINDOWS_PLUGIN_H_
