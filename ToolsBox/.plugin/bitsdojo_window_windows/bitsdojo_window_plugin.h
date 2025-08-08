#ifndef FLUTTER_PLUGIN_BITSDOJO_WINDOW_PLUGIN_H_
#define FLUTTER_PLUGIN_BITSDOJO_WINDOW_PLUGIN_H_

// This must be included before many other Windows headers.
#include <windows.h>

#include <flutter/ephemeral/method_channel.h>
#include <flutter/ephemeral/plugin_registrar_windows.h>
#include <flutter/ephemeral/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

#include "./bitsdojo_window_api.h"
#include "FlutterPluginRegistry.h"
#include "flutter/ephemeral/flutter_plugin_registrar.h"

class BitsdojoWindowPlugin : public flutter::Plugin, protected FlutterPlugin<BitsdojoWindowPlugin> {
public:
    BitsdojoWindowPlugin();

    static constexpr const char* PluginName = "BitsdojoWindowPlugin";
    static auto RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar) -> void;

    BitsdojoWindowPlugin(flutter::PluginRegistrarWindows* registrar, std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel);
    ~BitsdojoWindowPlugin() override;

private:
    // Called when a method is called on this plugin's channel from Dart.
    static auto HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue>& method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) -> void;

    // The registrar for this plugin.
    flutter::PluginRegistrarWindows* registrar_;

    // The cannel to send menu item activations on.
    std::unique_ptr<flutter::MethodChannel<>> channel_;
};

#if defined(__cplusplus)
extern "C" {
#endif

#define BDW_CUSTOM_FRAME    0x1
#define BDW_HIDE_ON_STARTUP 0x2

    int bitsdojo_window_configure(unsigned int flags);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_PLUGIN_BITSDOJO_WINDOW_PLUGIN_H_
