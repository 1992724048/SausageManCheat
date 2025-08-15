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

    static auto update_hook(CameraController* _i) -> void {
        HardBreakPoint::call_origin(update_hook, _i);
        camera_controller = _i;
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
