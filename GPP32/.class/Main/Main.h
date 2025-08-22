#pragma once
#include "FeatureRegistrar.h"
#include "HardBreakPoint.h"
#include "dxhook.h"
#include "pch.h"

#include ".class/ClassRegistrar.h"
#include ".class/BattleRole/BattleRole.h"
#include ".class/BattleWorld/BattleWorld.h"
#include ".class/CameraController/CameraController.h"

#include "memory/W2C/W2C.h"

class Main final : public II::MonoBehaviour, public ClassRegistrar<Main> {
public:
    inline static I::MethodPointer<void, Main*> update;
    inline static IC* class_;

    Main();
    ~Main();

    static auto update_hook(Main* _i) -> void try {
        HardBreakPoint::call_origin(update_hook, _i);

        CameraController::tg_.update_start();
        const auto w2c = W2C::instance();
        w2c->wait();
        w2c->clear();

        BattleRole::roles.clear();

        try {
            if (util::is_bad_ptr(CameraController::camera_controller)) {
                return;
            }

            const auto trans = CameraController::camera_tran[CameraController::camera_controller];
            if (util::is_bad_ptr(trans)) {
                return;
            }

            CameraController::camera_pos = trans->GetPosition();

            const auto camera_ptr = CameraController::camera[CameraController::camera_controller];
            if (util::is_bad_ptr(camera_ptr)) {
                CameraController::camera_controller = nullptr;
                return;
            }

            const auto view = glm::inverse(camera_ptr->camera_to_world_matrix());
            w2c->camera_matrix = camera_ptr->get_projection_matrix() * view;
            w2c->screen_size = DXHook::size;

            CameraController::local_role = CameraController::lock_role[CameraController::camera_controller];
            if (util::is_bad_ptr(CameraController::local_role)) {
                CameraController::camera_controller = nullptr;
                return;
            }

            CameraController::local_role_logic = BattleRole::role_logic[CameraController::local_role];
            if (util::is_bad_ptr(CameraController::local_role_logic)) {
                CameraController::camera_controller = nullptr;
                return;
            }

            CameraController::local_role_statistics = BattleRoleLogic::statistics_data[CameraController::local_role_logic];
            if (util::is_bad_ptr(CameraController::local_role_statistics)) {
                CameraController::camera_controller = nullptr;
                return;
            }

            CameraController::battle_world = StatisticsData::game_world[CameraController::local_role_statistics];
            if (util::is_bad_ptr(CameraController::battle_world)) {
                CameraController::camera_controller = nullptr;
                return;
            }

            CameraController::start_game = BattleWorld::start_game[CameraController::battle_world];
            BattleRole::get_all();
        } catch (...) {
            CameraController::camera_controller = nullptr;
            return;
        }

        if (BattleRole::roles.empty()) {
            CameraController::camera_controller = nullptr;
            return;
        }

        FeatureBase::instance().update();
        w2c->start();
        std::unique_lock lock(CameraController::rw_lock);
        CameraController::update_time = CameraController::tg_.get_duration<std::chrono::milliseconds>();
    } catch (...) {
        MessageBoxA(nullptr, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
    }

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("Main");
        class_->Get<IM>("Update")->Cast(update);
        HardBreakPoint::set_break_point(update, update_hook);
    }
};
