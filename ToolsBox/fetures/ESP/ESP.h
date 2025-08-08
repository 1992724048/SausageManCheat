#pragma once
#include <fetures/FeatureRegistrar.hpp>

class ESP final : public FeatureRegistrar<ESP> {
public:
    ESP();
    ~ESP() = default;

    config::Field<bool> f_enable;
    config::Field<bool> f_show_ai;
    config::Field<bool> f_show_box;
    config::Field<bool> f_show_role;
    config::Field<bool> f_show_info;
    config::Field<bool> f_show_bone;
    config::Field<bool> f_show_team;
    config::Field<bool> f_show_item;
    config::Field<bool> f_show_cars;

    METHODS_BEGIN
        METHOD_ADD(esp_get)
        METHOD_ADD(esp_set)
    METHODS_END

    static auto esp_get(METHOD_ARGS) -> void;
    static auto esp_set(METHOD_ARGS) -> void;
};
