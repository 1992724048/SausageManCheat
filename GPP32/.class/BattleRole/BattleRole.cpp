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

                if (!game_object[role]->get_active_in_hierarchy()) {
                    continue;
                }

                roles.push_back(role);
            } catch (...) {}
        }
    } catch (...) {}
}

BattleRole::BattleRole() {}

BattleRole::~BattleRole() {}
