#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/ClientCarFeatureManager/ClientCarFeatureManager.h"
#include ".class/ClientItemFeatureManager/ClientItemFeatureManager.h"
#include ".class/RoleAIManager/RoleAIManager.h"
#include ".class/StartGame/StartGame.h"

class BattleWorld final : public ClassRegistrar<BattleWorld> {
public:
    inline static IF::Variable<BattleWorld, StartGame*> start_game;
    inline static IF::Variable<BattleWorld, ClientItemFeatureManager*> item_client_manager;
    inline static IF::Variable<BattleWorld, ClientCarFeatureManager*> client_car_manager;
    inline static IF::Variable<BattleWorld, RoleAIManager*> role_ai_manager;

    inline static IC* class_;

    BattleWorld();
    ~BattleWorld();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("BattleWorld");
        start_game.Init(class_->Get<IF>("startGame"));
        item_client_manager.Init(class_->Get<IF>("ItemClientManager"));
        client_car_manager.Init(class_->Get<IF>("ClientCarManager"));
        role_ai_manager.Init(class_->Get<IF>("RoleAIManager"));
    }
};
