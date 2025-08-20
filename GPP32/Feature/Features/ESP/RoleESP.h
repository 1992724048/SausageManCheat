#pragma once
#include <pch.h>

#include "FeatureRegistrar.h"

#include ".class/BattleRole/BattleRole.h"

class ESP final : public FeatureRegistrar<ESP> {
public:
    ESP();

    struct Role {
        util::String name;
        int64_t team;
        float hp;
        float weak;
        bool local;
        bool falling;
        std::pair<glm::vec3, int> screen_pos;
        std::pair<glm::vec3, int> screen_pos_top;
        std::pair<glm::vec3, int> screen_pos_neck;
        util::Map<II::Animator::HumanBodyBones, std::pair<glm::vec3, int>> bones;
        glm::vec3 scale;
        glm::vec3 pos;
        std::array<std::pair<glm::vec3, int>, 16> rect_pos_upper;
        std::array<std::pair<glm::vec3, int>, 16> rect_pos_lower;
        float x_rot;
        float y_rot;
        BattleRole* role;
        RoleControl* role_control;
        BattleRoleLogic* role_logic;
        bool dead;
        bool hide;
    };

    std::mutex mutex;
    std::vector<Role, mi_stl_allocator<Role>> roles;
    std::vector<Role, mi_stl_allocator<Role>> roles_commit;
    std::atomic<Role*> local_role = nullptr;

    auto render() -> void override;
    auto update() -> void override;

    static auto draw_info(ImDrawList* _bg, const glm::vec3& _screen_pos, const util::String& _name, int64_t _team, float _weak, float _hp, bool _falling) -> void;
    auto draw_bone(ImDrawList* _bg, util::Map<II::Animator::HumanBodyBones, std::pair<glm::vec3, int>>& _bones) -> void;
    static auto draw_box(ImDrawList* _bg, const std::array<std::pair<glm::vec3, int>, 16>& _rect_pos_upper, const std::array<std::pair<glm::vec3, int>, 16>& _rect_pos_lower) -> void;

    auto process_data() -> void;
    static auto process_box(const glm::vec3& _scale_,
                            const glm::vec3& _pos_,
                            const std::pair<glm::vec3, int>& _pos_top,
                            std::array<std::pair<glm::vec3, int>, 16>& _rect_pos_upper,
                            std::array<std::pair<glm::vec3, int>, 16>& _rect_pos_lower,
                            float _rot) -> void;
    auto process_bone(::UnityResolve::UnityType::Animator* _animator, util::Map<II::Animator::HumanBodyBones, std::pair<glm::vec3, int>>& _bones, std::pair<glm::vec3, int>& _pos_bottom) const -> void;

    std::vector<II::Animator::HumanBodyBones> stick_figure_bones = {
        II::Animator::HumanBodyBones::Hips,
        II::Animator::HumanBodyBones::Spine,
        II::Animator::HumanBodyBones::Chest,
        II::Animator::HumanBodyBones::Neck,
        II::Animator::HumanBodyBones::Head,
        II::Animator::HumanBodyBones::LeftUpperLeg,
        II::Animator::HumanBodyBones::LeftLowerLeg,
        II::Animator::HumanBodyBones::LeftFoot,
        II::Animator::HumanBodyBones::RightUpperLeg,
        II::Animator::HumanBodyBones::RightLowerLeg,
        II::Animator::HumanBodyBones::RightFoot,
        II::Animator::HumanBodyBones::LeftShoulder,
        II::Animator::HumanBodyBones::LeftUpperArm,
        II::Animator::HumanBodyBones::LeftLowerArm,
        II::Animator::HumanBodyBones::LeftHand,
        II::Animator::HumanBodyBones::RightShoulder,
        II::Animator::HumanBodyBones::RightUpperArm,
        II::Animator::HumanBodyBones::RightLowerArm,
        II::Animator::HumanBodyBones::RightHand
    };

    std::vector<std::pair<II::Animator::HumanBodyBones, II::Animator::HumanBodyBones>> stick_figure_connections = {
        // 躯干
        {II::Animator::HumanBodyBones::Hips, II::Animator::HumanBodyBones::Spine},
        {II::Animator::HumanBodyBones::Spine, II::Animator::HumanBodyBones::Chest},
        {II::Animator::HumanBodyBones::Chest, II::Animator::HumanBodyBones::Neck},
        {II::Animator::HumanBodyBones::Neck, II::Animator::HumanBodyBones::Head},

        // 左腿
        {II::Animator::HumanBodyBones::Hips, II::Animator::HumanBodyBones::LeftUpperLeg},
        {II::Animator::HumanBodyBones::LeftUpperLeg, II::Animator::HumanBodyBones::LeftLowerLeg},
        {II::Animator::HumanBodyBones::LeftLowerLeg, II::Animator::HumanBodyBones::LeftFoot},

        // 右腿
        {II::Animator::HumanBodyBones::Hips, II::Animator::HumanBodyBones::RightUpperLeg},
        {II::Animator::HumanBodyBones::RightUpperLeg, II::Animator::HumanBodyBones::RightLowerLeg},
        {II::Animator::HumanBodyBones::RightLowerLeg, II::Animator::HumanBodyBones::RightFoot},

        // 左臂
        {II::Animator::HumanBodyBones::Chest, II::Animator::HumanBodyBones::LeftShoulder},
        {II::Animator::HumanBodyBones::LeftShoulder, II::Animator::HumanBodyBones::LeftUpperArm},
        {II::Animator::HumanBodyBones::LeftUpperArm, II::Animator::HumanBodyBones::LeftLowerArm},
        {II::Animator::HumanBodyBones::LeftLowerArm, II::Animator::HumanBodyBones::LeftHand},

        // 右臂
        {II::Animator::HumanBodyBones::Chest, II::Animator::HumanBodyBones::RightShoulder},
        {II::Animator::HumanBodyBones::RightShoulder, II::Animator::HumanBodyBones::RightUpperArm},
        {II::Animator::HumanBodyBones::RightUpperArm, II::Animator::HumanBodyBones::RightLowerArm},
        {II::Animator::HumanBodyBones::RightLowerArm, II::Animator::HumanBodyBones::RightHand}
    };
};
