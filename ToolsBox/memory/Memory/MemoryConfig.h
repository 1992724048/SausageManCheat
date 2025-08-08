#pragma once
#include "../SharedMemory.h"

#pragma pack(push, 4)
class MemoryConfig final : public SharedMemoryRegistrar<MemoryConfig> {
public:
    bool bullet_tracking;
    bool all_hit_head;
    bool damage_multi;
    int damage_multi_value;
    bool lock_role;
    bool all_gun_auto;
    bool ballistics_tracking;

    MemoryConfig();
    ~MemoryConfig() = default;
    auto config_callback() -> void override;
};
#pragma pack(pop)