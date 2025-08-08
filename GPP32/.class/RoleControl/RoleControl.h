#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/AnimatorControl/AnimatorControl.h"

class RoleControl final : public II::MonoBehaviour, public ClassRegistrar<RoleControl> {
public:
    inline static IF::Variable<RoleControl, AnimatorControl*> animator_control;
    inline static IF::Variable<RoleControl, float> y_cam_rot;
    inline static IF::Variable<RoleControl, float> x_cam_rot;

    inline static IC* class_;

    RoleControl();
    ~RoleControl();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("RoleControl");
        animator_control.Init(class_->Get<IF>("animatorControl"));
        y_cam_rot.Init(class_->Get<IF>("y_camRot"));
        x_cam_rot.Init(class_->Get<IF>("x_camRot"));
    }
};
