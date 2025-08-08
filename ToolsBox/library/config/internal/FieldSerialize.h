#pragma once

#include "../config/converters.h"
#include "FieldEntry.h"

namespace config::internal {
    namespace CHECK {
        struct No {};

        template<typename T, typename Arg>
        auto operator==(const T&, const Arg&) -> No;

        template<typename T, typename Arg = T>
        struct EqualExists {
            enum { value = !std::is_same_v<decltype(std::declval<T>() == std::declval<Arg>()), No> };
        };
    }

    template<typename T>
    class FieldSerialize : public FieldEntry {
    public:
        FieldSerialize(const std::string& _friendly_name,
                       const std::string& _name,
                       const std::string& _section_name,
                       const T& _default_value,
                       const bool _multi_profile = false) : FieldEntry(_friendly_name, _name, _section_name, _multi_profile), m_value(_default_value), m_default_value(_default_value) {}

        auto ToJson() -> nlohmann::json override {
            if constexpr (CHECK::EqualExists<T>::value) {
                if (m_value == m_default_value) {
                    return {};
                }
            }


            return converters::ToJson(m_value);
        }

        auto FromJson(const nlohmann::json& jObject) -> void override {
            if (jObject.empty()) {
                m_value = m_default_value;
                return;
            }

            converters::FromJson(m_value, jObject);
        }

        auto Reset() -> void override {
            m_value = m_default_value;
        }

        T m_value;
        T m_default_value;
    };
}
