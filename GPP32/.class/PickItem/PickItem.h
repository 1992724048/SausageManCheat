#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/AnimatorControl/AnimatorControl.h"
#include ".class/PickItemDataConfig/PickItemDataConfig.h"
#include ".class/PickItemNet/PickItemNet.h"

class PickItem final : public ClassRegistrar<PickItem> {
public:
    inline static IF::Variable<PickItem, PickItemNet*> my_pick_item_net;
    inline static IF::Variable<PickItem, PickItemDataConfig*> pick_item_data_config;
    inline static IF::Variable<PickItem, glm::vec3> pos;

    inline static IC* class_;

    PickItem();
    ~PickItem();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("PickItem");
        my_pick_item_net.Init(class_->Get<IF>("MyPickItemNet"));
        pick_item_data_config.Init(class_->Get<IF>("$a"));
        pos.Init(class_->Get<IF>("$E"));
    }
};
