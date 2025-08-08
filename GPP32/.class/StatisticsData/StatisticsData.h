#pragma once
#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/BattleWorld/BattleWorld.h"

class StatisticsData final : public ClassRegistrar<StatisticsData> {
public:
    inline static IF::Variable<StatisticsData, BattleWorld*> game_world;

    inline static IC* class_;

    StatisticsData();
    ~StatisticsData();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("StatisticsData");
        game_world.Init(class_->Get<IF>("mGameWorld"));
    }
};
