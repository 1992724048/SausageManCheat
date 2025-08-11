#include "AimBot.h"
#include "ToolsBox.h"

AimBot::AimBot() :
NFS(f_enable, "", "AimBot", true),
NFS(f_random, "", "AimBot", false),
NFS(f_speed, "", "AimBot", 1.0),
NFS(f_rect_w, "", "AimBot", 800),
NFS(f_rect_h, "", "AimBot", 600),
NFS(f_hotkey, "", "AimBot", VK_LSHIFT)
{}

auto AimBot::aim_get(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
    static util::Map<std::string, std::function<void()>> functions;
    static std::once_flag once;

    const flutter::EncodableMap& args = _method_call.arguments()->get<flutter::EncodableMap>();
    auto& field_name = args.at(flutter::EncodableValue("field_name")).get<std::string>();
    const auto that = instance();

    std::call_once(once,
                   [&] {
                       functions["f_enable"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_enable));
                       };

                       functions["f_speed"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_speed));
                           };

                       functions["f_rect_w"] = [&] {
                            _result->Success(flutter::EncodableValue(that->f_rect_w));
                           };

                       functions["f_rect_h"] = [&] {
                            _result->Success(flutter::EncodableValue(that->f_rect_h));
                           };

                       functions["f_hotkey"] = [&] {
                            _result->Success(flutter::EncodableValue(that->f_hotkey));
                           };

                       functions["f_random"] = [&] {
                            _result->Success(flutter::EncodableValue(that->f_random));
                           };
                   });

    if (functions.contains(field_name)) {
        return functions[field_name]();
    }
    _result->NotImplemented();
}

auto AimBot::aim_set(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
    const flutter::EncodableMap& args = _method_call.arguments()->get<flutter::EncodableMap>();
    auto& field_name = args.at(flutter::EncodableValue("field_name")).get<std::string>();
    const auto that = instance();

    if (field_name == "f_enable") {
        that->f_enable = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_speed") {
        that->f_speed = args.at(flutter::EncodableValue("value")).get<double>();
        goto _return;
    }

    if (field_name == "f_rect_w") {
        that->f_rect_w = args.at(flutter::EncodableValue("value")).get<int32_t>();
        goto _return;
    }

    if (field_name == "f_rect_h") {
        that->f_rect_h = args.at(flutter::EncodableValue("value")).get<int32_t>();
        goto _return;
    }

    if (field_name == "f_hotkey") {
        that->f_hotkey = args.at(flutter::EncodableValue("value")).get<int>();
        goto _return;
    }

    if (field_name == "f_random") {
        that->f_random = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    _return:
    _result->Success(flutter::EncodableValue(true));
    App::instance().commit_config();
}
