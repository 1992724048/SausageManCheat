#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class RoleAIHit final : public ClassRegistrar<RoleAIHit> {
public:
    inline static IF::Variable<RoleAIHit, II::String*> name;

    inline static IC* class_;

    RoleAIHit();
    ~RoleAIHit();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("RoleAIHit");
        // name.Init(class_->Get<IF>("<Name>k__BackingField"));
    }
};
