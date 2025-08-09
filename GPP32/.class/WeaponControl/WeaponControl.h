#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/SOWeaponControl/SOWeaponControl.h"

class WeaponControl final : public ClassRegistrar<WeaponControl> {
public:
    inline static IF::Variable<WeaponControl, SOWeaponControl*> so_weapon_control;

    inline static IC* class_;

    WeaponControl();
    ~WeaponControl();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("WeaponControl");
        so_weapon_control.Init(class_->Get<IF>("MySOWeaponControl"));
    }
};
