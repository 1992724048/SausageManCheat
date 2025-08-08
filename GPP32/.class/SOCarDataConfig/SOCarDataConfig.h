#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class SOCarDataConfig final : public ClassRegistrar<SOCarDataConfig> {
public:
    // inline static IF::Variable<SOCarDataConfig, II::List<Car*>*> show_car_list;

    inline static IC* class_;

    SOCarDataConfig();
    ~SOCarDataConfig();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("SOCarDataConfig");
        // show_car_list.Init(class_->Get<IF>("showCarList"));
    }
};
