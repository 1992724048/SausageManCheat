#include "BattleRole.h"

#include ".class/CameraController/CameraController.h"
#include ".class/StartGame/StartGame.h"

auto BattleRole::get_all() -> void {
    try {
        roles.clear();

        if (util::is_bad_ptr(CameraController::start_game)) {
            return;
        }

        const auto role_list = StartGame::role_list[CameraController::start_game];
        if (util::is_bad_ptr(role_list)) {
            return;
        }

        roles.reserve(role_list->size);
        const auto role_vec = std::move(role_list->ToArray()->ToVector());
        for (const auto& role_lg : role_vec) {
            try {
                if (util::is_bad_ptr(role_lg)) {
                    continue;
                }

                auto role = BattleRoleLogic::get_role(role_lg);

                if (util::is_bad_ptr(role)) {
                    continue;
                }

                const auto control = role_control[role];
                if (util::is_bad_ptr(control)) {
                    continue;
                }

                const auto animator = RoleControl::animator_control[control];
                if (util::is_bad_ptr(animator)) {
                    continue;
                }

                const auto skin = AnimatorControl::role_skin_manager[animator];
                if (util::is_bad_ptr(skin)) {
                    continue;
                }

                const auto base_skin = RoleSkinManager::base_skin_manager[skin];
                if (util::is_bad_ptr(base_skin)) {
                    continue;
                }

                const auto root_bone = SkinManager::root_bone_data[base_skin];
                if (util::is_bad_ptr(root_bone)) {
                    continue;
                }

                roles.push_back(role);
            } catch (...) {}
        }
    } catch (...) {
        
    }
}

BattleRole::BattleRole() {}

BattleRole::~BattleRole() {}
