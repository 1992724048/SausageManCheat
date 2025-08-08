#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class RoleAILogic final : public ClassRegistrar<RoleAILogic> {
public:
    inline static IF::Variable<RoleAILogic, II::String*> name;
    inline static IF::Variable<RoleAILogic, glm::vec3> position;
    inline static IF::Variable<RoleAILogic, float> hp;
    inline static IF::Variable<RoleAILogic, float> weak;

    inline static IC* class_;

    RoleAILogic();
    ~RoleAILogic();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("RoleAILogic");
        name.Init(class_->Get<IF>("<Name>k__BackingField"));
        position.Init(class_->Get<IF>("<Position>k__BackingField"));
        hp.Init(class_->Get<IF>("<Hp>k__BackingField"));
        weak.Init(class_->Get<IF>("<WeakValue>k__BackingField"));
    }
};
