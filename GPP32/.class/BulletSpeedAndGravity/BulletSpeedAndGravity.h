#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class BulletSpeedAndGravity final : public ClassRegistrar<BulletSpeedAndGravity> {
public:
    inline static IF::Variable<BulletSpeedAndGravity, float> gravity;
    inline static IF::Variable<BulletSpeedAndGravity, float> bullet_speed;
    inline static IF::Variable<BulletSpeedAndGravity, float> fly_time;

    inline static IC* class_;

    BulletSpeedAndGravity();
    ~BulletSpeedAndGravity();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("BulletSpeedAndGravity");
        gravity.Init(class_->Get<IF>("gravity"));
        bullet_speed.Init(class_->Get<IF>("bulletSpeed"));
        fly_time.Init(class_->Get<IF>("flyTime"));
    }
};
