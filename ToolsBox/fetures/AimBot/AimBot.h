#pragma once
#include <fetures/FeatureRegistrar.hpp>

class AimBot final : public FeatureRegistrar<AimBot> {
public:
    AimBot();
    ~AimBot() = default;

    config::Field<bool> f_enable;
    config::Field<bool> f_random;
    config::Field<double> f_rect_w;
    config::Field<double> f_rect_h;
    config::Field<double> f_speed;
    config::Field<int> f_hotkey;

    METHODS_BEGIN
        METHOD_ADD(aim_get)
        METHOD_ADD(aim_set)
    METHODS_END

    static auto aim_get(METHOD_ARGS) -> void;
    static auto aim_set(METHOD_ARGS) -> void;
};
