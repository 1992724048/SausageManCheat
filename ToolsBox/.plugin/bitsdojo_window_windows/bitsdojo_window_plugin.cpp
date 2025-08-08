#include "bitsdojo_window_plugin.h"

constexpr char kChannelName[] = "bitsdojo/window";
const auto bdwAPI = bitsdojo_window_api();

std::unique_ptr<flutter::MethodChannel<>> bitsdojo_window_channel;

BitsdojoWindowPlugin::BitsdojoWindowPlugin() : registrar_{nullptr} {}

// static
auto BitsdojoWindowPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar) -> void {
    auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(registrar->messenger(), kChannelName, &flutter::StandardMethodCodec::GetInstance());

    auto* channel_pointer = channel.get();

    auto plugin = std::make_unique<BitsdojoWindowPlugin>(registrar, std::move(channel));

    channel_pointer->SetMethodCallHandler([plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
    });

    registrar->AddPlugin(std::move(plugin));
}

BitsdojoWindowPlugin::BitsdojoWindowPlugin(flutter::PluginRegistrarWindows* registrar, std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel) : registrar_(registrar),
    channel_(std::move(channel)) {}

BitsdojoWindowPlugin::~BitsdojoWindowPlugin() {}

auto BitsdojoWindowPlugin::HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue>& method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) -> void {
    if (method_call.method_name().compare("dragAppWindow") == 0) {
        const bool call_result = bdwAPI->privateAPI->dragAppWindow();
        if (call_result) {
            result->Success();
        } else {
            result->Error("ERROR_DRAG_APP_WINDOW_FAILED", "Could not drag app window");
        }
    } else {
        result->NotImplemented();
    }
}
