#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/RoleAIHit/RoleAIHit.h"

class HitPart final : public ClassRegistrar<HitPart> {
public:
    inline static IC* class_;
    inline static I::MethodPointer<void, HitPart*, RoleAIHit*> hit_role_ai;

    HitPart();
    ~HitPart();

    auto api_hit_role_ai(RoleAIHit* _role_ai_hit) -> void {
        if (util::is_bad_ptr(_role_ai_hit) || util::is_bad_ptr(this)) {
            return;
        }

        return hit_role_ai(this, _role_ai_hit);
    }

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("HitPart");
        class_->Get<IM>("HitRoleAI")->Cast(hit_role_ai);
    }
};
