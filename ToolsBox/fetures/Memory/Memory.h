#pragma once
#include <fetures/FeatureRegistrar.hpp>

class Memory final : public FeatureRegistrar<Memory> {
public:
    Memory();
    ~Memory() = default;

    config::Field<bool> f_bullet_tracking;
    config::Field<bool> f_all_hit_head;
    config::Field<bool> f_lock_role;
    config::Field<bool> f_all_gun_auto;
    config::Field<bool> f_ballistics_tracking;
    config::Field<bool> f_damage_multi;
    config::Field<int> f_damage_multi_value;

    METHODS_BEGIN
        METHOD_ADD(mem_get)
        METHOD_ADD(mem_set)
    METHODS_END

    static auto mem_get(METHOD_ARGS) -> void;
    static auto mem_set(METHOD_ARGS) -> void;
};
