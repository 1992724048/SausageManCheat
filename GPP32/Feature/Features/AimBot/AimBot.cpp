#include "AimBot.h"

#include <execution>
#include <random>
#include <unordered_set>

#include ".class/BattleRole/BattleRole.h"
#include ".class/CameraController/CameraController.h"

#include "glm/gtx/compatibility.hpp"
#include "glm/gtx/euler_angles.hpp"

#include "magic_enum/magic_enum.hpp"
#include "memory/AimBot/AimBotConfig.h"
#include "memory/Memory/MemoryConfig.h"
#include "memory/W2C/W2C.h"
#include "Feature/Features/ESP/RoleESP.h"

AimBot::AimBot() {}

struct HotKey {
    bool is_mouse;
    int code;
};

auto decode(const int encoded) -> HotKey {
    HotKey hk{};
    hk.is_mouse = (encoded >> 31) & 1;
    hk.code = encoded & 0xFF;
    return hk;
}

auto is_pressed(const int encoded) -> bool {
    auto hk = decode(encoded);
    if (hk.is_mouse) {
        return GetAsyncKeyState(VK_LBUTTON + (hk.code == 0x01 ? 0 : hk.code == 0x02 ? 1 : hk.code == 0x04 ? 2 : hk.code == 0x05 ? 3 : 4)) & 0x8000;
    }
    return (GetAsyncKeyState(hk.code) & 0x8000) != 0;
}

auto calculate_angles(const glm::vec3& _from_point, const glm::vec3& _to_point) -> glm::vec2 {
    const glm::vec3 direction = _to_point - _from_point;
    float angle_yaw = glm::degrees(glm::atan2(direction.x, direction.z));

    if (angle_yaw < 0.0f) {
        angle_yaw += 360.0f;
    }

    float angle_pitch = -glm::degrees(glm::atan2(direction.y, glm::length(glm::vec2(direction.x, direction.z))));
    angle_pitch = glm::clamp(angle_pitch, -65.0f, 75.0f);

    return glm::vec2(angle_yaw, angle_pitch);
}

auto calc_quat(const glm::vec2 _euler) -> std::pair<glm::quat, glm::quat> {
    auto quaternion_x = glm::quat(glm::radians(glm::vec3(_euler.x, 0, 0)));
    auto quaternion_y = glm::quat(glm::radians(glm::vec3(0, _euler.y, 0)));
    return {{quaternion_x.w, 0, quaternion_x.x, 0}, {quaternion_y.w, quaternion_y.y, 0, 0}};
}

auto AimBot::render() -> void try {
    struct lock_ctx {
        BattleRole* ptr = nullptr;
        int64_t id = 0;
    };
    static lock_ctx lock{};

    std::vector<AimInfo, mi_stl_allocator<AimInfo>> temp;
    {
        std::lock_guard lg(mutex);
        if (roles.empty()) {
            return;
        }
        temp = roles;
    }

    if (temp.empty()) {
        return;
    }

    const auto cfg = AimBotConfig::instance();
    cfg->mutex();
    if (!cfg->enable) {
        return;
    }

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    const ImVec2 scr_center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);

    const ImVec2 r_min(scr_center.x - cfg->rect_w * 0.5f, scr_center.y - cfg->rect_h * 0.5f);
    const ImVec2 r_max(scr_center.x + cfg->rect_w * 0.5f, scr_center.y + cfg->rect_h * 0.5f);
    draw->AddRect(r_min, r_max, IM_COL32(0, 255, 0, 255), 0.0f, 0, 1.0f);

    AimInfo* best = nullptr;
    float best_dist = FLT_MAX;

    const auto local = local_role.load();
    if (!local) {
        return;
    }

    lock_role = nullptr;
    for (auto& a : temp) {
        if (a.real_ptr != lock.ptr || lock.ptr == nullptr) {
            if (!util::is_bad_ptr(lock.ptr)) {
                BattleRole::is_clear[lock.ptr] = false;
            }
        }

        if (a.hide || a.local || a.team == local->team) {
            continue;
        }
        if (a.screen_pos.first.z < 0.f) {
            continue;
        }
        if (a.dead || a.falling) {
            continue;
        }

        const ImVec2 sp(a.screen_pos.first.x, a.screen_pos.first.y);
        if (sp.x < r_min.x || sp.x > r_max.x || sp.y < r_min.y || sp.y > r_max.y) {
            continue;
        }

        const float d = glm::distance(glm::vec2(sp.x, sp.y), glm::vec2(scr_center.x, scr_center.y));
        if (d < best_dist) {
            best_dist = d;
            best = &a;
            lock_role = best;
        }
    }

    if (!is_pressed(cfg->hotkey)) {
        lock.ptr = nullptr;
        lock.id = 0;
        try {
            if (best) {
                draw->AddCircleFilled(ImVec2(best->screen_pos.first.x, best->screen_pos.first.y), 5.0f, IM_COL32(255, 0, 0, 255));
            }
        } catch (...) {}
    } else {
        bool still_valid = false;
        if (lock.ptr && lock.id) {
            for (const auto& a : temp) {
                if (a.real_ptr == lock.ptr && !a.hide && !a.dead && !a.local && a.team != local_role.load()->team) {
                    still_valid = true;
                    break;
                }
            }
        }

        if (!still_valid) {
            lock.ptr = best ? best->real_ptr : nullptr;
            lock.id = best ? reinterpret_cast<int64_t>(best->real_ptr) : 0;
        }
    }

    if (lock.ptr && lock.id) {
        const AimInfo* tgt = nullptr;
        for (const auto& a : temp) {
            if (a.real_ptr == lock.ptr) {
                tgt = &a;
                break;
            }
        }

        if (!tgt) {
            return;
        }

        const auto cfg_mem = MemoryConfig::instance();
        cfg_mem->mutex();
        try {
            if (cfg_mem->lock_role) {
                if (!util::is_bad_ptr(lock.ptr)) {
                    BattleRole::is_clear[lock.ptr] = true;
                }
            }

            glm::quat& x_q = CameraController::camera_rotation_x[CameraController::camera_controller];
            glm::quat& y_q = CameraController::camera_rotation_y[CameraController::camera_controller];

            static BattleRole* last_target_ptr = nullptr;
            static glm::vec3 offset(0.0f);
            static float next_change_time = 0.0f;
            static std::mt19937 gen(std::random_device{}());

            if (cfg->random) {
                float now_time = ImGui::GetTime();
                bool need_new_offset = false;

                if (tgt->real_ptr != last_target_ptr) {
                    last_target_ptr = tgt->real_ptr;
                    need_new_offset = true;
                } else if (now_time >= next_change_time) {
                    need_new_offset = true;
                }

                if (need_new_offset) {
                    std::uniform_real_distribution time_dis(0.05f, 0.2f);
                    next_change_time = now_time + time_dis(gen);

                    std::uniform_real_distribution chance(0.0f, 1.0f);
                    if (chance(gen) < 0.5f) {
                        std::uniform_real_distribution jitter(-0.05f, 0.05f);
                        offset = glm::vec3(jitter(gen), jitter(gen), jitter(gen));
                    } else {
                        offset = glm::vec3(0.0f);
                    }
                }
            }
            glm::vec3 target_pos = cfg->random ? tgt->pos_head + offset : tgt->pos_head;

            const float dist = glm::distance(local->pos_head, target_pos);
            float dist2 = dist - 2.0f;
            if (dist2 < 0) {
                dist2 = 0;
            }
            const float flight_time = dist2 / bullet_speed;
            if (bullet_gravity != 0.f) {
                target_pos.y += 0.5f * bullet_gravity * flight_time * flight_time;
            }

            glm::vec2 angles = calculate_angles(CameraController::camera_pos, target_pos);
            auto [xq_target, yq_target] = calc_quat(angles);
            if (cfg->speed <= 1.0f) {
                x_q = xq_target;
                y_q = yq_target;
                return;
            }

            const ESP::Role* role = ESP::instance()->local_role.load();
            const float actual_speed = 12.1f - cfg->speed;
            if (!role) {
                return;
            }

            float cur_pitch_deg = glm::degrees(glm::eulerAngles(y_q).x);
            float cur_yaw_deg = role->y_rot;

            const float target_yaw = angles.x;
            const float target_pitch = angles.y;

            const float diff_yaw = target_yaw - cur_yaw_deg;
            if (glm::abs(diff_yaw) <= actual_speed) {
                cur_yaw_deg = target_yaw;
            } else {
                cur_yaw_deg = cur_yaw_deg + glm::sign(diff_yaw) * actual_speed;
            }

            const float diff_pitch = target_pitch - cur_pitch_deg;
            if (glm::abs(diff_pitch) <= actual_speed) {
                cur_pitch_deg = target_pitch;
            } else {
                cur_pitch_deg += glm::sign(diff_pitch) * actual_speed;
            }

            cur_pitch_deg = glm::clamp(cur_pitch_deg, -90.0f, 90.0f);
            auto [new_xq, new_yq] = calc_quat(glm::vec2(cur_yaw_deg, cur_pitch_deg));
            x_q = new_xq;
            y_q = new_yq;
        } catch (...) {}
    }
} catch (...) {
    MessageBoxA(DXHook::hwnd, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}

auto AimBot::update() -> void try {
    const auto w2c = W2C::instance();
    process_data();
    roles_commit.clear();
    roles_commit.resize(BattleRole::roles.size());

    int index = 0;
    for (const auto& role : BattleRole::roles) {
        try {
            AimInfo& _i = roles_commit[index++];

            if (util::is_bad_ptr(role)) {
                continue;
            }

            const auto role_control = BattleRole::role_control[role];
            if (util::is_bad_ptr(role_control)) {
                continue;
            }

            const auto animator_control = RoleControl::animator_control[role_control];
            if (util::is_bad_ptr(animator_control)) {
                continue;
            }

            const auto animator = AnimatorControl::animator[animator_control];
            if (util::is_bad_ptr(animator_control)) {
                continue;
            }

            const auto pos_trans = BattleRole::pos[role];
            if (util::is_bad_ptr(pos_trans)) {
                continue;
            }

            const auto trans = AnimatorControl::head[animator_control];
            if (util::is_bad_ptr(trans)) {
                continue;
            }

            const auto trans_neck = animator->GetBoneTransform(II::Animator::HumanBodyBones::Neck);
            if (util::is_bad_ptr(trans_neck)) {
                continue;
            }

            const auto trans_hip = animator->GetBoneTransform(II::Animator::HumanBodyBones::Hips);
            if (util::is_bad_ptr(trans_hip)) {
                continue;
            }

            BattleRoleLogic* role_logic = BattleRole::role_logic[role];
            _i.pos = pos_trans->GetPosition();
            _i.team = BattleRoleLogic::team[role_logic];
            const auto hp = BattleRoleLogic::hp[role_logic];
            const auto weak = BattleRoleLogic::weak[role_logic];
            _i.x_rot = RoleControl::x_cam_rot[role_control];
            _i.y_rot = RoleControl::y_cam_rot[role_control];
            _i.local = BattleRoleLogic::local[role_logic];
            _i.falling = weak != 100 && hp == 0;
            _i.dead = weak == 0 && hp == 0;
            _i.pos_neck = trans_neck->GetPosition();
            _i.pos_hip = trans_hip->GetPosition();
            _i.pos_head = glm::mix(_i.pos_neck, trans->GetPosition(), 0.7);
            _i.screen_pos.second = w2c->commit(_i.pos_head);
            _i.real_ptr = role;
        } catch (...) {}
    }
} catch (...) {
    MessageBoxA(DXHook::hwnd, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}

auto AimBot::process_data() -> void try {
    if (roles_commit.empty()) {
        return;
    }

    const auto w2c = W2C::instance();
    for (auto& value : roles_commit) {
        value.screen_pos.first = w2c->pos_done[value.screen_pos.second];

        if (value.local) {
            local_role = &value;
        }
    }

    try {
        if (util::is_bad_ptr(CameraController::local_role)) {
            roles.clear();
            return;
        }

        const auto weapon = BattleRole::user_weapon[CameraController::local_role];
        if (util::is_bad_ptr(weapon)) {
            roles.clear();
            return;
        }

        const auto control = WeaponControl::so_weapon_control[weapon];
        if (util::is_bad_ptr(control)) {
            roles.clear();
            return;
        }

        const auto weapon_array = SOWeaponControl::bullet_speed_and_gravity[control];
        if (util::is_bad_ptr(weapon_array)) {
            roles.clear();
            return;
        }

        const auto array = std::move(weapon_array->ToVector());
        for (auto& value : array) {
            bullet_gravity = BulletSpeedAndGravity::gravity[value];
            bullet_speed = BulletSpeedAndGravity::bullet_speed[value];
        }
    } catch (...) {
        roles.clear();
        return;
    }

    const auto local = local_role.load();
    for (auto& role : roles_commit) {
        if (role.local) {
            continue;
        }

        role.hide = true;

        try {
            II::RaycastHit hit;
            if (!II::Physics::linecast(local->pos_head, role.pos_head, hit)) {
                continue;
            }

            const auto collider = II::UnityObject::find_object_from_instance_id<II::Collider*>(hit.m_Collider);
            if (util::is_bad_ptr(collider)) {
                continue;
            }

            const auto role_ptr = collider->get_component_in_parent<BattleRole*>(BattleRole::class_);
            if (util::is_bad_ptr(role_ptr) || local->real_ptr == role_ptr || role_ptr != role.real_ptr) {
                continue;
            }

            role.hide = false;
        } catch (...) {}
    }

    std::lock_guard lock_s(mutex);
    roles = std::move(roles_commit);
} catch (...) {
    MessageBoxA(DXHook::hwnd, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}
