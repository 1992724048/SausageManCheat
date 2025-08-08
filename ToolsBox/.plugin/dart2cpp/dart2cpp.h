#pragma once
#include "FlutterPluginRegistry.h"
#include "flutter/ephemeral/method_channel.h"
#include "flutter/ephemeral/plugin_registrar.h"
#include "flutter/ephemeral/standard_method_codec.h"


class ConfigData final : public flutter::Plugin, FlutterPlugin<ConfigData> {
public:
    static constexpr auto PluginName = "dart2cpp";
    static auto RegisterWithRegistrar(flutter::PluginRegistrarWindows* _registrar) -> void;

    static auto instance() -> ConfigData& {
        static ConfigData config;
        return config;
    }

    ConfigData();
    ConfigData(flutter::PluginRegistrarWindows* _registrar);

    ~ConfigData() override;
private:
    std::unique_ptr<flutter::MethodChannel<>> channel;

    static auto handle_method_call(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>> _result) -> void;
};
