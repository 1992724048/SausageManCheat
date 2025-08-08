#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/CarNet/CarNet.h"
#include ".class/SOCarDataConfig/SOCarDataConfig.h"

class Car final : public ClassRegistrar<Car> {
public:
    inline static IF::Variable<Car, II::Transform*> transform;
    inline static IF::Variable<Car, II::String*> car_data_id;
    inline static IF::Variable<Car, CarNet*> car_net;
    inline static IF::Variable<Car, SOCarDataConfig*> so_car_data;

    inline static IC* class_;

    Car();
    ~Car();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("Car");
        transform.Init(class_->Get<IF>("myTransform"));
        car_net.Init(class_->Get<IF>("MyCarNet"));
        so_car_data.Init(class_->Get<IF>("soCarData"));
        car_data_id.Init(class_->Get<IF>("CarDataId"));
    }
};
