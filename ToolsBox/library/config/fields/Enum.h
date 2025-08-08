#pragma once

#include "../../magic_enum/magic_enum.hpp"
#include "../../json.hpp"

namespace config {
    template<typename T>
    class Enum {
    public:
        Enum() {
            static_assert(std::is_enum_v<T>, "Must be an enum type");
            m_Value = T();
        }

        Enum(T enumValue) {
            static_assert(std::is_enum_v<T>, "Must be an enum type");
            m_Value = enumValue;
        }

        auto value() const -> T {
            return m_Value;
        }

        auto pointer() const -> T* {
            return const_cast<T*>(&m_Value);
        }

        operator T&() {
            return m_Value;
        }

        auto operator&() -> T* {
            return pointer();
        }

        auto raw() const -> uint32_t {
            return static_cast<uint32_t>(m_Value);
        }

        auto operator=(const T& other) -> Enum& {
            static_assert(std::is_enum_v<T>, "Must be an enum type");
            m_Value = other;
            return *this;
        }

        auto operator=(const uint32_t& other) -> Enum& {
            m_Value = static_cast<T>(other);
            return *this;
        }

    private:
        T m_Value;
    };

    template<typename K>
    class Field<Enum<K>> : public internal::FieldBase<Enum<K>> {
    public:
        using base = internal::FieldBase<Enum<K>>;
        using base::operator=;
        using base::base;

        operator bool() const {
            return base::value();
        }

        operator K&() const {
            return *base::value().operator&();
        }
    };
}

namespace nlohmann {
    template<typename T>
    struct adl_serializer<config::Enum<T>> {
        static auto to_json(json& j, const config::Enum<T>& enumValue) -> void {
            j = {{"name", magic_enum::enum_name(enumValue.value())}, {"value", enumValue.raw()}};
        }

        static auto from_json(const json& j, config::Enum<T>& value) -> void {
            value = j["value"].get<uint32_t>();
        }
    };
}
