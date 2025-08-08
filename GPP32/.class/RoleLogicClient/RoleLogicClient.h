#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class BattleRole;

class RoleLogicClient final : public ClassRegistrar<RoleLogicClient> {
public:
    inline static IF::Variable<RoleLogicClient, BattleRole*> client;

    inline static IC* class_;

    RoleLogicClient();
    ~RoleLogicClient();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("$w");
        client.Init(class_->Get<IF>("$P"));
    }
};
