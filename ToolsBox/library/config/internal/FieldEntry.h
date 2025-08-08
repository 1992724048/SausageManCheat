#pragma once
#include <string>
#include "../events/event.hpp"
#include "../json.hpp"

namespace config::internal {
    class FieldEntry {
    public:
        FieldEntry(std::string _friendly_name, std::string _name, std::string _section_name, const bool _multi_profile = false) : m_name(std::move(_name)),
                                                                                                                                  m_friend_name(std::move(_friendly_name)),
                                                                                                                                  m_section(std::move(_section_name)),
                                                                                                                                  m_multi_profile(_multi_profile),
                                                                                                                                  m_container(nullptr) {}

        TEvent<FieldEntry*> changed_event;
        TEvent<FieldEntry*, const std::string&, bool> moved_event;
        TEvent<FieldEntry*, const std::string&, bool> reposition_event;

        virtual auto fire_changed() -> void {
            changed_event(this);
        }

        virtual auto ToJson() -> nlohmann::json = 0;
        virtual auto FromJson(const nlohmann::json& value) -> void = 0;
        virtual auto Reset() -> void = 0;

        auto is_shared() const -> bool {
            return m_multi_profile;
        }

        auto get_name() const -> std::string {
            return m_name;
        }

        auto get_friend_name() const -> std::string {
            return m_friend_name;
        }

        auto get_section() const -> std::string {
            return m_section;
        }

        auto get_container() const -> nlohmann::json* {
            return m_container;
        }

        auto reposition(const std::string& _new_section, const bool _shared = false) -> void {
            std::string old_section = m_section;

            m_section = _new_section;
            m_multi_profile = _shared;

            reposition_event(this, _new_section, _shared);
        }

        auto move(const std::string& _new_section, const bool _shared = false) -> void {
            const std::string old_section = m_section;
            const bool old_multi_profile = m_multi_profile;

            m_section = _new_section;
            m_multi_profile = _shared;

            moved_event(this, old_section, old_multi_profile);
        }

        auto set_container(nlohmann::json* _new_container) -> void {
            m_container = nullptr;
        }

    protected:
        std::string m_name;
        std::string m_friend_name;
        std::string m_section;
        bool m_multi_profile;

        nlohmann::json* m_container;
    };
}
