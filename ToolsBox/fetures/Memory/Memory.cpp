#include "Memory.h"
#include "ToolsBox.h"

Memory::Memory() :
NFS(f_bullet_tracking, "", "Memory", false),
NFS(f_all_hit_head, "", "Memory", false),
NFS(f_lock_role, "", "Memory", false),
NFS(f_all_gun_auto, "", "Memory", false),
NFS(f_ballistics_tracking, "", "Memory", false),
NFS(f_bullet_no_gravity, "", "Memory", false),
NFS(f_damage_multi, "", "Memory", false),
NFS(f_damage_multi_value, "", "Memory", 1) {}

auto Memory::mem_get(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
    static util::Map<std::string, std::function<void()>> functions;
    static std::once_flag once;

    const flutter::EncodableMap& args = _method_call.arguments()->get<flutter::EncodableMap>();
    auto& field_name = args.at(flutter::EncodableValue("field_name")).get<std::string>();
    const auto that = instance();

    std::call_once(once,
        [&] {
            functions["f_bullet_tracking"] = [&] {
                _result->Success(flutter::EncodableValue(that->f_bullet_tracking));
                };

            functions["f_all_hit_head"] = [&] {
                _result->Success(flutter::EncodableValue(that->f_all_hit_head));
                };

            functions["f_lock_role"] = [&] {
                _result->Success(flutter::EncodableValue(that->f_lock_role));
                };

            functions["f_all_gun_auto"] = [&] {
                _result->Success(flutter::EncodableValue(that->f_all_gun_auto));
                };

            functions["f_damage_multi"] = [&] {
                _result->Success(flutter::EncodableValue(that->f_damage_multi));
                };

            functions["f_damage_multi_value"] = [&] {
                _result->Success(flutter::EncodableValue(that->f_damage_multi_value));
                };

            functions["f_ballistics_tracking"] = [&] {
                _result->Success(flutter::EncodableValue(that->f_ballistics_tracking));
                };

            functions["f_bullet_no_gravity"] = [&] {
                _result->Success(flutter::EncodableValue(that->f_bullet_no_gravity));
                };
        });

    if (functions.contains(field_name)) {
        return functions[field_name]();
    }
    _result->NotImplemented();
}

auto Memory::mem_set(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
    const flutter::EncodableMap& args = _method_call.arguments()->get<flutter::EncodableMap>();
    auto& field_name = args.at(flutter::EncodableValue("field_name")).get<std::string>();
    const auto that = instance();

    if (field_name == "f_bullet_tracking") {
        that->f_bullet_tracking = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_all_hit_head") {
        that->f_all_hit_head = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_lock_role") {
        that->f_lock_role = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_all_gun_auto") {
        that->f_all_gun_auto = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_damage_multi") {
        that->f_damage_multi = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_damage_multi_value") {
        that->f_damage_multi_value = args.at(flutter::EncodableValue("value")).get<int>();
        goto _return;
    }

    if (field_name == "f_ballistics_tracking") {
        that->f_ballistics_tracking = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_bullet_no_gravity") {
        that->f_bullet_no_gravity = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

_return:
    _result->Success(flutter::EncodableValue(true));
    App::instance().commit_config();
}
