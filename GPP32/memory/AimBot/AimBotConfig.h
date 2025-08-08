#pragma once
#include "../SharedMemory.h"

#pragma pack(push, 4)
class AimBotConfig final : public SharedMemoryRegistrar<AimBotConfig> {
public:
    char space[4];
    bool enable;
    double speed;
    double rect_w;
    double rect_h;
    int hotkey;

    AimBotConfig();
    ~AimBotConfig() = default;
};
#pragma pack(pop)