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
    inline static IF::Variable<CameraController, glm::quat> camera_rotation_x;
    inline static IF::Variable<CameraController, glm::quat> camera_rotation_y;
    inline static IF::Variable<CameraController, II::Transform*> camera_tran;
    inline static IF::Variable<CameraController, II::Camera*> camera;
    inline static IF::Variable < CameraController, BattleRole*> lock_role;

    inline static I::MethodPointer<void, CameraController*> update;

    inline static glm::vec3 camera_pos;
    inline static BattleWorld* battle_world;
    inline static StartGame* start_game;
    inline static CameraController* camera_controller;

    inline static IC* class_;

    CameraController();
    ~CameraController();

    static auto update_hook(CameraController* _i) -> void {
        HardBreakPoint::call_origin(update_hook, _i);

        const auto w2c = W2C::instance();
        w2c->wait();
        w2c->clear();

        camera_controller = _i;
        BattleRole::roles.clear();

        LOG_DEBUG << "update_hook is call";

        try {
            const auto trans = camera_tran[camera_controller];
            if (util::is_bad_ptr(trans)) {
                LOG_DEBUG << "trans is null";
                camera_pos = trans->GetPosition();
                return;
            }

            const auto camera_ptr = camera[_i];
            if (util::is_bad_ptr(camera_ptr)) {
                LOG_DEBUG << "camera_ptr is null";
                camera_controller = nullptr;
                return;
            }

            const auto view = glm::inverse(camera_ptr->CameraToWorldMatrix());
            w2c->camera_matrix = camera_ptr->GetProjectionMatrix() * view;
            w2c->screen_size = DXHook::size;

            const auto local = lock_role[camera_controller];
            if (util::is_bad_ptr(local)) {
                LOG_DEBUG << "local is null";
                camera_controller = nullptr;
                return;
            }

            const auto role_logic = BattleRole::role_logic[local];
            if (util::is_bad_ptr(role_logic)) {
                LOG_DEBUG << "role_logic is null";
                camera_controller = nullptr;
                return;
            }

            const auto data = BattleRoleLogic::statistics_data[role_logic];
            if (util::is_bad_ptr(data)) {
                LOG_DEBUG << "data is null";
                camera_controller = nullptr;
                return;
            }

            battle_world = StatisticsData::game_world[data];
            if (util::is_bad_ptr(battle_world)) {
                LOG_DEBUG << "battle_world is null";
                camera_controller = nullptr;
                return;
            }

            LOG_DEBUG << "get start_game";
            start_game = BattleWorld::start_game[battle_world];
            LOG_DEBUG << "start_game is not null";
            BattleRole::get_all();
            LOG_DEBUG << "roles: " << BattleRole::roles.size();
        } catch (...) {
            LOG_DEBUG << "process is error";
            camera_controller = nullptr;
            return;
        }

        if (BattleRole::roles.empty()) {
            LOG_DEBUG << "roles is null";
            camera_controller = nullptr;
            return;
        }

        LOG_DEBUG << "update is call";

        FeatureBase::instance().update();
        w2c->start();
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
