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

        roles.resize(role_list->size);
        const auto role_vec = role_list->ToArray();
        for (unsigned i = 0; i < role_list->size; i++) {
            try {
                const auto role_lg = role_vec->at(i);
                if (util::is_bad_ptr(role_lg)) {
                    continue;
                }

                auto role = role_lg->api_get_role();
                if (util::is_bad_ptr(role)) {
                    continue;
                }

                if (!game_object[role]->get_active_in_hierarchy()) {
                    continue;
                }

                roles[i] = role;
            } catch (...) {}
        }
    } catch (...) {}
}

BattleRole::BattleRole() {}

BattleRole::~BattleRole() {}
