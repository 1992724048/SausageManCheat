#pragma once
#include "FeatureRegistrar.h"
#include "HardBreakPoint.h"
#include "dxhook.h"
#include "pch.h"

#include ".class/ClassRegistrar.h"
#include ".class/BattleRole/BattleRole.h"
#include ".class/BattleWorld/BattleWorld.h"

#include "memory/W2C/W2C.h"

class CameraController final : public II::MonoBehaviour, public ClassRegistrar<CameraController> {
public:
    inline static tp::TimeGuard tg_;

    inline static IF::Variable<CameraController, glm::quat> camera_rotation_x;
    inline static IF::Variable<CameraController, glm::quat> camera_rotation_y;
    inline static IF::Variable<CameraController, II::Transform*> camera_tran;
    inline static IF::Variable<CameraController, II::Camera*> camera;
    inline static IF::Variable<CameraController, BattleRole*> lock_role;

    inline static I::MethodPointer<void, CameraController*> update;

    inline static glm::vec3 camera_pos;
    inline static BattleWorld* battle_world;
    inline static StartGame* start_game;
    inline static CameraController* camera_controller;
    inline static BattleRole* local_role;
    inline static BattleRoleLogic* local_role_logic;
    inline static StatisticsData* local_role_statistics;

    inline static IC* class_;
    inline static std::shared_mutex rw_lock;
    inline static float update_time;

    CameraController();
    ~CameraController();

    static auto update_hook(CameraController* _i) -> void try {
        HardBreakPoint::call_origin(update_hook, _i);
        camera_controller = _i;

        tg_.update_start();
        const auto w2c = W2C::instance();
        w2c->wait();
        w2c->clear();

        BattleRole::roles.clear();

        try {
            if (util::is_bad_ptr(camera_controller)) {
                return;
            }

            const auto trans = camera_tran[camera_controller];
            if (util::is_bad_ptr(trans)) {
                return;
            }

            camera_pos = trans->GetPosition();

            const auto camera_ptr = camera[camera_controller];
            if (util::is_bad_ptr(camera_ptr)) {
                camera_controller = nullptr;
                return;
            }

            const auto view = glm::inverse(camera_ptr->CameraToWorldMatrix());
            w2c->camera_matrix = camera_ptr->GetProjectionMatrix() * view;
            w2c->screen_size = DXHook::size;

            local_role = lock_role[camera_controller];
            if (util::is_bad_ptr(local_role)) {
                camera_controller = nullptr;
                return;
            }

            local_role_logic = BattleRole::role_logic[local_role];
            if (util::is_bad_ptr(local_role_logic)) {
                camera_controller = nullptr;
                return;
            }

            local_role_statistics = BattleRoleLogic::statistics_data[local_role_logic];
            if (util::is_bad_ptr(local_role_statistics)) {
                camera_controller = nullptr;
                return;
            }

            battle_world = StatisticsData::game_world[local_role_statistics];
            if (util::is_bad_ptr(battle_world)) {
                camera_controller = nullptr;
                return;
            }

            start_game = BattleWorld::start_game[battle_world];
            BattleRole::get_all();
        } catch (...) {
            camera_controller = nullptr;
            return;
        }

        if (BattleRole::roles.empty()) {
            camera_controller = nullptr;
            return;
        }

        FeatureBase::instance().update();
        w2c->start();
        std::unique_lock lock(rw_lock);
        update_time = tg_.get_duration<std::chrono::milliseconds>();
    } catch (...) {
        HardBreakPoint::call_origin(update_hook, _i);
        MessageBoxA(DXHook::hwnd, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
    }

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("CameraController");
        camera_rotation_x.Init(class_->Get<IF>("MyCameraRotationX"));
        camera_rotation_y.Init(class_->Get<IF>("MyCameraRotationY"));
        camera_tran.Init(class_->Get<IF>("MyCameraTran"));
        lock_role.Init(class_->Get<IF>("LockRole"));
        camera.Init(class_->Get<IF>("MyCamera"));

        class_->Get<IM>("Update")->Cast(update);
        HardBreakPoint::set_break_point(update, update_hook);
    }
};
