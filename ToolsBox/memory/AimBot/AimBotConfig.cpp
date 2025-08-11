#include "AimBotConfig.h"

#include "fetures/AimBot/AimBot.h"

AimBotConfig::AimBotConfig() {
   
}

auto AimBotConfig::config_callback() -> void {
    auto lock = mutex();
    const auto i = AimBot::instance();
    enable = i->f_enable;
    speed = i->f_speed;
    rect_h = i->f_rect_h;
    rect_w = i->f_rect_w;
    hotkey = i->f_hotkey;
    random = i->f_random;
}
