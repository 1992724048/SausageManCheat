#pragma once

#include "pch.h"
#include ".class/ClassRegistrar.h"
#include ".class/RoleLogicClient/RoleLogicClient.h"
#include ".class/StatisticsData/StatisticsData.h"

class BattleRoleLogic final : public II::MonoBehaviour, public ClassRegistrar<BattleRoleLogic> {
public:
    inline static IF::Variable<BattleRoleLogic, float> hp;
    inline static IF::Variable<BattleRoleLogic, II::String*> name;
    inline static IF::Variable<BattleRoleLogic, int64_t> team;
    inline static IF::Variable<BattleRoleLogic, float> weak;
    inline static IF::Variable<BattleRoleLogic, bool> local;
    inline static IF::Variable<BattleRoleLogic, StatisticsData*> statistics_data;
    inline static IF::Variable<BattleRoleLogic, RoleLogicClient*> client;
    inline static IF::Variable<BattleRoleLogic, II::List<PickItemNet*>*> get_weapons;

    inline static I::MethodPointer<BattleRole*, BattleRoleLogic*> get_role;
    inline static IC* class_;

    auto api_get_role() -> BattleRole* {
        return get_role(this);
    }

    BattleRoleLogic();
    ~BattleRoleLogic();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("BattleRoleLogic");
        hp.Init(class_->Get<IF>("Power"));
        hp.offset += sizeof(float);
        name.Init(class_->Get<IF>("NickName"));
        weak.Init(class_->Get<IF>("WeakValue"));
        team.Init(class_->Get<IF>("TeamNum"));
        local.Init(class_->Get<IF>("deadShipDiffPos"));
        local.offset += sizeof(glm::vec3);
        statistics_data.Init(class_->Get<IF>("$B"));
        client.Init(class_->Get<IF>("teammateConfig"));
        client.offset -= sizeof(void*);
        get_weapons.Init(class_->Get<IF>("GetWeapons"));

        for (auto& method : class_->methods) {
            if (I::name_map[method.return_type->name] == "BattleRole") {
                method.Cast(get_role);
            }
        }
    }
};
