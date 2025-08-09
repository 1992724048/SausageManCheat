#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/BulletSpeedAndGravity/BulletSpeedAndGravity.h"

class SOWeaponControl final : public ClassRegistrar<SOWeaponControl> {
public:
    inline static IF::Variable<SOWeaponControl, bool> has_auto_fire;
    inline static IF::Variable<SOWeaponControl, II::Array<BulletSpeedAndGravity*>*> bullet_speed_and_gravity;

    inline static IC* class_;

    SOWeaponControl();
    ~SOWeaponControl();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("SOWeaponControl");
        has_auto_fire.Init(class_->Get<IF>("HasAutoFire"));
        bullet_speed_and_gravity.Init(class_->Get<IF>("bulletSpeedAndGravity"));
    }
};
