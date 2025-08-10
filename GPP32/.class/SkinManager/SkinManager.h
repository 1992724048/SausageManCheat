#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"

class SkinManager final : public II::MonoBehaviour, public ClassRegistrar<SkinManager> {
public:
    inline static IF::Variable<SkinManager, void*> root_bone_data;

    inline static IC* class_;

    SkinManager();
    ~SkinManager();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("SkinManager");
        root_bone_data.Init(class_->Get<IF>("rootBoneData"));
    }
};
