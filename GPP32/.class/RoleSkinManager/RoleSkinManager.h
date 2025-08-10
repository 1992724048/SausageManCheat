#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/SkinManager/SkinManager.h"

class RoleSkinManager final : public II::MonoBehaviour, public ClassRegistrar<RoleSkinManager> {
public:
    inline static IF::Variable<RoleSkinManager, SkinManager*> base_skin_manager;

    inline static IC* class_;

    RoleSkinManager();
    ~RoleSkinManager();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("RoleSkinManager");
        base_skin_manager.Init(class_->Get<IF>("baseSkinManager"));
    }
};
