#pragma once
#include <pch.h>

#include "FeatureRegistrar.h"

class CarEsp final : public FeatureRegistrar<CarEsp> {
public:
    CarEsp();

    struct Car {
        util::String name;
        std::pair<glm::vec3, int> screen_pos;
        float hp;
        float hp_max;
        float oil;
        float oil_max;
    };

    std::mutex mutex;
    std::vector<Car, mi_stl_allocator<Car>> cars;
    std::vector<Car, mi_stl_allocator<Car>> cars_commit;

    static auto draw_info(ImDrawList* _bg, const std::conditional_t<true, glm::vec<3, float>, int>& _screen_pos, const util::String& _id, float _hp, float _hp_max, float _oil, float _oil_max) -> void;
    auto render() -> void override;
    auto update() -> void override;

    auto process_data() -> void;

    inline static phmap::flat_hash_map<util::String, util::String> id_to_name{
        {"HoverBoard", (const char*)u8"悬浮滑板"},
        {"ArmoredBus", (const char*)u8"装甲巴士"},
        {"UFO", (const char*)u8"小飞碟"},
        {"Machine_Carrier", (const char*)u8"机甲"},
        {"PonyVehicle", (const char*)u8"小马"},
        {"JetCar", (const char*)u8"飞车"},
        {"Jeep", (const char*)u8"吉普车"},
        {"Buggy", (const char*)u8"蹦蹦车"},
        {"TRexKing", (const char*)u8"霸王龙"},
        {"Dragon", (const char*)u8"呆呆龙"},
        {"Peterosaur", (const char*)u8"翼龙"},
        {"Triceratops", (const char*)u8"憨憨龙"},
        {"Raptors", (const char*)u8"奔奔龙"},
        {"Kayak", (const char*)u8"快艇"},
        {"FlyingBroom", (const char*)u8"飞行扫帚"},
        {"SwordTiger", (const char*)u8"剑齿虎"},
    };
};
