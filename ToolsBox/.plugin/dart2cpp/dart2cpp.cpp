#include <fetures/FeatureRegistrar.hpp>
#include "dart2cpp.h"
auto ConfigData::RegisterWithRegistrar(flutter::PluginRegistrarWindows* _registrar) -> void {
    auto plugin = std::make_unique<ConfigData>(_registrar);
    _registrar->AddPlugin(std::move(plugin));
}

ConfigData::ConfigData() = default;

ConfigData::ConfigData(flutter::PluginRegistrarWindows* _registrar) {
    channel = std::make_unique<flutter::MethodChannel<>>(_registrar->messenger(), PluginName, &flutter::StandardMethodCodec::GetInstance());
    channel->SetMethodCallHandler(handle_method_call);
}

ConfigData::~ConfigData() = default;

auto ConfigData::handle_method_call(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>> _result) -> void {
    const std::string& method_name = _method_call.method_name();
    const auto method = FeatureBase::get_method(method_name);
    if (!method) {
        _result->NotImplemented();
    }
    method.value()(_method_call, _result);
}
