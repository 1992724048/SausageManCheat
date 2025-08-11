#pragma once
#include "pch.h"
#include "HardBreakPoint.h"
#include ".class/ClassRegistrar.h"
#include ".class/AnimatorControl/AnimatorControl.h"
#include ".class/BodyPart/BodyPart.h"
#include ".class/PickItem/PickItem.h"

#include "Features/AimBot/AimBot.h"

#include "memory/Memory/MemoryConfig.h"

class BulletControl final : public ClassRegistrar<BulletControl> {
public:
    inline static I::MethodPointer<void, BulletControl*, BodyPart*> hit_role;
    inline static I::MethodPointer<void, BulletControl*, StartGame*, int, int, int, II::String*, int, glm::vec3, glm::quat> local_role_weapon_init;

    inline static IF::Variable<BulletControl, bool> is_clear;
    inline static IF::Variable<BulletControl, II::Array<BulletSpeedAndGravity*>*> speed_gravity;

    inline static IC* class_;

    BulletControl();
    ~BulletControl();

    static auto local_role_weapon_init_hook(BulletControl* _bc, StartGame* _a, const int _b, const int _c, const int _d, II::String* _e, const int _f, glm::vec3 _fire_pos, glm::quat _rot) -> void {
        const auto mem = MemoryConfig::instance();
        mem->mutex();

        HardBreakPoint::call_origin(local_role_weapon_init_hook, _bc, _a, _b, _c, _d, _e, _f, _fire_pos, _rot);

        float speed = std::numeric_limits<float>::max();
        float gravity = 0.f;

        try {
            const auto sg = speed_gravity[_bc]->ToVector()[0];
            speed = BulletSpeedAndGravity::bullet_speed[sg];
            gravity = BulletSpeedAndGravity::gravity[sg];
        } catch (...) {}

        if (mem->ballistics_tracking) {
            const auto aim = AimBot::instance();
            if (const auto target = aim->lock_role.load()) {
                const float dist = glm::distance(_fire_pos, target->pos_head);

                float flight_time = dist / speed;
                float dt = 1.0f / ImGui::GetIO().Framerate;
                glm::vec3 target_velocity = target->move_dir / dt;

                for (int i = 0; i < 2; ++i) {
                    glm::vec3 predicted_pos = target->pos_head + target_velocity * flight_time;

                    if (gravity != 0.f) {
                        predicted_pos.y += 0.5f * gravity * flight_time * flight_time;
                    }

                    float new_dist = glm::distance(_fire_pos, predicted_pos);
                    flight_time = new_dist / speed;
                }

                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_real_distribution dis(0.0f, 1.0f);
                static std::uniform_real_distribution hit(0.0f, 1.0f);

                float r = dis(gen);
                float hit_pos = hit(gen);
                glm::vec3 aim_point = target->pos_head;

                if (mem->random) {
                    if (r < 0.4f) {
                        aim_point = target->pos_head;
                    } else if (r < 0.8f) {
                        aim_point = target->pos_hip;
                    } else {
                        if (r < 0.9f) {
                            aim_point = target->pos_head + (hit_pos > 0.5f ? glm::vec3(1.5f, 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.5f));
                        } else {
                            aim_point = target->pos_hip + (hit_pos < 0.5f ? glm::vec3(1.5f, 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.5f));
                        }
                    }
                }

                glm::vec3 final_pos = aim_point + target_velocity * flight_time;
                if (gravity != 0.f) {
                    final_pos.y += 0.5f * gravity * flight_time * flight_time;
                }

                glm::vec3 dir = glm::normalize(final_pos - _fire_pos);
                _rot = glm::quatLookAtLH(dir, glm::vec3(0, 1, 0));
            }
        }

        return HardBreakPoint::call_origin(local_role_weapon_init_hook, _bc, _a, _b, _c, _d, _e, _f, _fire_pos, _rot);
    }

    static auto hit_role_hook(BulletControl* _i, BodyPart* _body) -> void {
        const auto mem = MemoryConfig::instance();
        mem->mutex();

        try {
            if (mem->all_hit_head) {
                BodyPart::body_type[_body] = BodyPartData::Head;
            }

            if (mem->damage_multi) {
                for (int i = 0; i < mem->damage_multi_value; ++i) {
                    HardBreakPoint::call_origin(hit_role_hook, _i, _body);
                }
            }
        } catch (...) {}

        HardBreakPoint::call_origin(hit_role_hook, _i, _body);
    }

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("$iA");
        is_clear.Init(class_->Get<IF>("$b"));
        speed_gravity.Init(class_->Get<IF>("$I"));

        class_->Get<IM>("$sB")->Cast(hit_role);
        class_->Get<IM>("$Mb")->Cast(local_role_weapon_init);
        // HardBreakPoint::set_break_point(hit_role, hit_role_hook);
        HardBreakPoint::set_break_point(local_role_weapon_init, local_role_weapon_init_hook);
    }

    /*
        if (mem->bullet_tracking) {
            const auto aim = AimBot::instance();
            glm::vec3 _pos2;
            if (const auto aim_role = aim->lock_role.load()) {
                LOG_DEBUG << "x:" << aim_role->pos_head.x << " y:" << aim_role->pos_head.y << " z:" << aim_role->pos_head.z;
                _pos2 = aim_role->pos_head;
            } else {
                goto no_tg;
            }

            LOG_DEBUG << "is_clear1:" << is_clear[_bc] << " is_clear2:" << is_clear2[_bc] << " is_clear3:" << is_clear3[_bc] << " is_clear4:" << is_clear4[_bc] << " is_clear5:" << is_clear5[_bc];
            HardBreakPoint::call_origin(local_role_weapon_init_Hook, _bc, _a, _b, _c, _d, _e, _f, _pos, _g);
            LOG_DEBUG << "is_clear1:" << is_clear[_bc] << " is_clear2:" << is_clear2[_bc] << " is_clear3:" << is_clear3[_bc] << " is_clear4:" << is_clear4[_bc] << " is_clear5:" << is_clear5[_bc];
            HardBreakPoint::call_origin(local_role_weapon_init_Hook, _bc, _a, _b, _c, _d, _e, _f, _pos2, _g);
            LOG_DEBUG << "is_clear1:" << is_clear[_bc] << " is_clear2:" << is_clear2[_bc] << " is_clear3:" << is_clear3[_bc] << " is_clear4:" << is_clear4[_bc] << " is_clear5:" << is_clear5[_bc];

            /*is_clear[_bc] = false;
            is_clear2[_bc] = false;
            is_clear3[_bc] = false;
            is_clear4[_bc] = false;
            is_clear5[_bc] = false;#1#

            is_clear[_bc] = false;
            is_clear2[_bc] = false;
            return;
        }
        */
};
