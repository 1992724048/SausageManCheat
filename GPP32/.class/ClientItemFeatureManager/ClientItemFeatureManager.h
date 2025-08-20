#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/AnimatorControl/AnimatorControl.h"
#include ".class/PickItem/PickItem.h"

class ClientItemFeatureManager final : public ClassRegistrar<ClientItemFeatureManager> {
public:
    inline static IF::Variable<ClientItemFeatureManager, II::Dictionary<int, II::List<PickItem*>*>*> pick_items_point_data;
    inline static IF::Variable<ClientItemFeatureManager, II::List<II::List<PickItem*>*>*> event_show_items;

    inline static IC* class_;

    ClientItemFeatureManager();
    ~ClientItemFeatureManager();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("ClientItemFeatureManager");
        pick_items_point_data.Init(class_->Get<IF>("pickItemsPointData"));
        event_show_items.Init(class_->Get<IF>("eventShowItems"));
    }
};
