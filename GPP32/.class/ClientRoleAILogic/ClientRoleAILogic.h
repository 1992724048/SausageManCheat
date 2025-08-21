#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/RoleAIHit/RoleAIHit.h"

class ClientRoleAILogic final : public ClassRegistrar<ClientRoleAILogic> {
public:
    inline static IF::Variable<ClientRoleAILogic, II::List<RoleAIHit*>*> role_ai_hits;

    inline static IC* class_;

    ClientRoleAILogic();
    ~ClientRoleAILogic();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("ClientRoleAILogic");
        role_ai_hits.Init(class_->Get<IF>("roleAIHits"));
    }
};
