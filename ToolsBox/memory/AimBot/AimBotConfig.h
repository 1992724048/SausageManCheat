#pragma once
#include "../SharedMemory.h"

#pragma pack(push, 4)
class AimBotConfig final : public SharedMemoryRegistrar<AimBotConfig> {
public:
    bool enable;
    double speed;
    double rect_w;
    double rect_h;
    int hotkey;
    bool random;

    AimBotConfig();
    ~AimBotConfig() = default;
    auto config_callback() -> void override;
};
#pragma pack(pop)