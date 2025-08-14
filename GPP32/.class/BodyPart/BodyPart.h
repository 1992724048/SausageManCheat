#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

enum class BodyPartData {
    None,
    Head,
    Face,
    Body,
    LeftHand,
    RightHand,
    Hip,
    LeftLeg,
    RightLeg,
    Foot,
    BodyEffect,
    WeakHead,
    Role,
    WordPoint,
    LockTarget,
    CatLockTarget,
    LHand,
    RHand
};

class BodyPart final : public ClassRegistrar<BodyPart> {
public:
    inline static IF::Variable<BodyPart, BodyPartData> body_type;

    inline static IC* class_;

    BodyPart();
    ~BodyPart();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("BodyPart", "*", "HitType");
        body_type.Init(class_->Get<IF>("BodyType"));
    }
};
