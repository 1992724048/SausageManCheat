#pragma once
#include "../SharedMemory.h"

#pragma pack(push, 4)
class ESPConfig final : public SharedMemoryRegistrar<ESPConfig> {
public:
    char space[4];
    bool enable;
    bool show_ai;
    bool show_box;
    bool show_role;
    bool show_info;
    bool show_bone;
    bool show_team;
    bool show_item;
    bool show_cars;

    ESPConfig();
    ~ESPConfig() = default;
};
#pragma pack(pop)