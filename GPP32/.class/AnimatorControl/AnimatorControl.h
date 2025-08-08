#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class AnimatorControl final : public II::MonoBehaviour, public ClassRegistrar<AnimatorControl> {
public:
    inline static IF::Variable<AnimatorControl, II::Animator*> animator;
    inline static IF::Variable<AnimatorControl, II::Transform*> head;

    inline static IC* class_;

    AnimatorControl();
    ~AnimatorControl();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("AnimatorControl");
        animator.Init(class_->Get<IF>("animator"));
        head.Init(class_->Get<IF>("Headring"));
    }
};
