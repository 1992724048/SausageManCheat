#pragma once

#include <filesystem>
#include <memory>

#include "Field.h"
#include <vector>
#include <string>
#include "fields/Enum.h"

#include "../events/event.hpp"

#define SNFEX(field, friendName, name, section, defaultValue, shared) config::create_field<decltype(field)::_ValueType>(friendName, name, section, shared, defaultValue)
#define SNFB(field, name, section, defaultValue, shared) SNFEX(field, name, config::internal::fix_field_name(#field), section, defaultValue, shared)
#define SNF(field, name, section, defaultValue) SNFB(field, name, section, defaultValue, false)

#define NFEX(field, friendName, name, section, defaultValue, shared) field(SNFEX(field, friendName, name, section, defaultValue, shared))
#define NFEXUP(field, friendName, name, section, shared, ...) field(config::create_field<decltype(field)::_ValueType>(friendName, name, section, shared, __VA_ARGS__))

#define NFB(field, name, section, defaultValue, shared) NFEX(field, name, config::internal::fix_field_name(#field), section, defaultValue, shared)
#define NFS(field, name, section, defaultValue) NFB(field, name, section, defaultValue, true)
#define NF(field, name, section, defaultValue) NFB(field, name, section, defaultValue, false)

#define NFPB(field, name, section, shared, ...) NFEXUP(field, name, config::internal::fix_field_name(#field), section, shared, __VA_ARGS__)
#define NFPS(field, name, section, ...) NFPB(field, name, section, true, __VA_ARGS__)
#define NFP(field, name, section, ...) NFPB(field, name, section, false, __VA_ARGS__)

namespace config {
    namespace internal {
        template<typename T>
        inline static std::vector<T> s_fields;

        auto add_field(const std::shared_ptr<FieldEntry>& _field) -> void;

        inline auto fix_field_name(const std::string& _field_name) -> std::string {
            if (_field_name.substr(1, 1) == "_") {
                return _field_name.substr(2);
            }
            return _field_name;
        }
    }

    template<typename T, typename... Args>
    auto create_field(const std::string& _friend_name, const std::string& _name, const std::string& _section, bool _multi_profile, Args... _args) -> Field<T> {
        auto new_field = Field<T>(_friend_name, _name, _section, T(_args...), _multi_profile);
        internal::s_fields<Field<T>>.push_back(new_field);
        internal::add_field(new_field.entry());
        return new_field;
    }

    template<typename T>
    auto get_fields() -> std::vector<Field<T>>& {
        return internal::s_fields<Field<T>>;
    }

    auto initialize(const std::filesystem::path& _file_path) -> void;
    auto setup_update(TEvent<>*) -> void;

    auto refresh() -> void;
    auto save() -> void;

    auto create_profile(const std::string& _profile_name, bool _move_after_create = true) -> void;
    auto remove_profile(const std::string& _profile_name) -> void;
    auto rename_profile(const std::string& _old_profile_name, const std::string& _new_profile_name) -> void;
    auto change_profile(const std::string& _profile_name) -> void;
    auto duplicate_profile(const std::string& _profile_name) -> void;
    auto get_profiles() -> const std::vector<std::string>&;
    auto current_profile_name() -> const std::string&;

    extern TEvent<> profile_changed;
}
