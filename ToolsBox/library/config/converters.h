#pragma once

#include "../json.hpp"

namespace config::converters {
    template<typename T>
    auto ToJson(const T& value) -> nlohmann::json {
        return nlohmann::json(value);
    }

    template<typename T>
    auto FromJson(T& value, const nlohmann::json& jObject) -> void {
        value = jObject.get<T>();
    }
}
