#include "MemoryConfig.h"

#include "fetures/Memory/Memory.h"

MemoryConfig::MemoryConfig() {
    
}

void MemoryConfig::config_callback() {
    auto lock = mutex();
    const auto i = Memory::instance();
    bullet_tracking = i->f_bullet_tracking;
    all_hit_head = i->f_all_hit_head;
    damage_multi = i->f_damage_multi;
    damage_multi_value = i->f_damage_multi_value;
    lock_role = i->f_lock_role;
    all_gun_auto = i->f_all_gun_auto;
    ballistics_tracking = i->f_ballistics_tracking;
    bullet_no_gravity = i->f_bullet_no_gravity;
    random = i->f_random;
}
