#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/AnimatorControl/AnimatorControl.h"
#include ".class/PickItemNet/PickItemNet.h"

class PickItemDataConfig final : public ClassRegistrar<PickItemDataConfig> {
public:
    inline static IF::Variable<PickItemDataConfig, II::String*> item_name;
    inline static IF::Variable<PickItemDataConfig, int64_t> item_id;
    inline static IF::Variable<PickItemDataConfig, int64_t> item_type;
    inline static IF::Variable<PickItemDataConfig, int32_t> item_pick_num_limit;

    inline static IC* class_;

    PickItemDataConfig();
    ~PickItemDataConfig();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("PickItemDataConfig");
        item_name.Init(class_->Get<IF>("itemName"));
        item_id.Init(class_->Get<IF>("ItemId"));
        item_type.Init(class_->Get<IF>("ItemType"));
        item_pick_num_limit.Init(class_->Get<IF>("ItemPickNumLimit"));
    }
};
