#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/ClientRoleAILogic/ClientRoleAILogic.h"

class RoleAILogic final : public ClassRegistrar<RoleAILogic> {
public:
    inline static IF::Variable<RoleAILogic, II::String*> name;
    inline static IF::Variable<RoleAILogic, glm::vec3> position;
    inline static IF::Variable<RoleAILogic, float> hp;
    inline static IF::Variable<RoleAILogic, float> weak;
    inline static IF::Variable<RoleAILogic, ClientRoleAILogic*> client_logic;

    inline static IC* class_;
    inline static I::MethodPointer<void, RoleAILogic*, glm::vec3> set_pos;

    auto api_set_pos(const glm::vec3& _pos) -> void {
        if (util::is_bad_ptr(this)) {
            return;
        }

        return set_pos(this, _pos);
    }

    RoleAILogic();
    ~RoleAILogic();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("RoleAILogic");
        name.Init(class_->Get<IF>("<Name>k__BackingField"));
        position.Init(class_->Get<IF>("<Position>k__BackingField"));
        hp.Init(class_->Get<IF>("<Hp>k__BackingField"));
        weak.Init(class_->Get<IF>("<WeakValue>k__BackingField"));
        client_logic.Init(class_->Get<IF>("<ClientLogic>k__BackingField"));

        class_->Get<IM>("set_Position")->Cast(set_pos);
    }
};
