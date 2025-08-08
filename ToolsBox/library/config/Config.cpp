#include "config.h"

#include <atomic>
#include <fstream>

#include "../json.hpp"

#include <mutex>

#include "Logger.h"

namespace config {
    TEvent<> profile_changed;

    static std::filesystem::path s_filepath;
    static nlohmann::json s_config_root;
    static nlohmann::json s_empty_j_object = nlohmann::json::object();

    // Little speed-up
    static nlohmann::json* s_profile_root = nullptr;
    static nlohmann::json* s_profiles = nullptr;
    static nlohmann::json* s_shared_root = nullptr;

    static std::mutex s_profile_mutex;
    static std::string s_profile_name;
    static std::vector<std::string> s_profiles_names;

    static constexpr int c_save_delay = 1;
    static TEvent<>* s_update_event = nullptr;
    static std::atomic<int64_t> s_next_save_timestamp = 0;

    static std::vector<std::shared_ptr<internal::FieldEntry>> s_entries;

    static auto string_split(const std::string& _delimiter, const std::string& _content) -> std::vector<std::string> {
        std::vector<std::string> tokens;
        size_t pos;
        size_t prev_pos = 0;
        while ((pos = _content.find(_delimiter, prev_pos)) != std::string::npos) {
            std::string token = _content.substr(prev_pos, pos - prev_pos);
            tokens.push_back(token);
            prev_pos = pos + _delimiter.length();
        }
        tokens.push_back(_content.substr(prev_pos));
        return tokens;
    }

    static auto load_file() -> void {
        std::ifstream file_input(s_filepath, std::ios::in);
        if (!file_input.is_open()) {
            LOG_DEBUG << "打开配置文件失败，可能是第一次启动.";
            return;
        }

        try {
            s_config_root = nlohmann::json::parse(file_input);
        } catch (nlohmann::json::parse_error& ex) {
            LOG_ERROR << "按字节解析错误:" << ex.byte;
        }
    }

    static auto update_profiles_names() -> void {
        std::lock_guard lock(s_profile_mutex);
        s_profiles_names.clear();
        for (auto& [name, _] : s_profiles->items()) {
            s_profiles_names.push_back(name);
        }
    }

    auto initialize(const std::filesystem::path& _file_path) -> void {
        s_config_root = {};
        s_filepath = _file_path;
        load_file();

        if (!s_config_root.contains("current_profile")) {
            s_config_root = {{"shared", {}}, {"profiles", {}}, {"current_profile", ""}};
        }

        s_profiles = &s_config_root["profiles"];
        s_shared_root = &s_config_root["shared"];

        if (s_config_root["current_profile"].empty()) {
            create_profile("default");
        } else {
            change_profile(s_config_root["current_profile"]);
        }

        update_profiles_names();
    }

    static auto on_update() -> void;

    auto setup_update(TEvent<>* updateEvent) -> void {
        s_update_event = updateEvent;
        (*s_update_event) += FUNCTION_HANDLER(on_update);
    }

    static auto update_save_timestamp() -> void {
        if (!s_update_event) {
            return;
        }

        if (s_next_save_timestamp != 0) {
            return;
        }

        s_next_save_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + c_save_delay;
    }

    static auto reset_not_shared() -> void {
        for (const auto& entry : s_entries) {
            if (!entry->is_shared()) {
                entry->Reset();
            }
        }
    }

    static auto get_field_json_container(internal::FieldEntry* field, bool create = false) -> nlohmann::json& {
        if (field->get_container() != nullptr) {
            return *field->get_container();
        }

        nlohmann::json* rootContainer = s_profile_root;
        if (field->is_shared()) {
            rootContainer = s_shared_root;
        }

        const auto section_parts = string_split("::", field->get_section());
        for (auto& part : section_parts) {
            if (!rootContainer->contains(part)) {
                if (!create) {
                    return s_empty_j_object;
                }

                (*rootContainer)[part] = {};
            }

            rootContainer = &(*rootContainer)[part];
        }

        auto& section_container = *rootContainer;
        if (!section_container.contains(field->get_name())) {
            if (!create) {
                return s_empty_j_object;
            }

            section_container[field->get_name()] = {};
        }

        auto& field_container = section_container[field->get_name()];
        field->set_container(&field_container);
        return field_container;
    }

    static auto remove_field_container(internal::FieldEntry* field, const std::string& section, const std::string& name, bool shared) -> void {
        field->set_container(nullptr);

        nlohmann::json* root_container = s_profile_root;
        if (shared) {
            root_container = s_shared_root;
        }

        auto section_parts = string_split("::", section);
        std::list<std::pair<std::string, nlohmann::json*>> node_path;
        for (auto& part : section_parts) {
            if (!root_container->contains(part)) {
                return;
            }

            node_path.emplace_front(part, root_container);
            root_container = &(*root_container)[part];
        }

        if (!root_container->contains(name)) {
            return;
        }

        root_container->erase(name);
        for (auto& [key, node] : node_path) {
            if (!(*node)[key].empty()) {
                break;
            }

            node->erase(key);
        }
    }

    static auto update_field(internal::FieldEntry* _field) -> void {
        const auto& field_container = get_field_json_container(_field);
        _field->FromJson(field_container);
    }

    static auto update_not_shared() -> void {
        reset_not_shared();
        for (auto& entry : s_entries) {
            if (!entry->is_shared()) {
                update_field(entry.get());
            }
        }
    }

    static auto load_field(internal::FieldEntry* field) -> void {
        auto& field_container = get_field_json_container(field, true);

        const auto jObject = field->ToJson();
        if (jObject.empty()) {
            remove_field_container(field, field->get_section(), field->get_name(), field->is_shared());
        } else {
            field_container = jObject;
        }
    }

    static auto load_all() -> void {
        for (auto& entry : s_entries) {
            load_field(entry.get());
        }
    }

    static auto on_field_changed(internal::FieldEntry* field) -> void {
        load_field(field);
        save();
    }

    static auto on_field_moved(internal::FieldEntry* field, const std::string& oldSection, bool oldShared) -> void {
        remove_field_container(field, oldSection, field->get_name(), oldShared);
        on_field_changed(field);
    }

    static auto on_field_reposition(internal::FieldEntry* field, const std::string& oldSection, bool oldShared) -> void {
        field->set_container(nullptr);
        update_field(field);
    }

    auto internal::add_field(const std::shared_ptr<FieldEntry>& _field) -> void {
        s_entries.push_back(_field);
        update_field(_field.get());
        _field->changed_event += FUNCTION_HANDLER(on_field_changed);
        _field->moved_event += FUNCTION_HANDLER(on_field_moved);
        _field->reposition_event += FUNCTION_HANDLER(on_field_reposition);
    }

    auto refresh() -> void {
        load_all();
        save();
    }

    static auto save_internal() -> void {
        std::ofstream file_output(s_filepath, std::ios::out);
        if (!file_output.is_open()) {
            LOG_DEBUG << "打开配置文件进行写入失败.";
            update_save_timestamp();
            return;
        }

        file_output << s_config_root.dump(4);
        file_output.close();
    }

    auto save() -> void {
        if (s_update_event) {
            update_save_timestamp();
            return;
        }
        save_internal();
    }

    auto on_update() -> void {
        if (s_next_save_timestamp > 0 && std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() > s_next_save_timestamp) {
            s_next_save_timestamp = 0;
            save_internal();
        }
    }

    auto create_profile(const std::string& _profile_name, bool _move_after_create) -> void {
        if (s_profiles->contains(_profile_name)) {
            if (_move_after_create) {
                change_profile(_profile_name);
            }
            return;
        }

        (*s_profiles)[_profile_name] = {};
        update_profiles_names();

        if (_move_after_create) {
            change_profile(_profile_name);
        }
        save();
    }

    auto remove_profile(const std::string& _profile_name) -> void {
        if (!s_profiles->contains(_profile_name)) {
            return;
        }

        if (s_profiles->size() == 1) {
            return;
        }

        if (s_profile_name == _profile_name) {
            for (auto& [name, value] : s_profiles->items()) {
                if (name != _profile_name) {
                    change_profile(name);
                    break;
                }
            }
        }

        s_profiles->erase(_profile_name);
        update_profiles_names();
        save();
    }

    auto rename_profile(const std::string& _old_profile_name, const std::string& _new_profile_name) -> void {
        if (!s_profiles->contains(_old_profile_name) || s_profiles->contains(_new_profile_name)) {
            return;
        }

        if (s_profile_name == _old_profile_name) {
            s_profile_root = nullptr;
        }

        (*s_profiles)[_new_profile_name] = (*s_profiles)[_old_profile_name];
        s_profiles->erase(_old_profile_name);

        if (s_profile_root == nullptr) {
            for (const auto& entry : s_entries) {
                if (!entry->is_shared()) {
                    entry->set_container(nullptr);
                }
            }

            change_profile(_new_profile_name);
        }
        update_profiles_names();
        save();
    }

    auto change_profile(const std::string& _profile_name) -> void {
        if (s_profile_name == _profile_name) {
            return;
        }

        if (!s_profiles->contains(_profile_name)) {
            return;
        }

        std::lock_guard lock(s_profile_mutex);

        s_profile_root = &(*s_profiles)[_profile_name];
        s_profile_name = _profile_name;

        s_config_root["current_profile"] = _profile_name;
        update_not_shared();
        save();

        profile_changed();
    }

    auto duplicate_profile(const std::string& _profile_name) -> void {
        // Find a unique name for the new profile
        uint32_t counter = 0;
        std::ostringstream buffer;
        std::string new_profile_name;
        do {
            buffer.str(std::string());
            buffer.clear();
            counter++;
            buffer << _profile_name << " (" << counter << ")";
            new_profile_name = buffer.str();
        } while (s_profiles->contains(new_profile_name));

        // nlohmann::json copy constructor will take care of duplicating
        (*s_profiles)[new_profile_name] = (*s_profiles)[_profile_name];
        update_profiles_names();
        save();
    }

    auto get_profiles() -> const std::vector<std::string>& {
        return s_profiles_names;
    }

    auto current_profile_name() -> const std::string& {
        return s_profile_name;
    }
}
