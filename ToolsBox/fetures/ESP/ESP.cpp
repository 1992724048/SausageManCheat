#include "ESP.h"
#include "ToolsBox.h"

ESP::ESP() :NFS(f_enable, "", "ESP", true),
            NFS(f_show_ai, "", "ESP", true),
            NFS(f_show_box, "", "ESP", true),
            NFS(f_show_role, "", "ESP", true),
            NFS(f_show_info, "", "ESP", true),
            NFS(f_show_bone, "", "ESP", true),
            NFS(f_show_team, "", "ESP", true),
            NFS(f_show_item, "", "ESP", true),
            NFS(f_show_cars, "", "ESP", true) {}

auto ESP::esp_get(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
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

                       functions["f_show_ai"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_show_ai));
                       };

                       functions["f_show_role"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_show_role));
                       };

                       functions["f_show_info"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_show_info));
                       };

                       functions["f_show_bone"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_show_bone));
                       };

                       functions["f_show_box"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_show_box));
                       };

                       functions["f_show_team"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_show_team));
                       };

                       functions["f_show_item"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_show_item));
                       };

                       functions["f_show_cars"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_show_cars));
                       };
                   });

    if (functions.contains(field_name)) {
        return functions[field_name]();
    }
    _result->NotImplemented();
}

auto ESP::esp_set(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
    const flutter::EncodableMap& args = _method_call.arguments()->get<flutter::EncodableMap>();
    auto& field_name = args.at(flutter::EncodableValue("field_name")).get<std::string>();
    const auto that = instance();

    if (field_name == "f_enable") {
        that->f_enable = args.at(flutter::EncodableValue("value")).get<bool>();
    }

    if (field_name == "f_show_ai") {
        that->f_show_ai = args.at(flutter::EncodableValue("value")).get<bool>();
    }

    if (field_name == "f_show_box") {
        that->f_show_box = args.at(flutter::EncodableValue("value")).get<bool>();
    }

    if (field_name == "f_show_role") {
        that->f_show_role = args.at(flutter::EncodableValue("value")).get<bool>();
    }

    if (field_name == "f_show_info") {
        that->f_show_info = args.at(flutter::EncodableValue("value")).get<bool>();
    }

    if (field_name == "f_show_bone") {
        that->f_show_bone = args.at(flutter::EncodableValue("value")).get<bool>();
    }

    if (field_name == "f_show_team") {
        that->f_show_team = args.at(flutter::EncodableValue("value")).get<bool>();
    }

    if (field_name == "f_show_item") {
        that->f_show_item = args.at(flutter::EncodableValue("value")).get<bool>();
    }

    if (field_name == "f_show_cars") {
        that->f_show_cars = args.at(flutter::EncodableValue("value")).get<bool>();
    }

_return:
    _result->Success(flutter::EncodableValue(true));
    App::instance().commit_config();
}
