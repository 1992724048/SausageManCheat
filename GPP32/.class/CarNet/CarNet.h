#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/CarNetMirror/CarNetMirror.h"

class CarNet final : public ClassRegistrar<CarNet> {
public:
    inline static IF::Variable<CarNet, CarNetMirror*> mirror;

    inline static IC* class_;

    CarNet();
    ~CarNet();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("CarNet");
        mirror.Init(class_->Get<IF>("mirror"));
    }
};
