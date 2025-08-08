#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class BattleRoleLogic;

class StartGame final : public ClassRegistrar<StartGame> {
public:
    inline static IF::Variable<StartGame, II::List<BattleRoleLogic*>*> role_list;

    inline static IC* class_;

    StartGame();
    ~StartGame();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("StartGame");
        role_list.Init(class_->Get<IF>("RoleList"));
    }
};
