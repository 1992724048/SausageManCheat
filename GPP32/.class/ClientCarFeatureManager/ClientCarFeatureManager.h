#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/AnimatorControl/AnimatorControl.h"
#include ".class/Car/Car.h"

class ClientCarFeatureManager final : public ClassRegistrar<ClientCarFeatureManager> {
public:
    inline static IF::Variable<ClientCarFeatureManager, II::List<Car*>*> show_car_list;

    inline static IC* class_;

    ClientCarFeatureManager();
    ~ClientCarFeatureManager();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("ClientCarFeatureManager");
        show_car_list.Init(class_->Get<IF>("showCarList"));
    }
};
