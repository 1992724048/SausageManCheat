#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/RoleAILogic/RoleAILogic.h"

class RoleAIManager final : public ClassRegistrar<RoleAIManager> {
public:
    inline static IF::Variable<RoleAIManager, II::List<RoleAILogic*>*> logic_list;

    inline static IC* class_;

    RoleAIManager();
    ~RoleAIManager();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("RoleAIManager");
        logic_list.Init(class_->Get<IF>("logicList"));
    }
};
