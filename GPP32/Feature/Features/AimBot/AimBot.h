#pragma once
#include <pch.h>

#include "FeatureRegistrar.h"

#include ".class/BattleRole/BattleRole.h"

class AimBot final : public FeatureRegistrar<AimBot> {
public:
    AimBot();

    struct AimInfo {
        BattleRole* real_ptr;
        int64_t team;
        bool local;
        bool falling;
        std::pair<glm::vec3, int> screen_pos;
        glm::vec3 pos;
        glm::vec3 pos_head;
        glm::vec3 pos_neck;
        glm::vec3 pos_hip;
        glm::vec3 move_dir;
        float x_rot;
        float y_rot;
        bool dead;
    };

    std::mutex mutex;
    std::vector<AimInfo, mi_stl_allocator<AimInfo>> roles;
    std::vector<AimInfo, mi_stl_allocator<AimInfo>> roles_commit;
    std::atomic<AimInfo*> local_role = nullptr;
    std::atomic<AimInfo*> lock_role = nullptr;

    float bullet_speed;
    float bullet_gravity;

    auto render() -> void override;
    auto update() -> void override;

    auto process_data() -> void;
};
