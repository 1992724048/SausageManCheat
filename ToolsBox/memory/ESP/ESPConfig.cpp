#include "ESPConfig.h"

#include "fetures/ESP/ESP.h"

ESPConfig::ESPConfig() {}

auto ESPConfig::config_callback() -> void {
    auto lock = mutex();
    const auto i = ESP::instance();
    enable = i->f_enable;
    show_ai = i->f_show_ai;
    show_box = i->f_show_box;
    show_role = i->f_show_role;
    show_info = i->f_show_info;
    show_bone = i->f_show_bone;
    show_team = i->f_show_team;
    show_item = i->f_show_item;
    show_cars = i->f_show_cars;
}
