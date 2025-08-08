#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/AnimatorControl/AnimatorControl.h"

class PickItemNet final : public ClassRegistrar<PickItemNet> {
public:
    inline static IF::Variable<PickItemNet, int> num;
    inline static IF::Variable<PickItemNet, float> equip_hp;
    inline static IF::Variable<PickItemNet, float> base_equip_hp;


    inline static IC* class_;

    PickItemNet();
    ~PickItemNet();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("PickItemNet");
        num.Init(class_->Get<IF>("BaseValue"));
        equip_hp.Init(class_->Get<IF>("EquipHp"));
        base_equip_hp.Init(class_->Get<IF>("BaseEquipHp"));
    }
};
