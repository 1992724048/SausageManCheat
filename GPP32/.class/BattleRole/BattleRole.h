#pragma once
#include ".class/ClassRegistrar.h"
#include ".class/BattleRoleLogic/BattleRoleLogic.h"
#include ".class/RoleControl/RoleControl.h"
#include ".class/WeaponControl/WeaponControl.h"

class BattleRole final : public II::MonoBehaviour, public ClassRegistrar<BattleRole> {
public:
    inline static IF::Variable<BattleRole, II::Transform*> pos;
    inline static IF::Variable<BattleRole, BattleRoleLogic*> role_logic;
    inline static IF::Variable<BattleRole, RoleControl*> role_control;
    inline static IF::Variable<BattleRole, bool> is_clear;
    inline static IF::Variable<BattleRole, WeaponControl*> user_weapon;
    inline static IF::Variable<BattleRole, II::GameObject*> game_object;

    inline static IC* class_;
    inline static std::vector<BattleRole*, mi_stl_allocator<BattleRole*>> roles;

    static auto get_all() -> void;

    BattleRole();
    ~BattleRole();

    auto init() -> void override {
        class_ = I::Get("Assembly-CSharp.dll")->Get("BattleRole");
        pos.Init(class_->Get<IF>("BuffBody"));
        role_logic.Init(class_->Get<IF>("$a"));
        role_control.Init(class_->Get<IF>("MyRoleControl"));
        is_clear.Init(class_->Get<IF>("IsClear"));
        user_weapon.Init(class_->Get<IF>("UserWeapon"));
        game_object.Init(class_->Get<IF>("LockTarget"));
        game_object.offset -= sizeof(glm::vec3);
        game_object.offset -= sizeof(void*);
    }
};
