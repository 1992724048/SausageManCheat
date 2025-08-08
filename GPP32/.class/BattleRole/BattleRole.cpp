#include "BattleRole.h"

#include ".class/CameraController/CameraController.h"
#include ".class/StartGame/StartGame.h"

auto BattleRole::get_all() -> void {
    try {
        roles.clear();

        if (util::is_bad_ptr(CameraController::start_game)) {
            LOG_DEBUG << "start_game is nullptr";
            return;
        }

        const auto role_list = StartGame::role_list[CameraController::start_game];
        if (util::is_bad_ptr(role_list)) {
            LOG_DEBUG << "role_list is nullptr";
            return;
        }

        roles.reserve(role_list->size);
        const auto role_vec = std::move(role_list->ToArray()->ToVector());
        for (const auto& role_lg : role_vec) {
            const auto lg = BattleRoleLogic::client[role_lg];
            if (util::is_bad_ptr(role_lg)) {
                continue;
            }

            const auto client = RoleLogicClient::client[lg];
            if (util::is_bad_ptr(client)) {
                continue;
            }

            roles.push_back(client);
        }

        LOG_DEBUG << "roles :" << roles.size();
    } catch (...) {}
}

BattleRole::BattleRole() {}

BattleRole::~BattleRole() {}
