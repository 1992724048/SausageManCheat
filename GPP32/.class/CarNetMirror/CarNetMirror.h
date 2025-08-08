#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class CarNetMirror final : public ClassRegistrar<CarNetMirror> {
public:
    inline static IF::Variable<CarNetMirror, float> sync_hp;
    inline static IF::Variable<CarNetMirror, float> sync_max_hp;
    inline static IF::Variable<CarNetMirror, float> sync_oil_consumption;
    inline static IF::Variable<CarNetMirror, float> sync_max_oil_consumption;

    inline static IC* class_;

    CarNetMirror();
    ~CarNetMirror();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("CarNetMirror");
        sync_hp.Init(class_->Get<IF>("SyncHp"));
        sync_max_hp.Init(class_->Get<IF>("SyncMaxHp"));
        sync_oil_consumption.Init(class_->Get<IF>("SyncOilConsumption"));
        sync_max_oil_consumption.Init(class_->Get<IF>("SyncMaxOilConsumption"));
    }
};
