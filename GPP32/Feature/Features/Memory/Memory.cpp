#include "Memory.h"

#include ".class/CameraController/CameraController.h"

#include "memory/Memory/MemoryConfig.h"

Memory::Memory() {}

auto Memory::render() -> void {}

auto Memory::update() -> void {
    static auto last_gc_time = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    if (now - last_gc_time >= 5min) {
        II::GC::Collect();
        last_gc_time = now;
    }

    const auto cfg = MemoryConfig::instance();
    auto lock = cfg->mutex();

    try {
        if (util::is_bad_ptr(CameraController::local_role)) {
            goto next1;
        }

        const auto weapon = BattleRole::user_weapon[CameraController::local_role];
        if (util::is_bad_ptr(weapon)) {
            goto next1;
        }

        const auto control = WeaponControl::so_weapon_control[weapon];
        if (util::is_bad_ptr(control)) {
            goto next1;
        }

        if (cfg->bullet_no_gravity) {
            const auto array = std::move(SOWeaponControl::bullet_speed_and_gravity[control]->to_vector());
            for (const auto& bullet_speed_and_gravity : array) {
                BulletSpeedAndGravity::gravity[bullet_speed_and_gravity] = 0;
                BulletSpeedAndGravity::fly_time[bullet_speed_and_gravity] = 80000;
            }
        }

        if (cfg->all_gun_auto) {
            SOWeaponControl::has_auto_fire[control] = true;
        }
    } catch (...) {}
    
    next1:
    return;
}
