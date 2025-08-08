#pragma once
#include <pch.h>

#include "FeatureRegistrar.h"

class AIESP final : public FeatureRegistrar<AIESP> {
public:
    AIESP();

    struct AI {
        util::String name;
        float hp;
        float weak;
        std::pair<glm::vec3, int> screen_pos;
        glm::vec3 pos;
    };

    std::mutex mutex;
    std::vector<AI, mi_stl_allocator<AI>> ais;
    std::vector<AI, mi_stl_allocator<AI>> ais_commit;

    static auto draw_info(ImDrawList* _bg, std::pair<glm::vec3, int> _screen_pos_, float& _hp, float& _weak, const util::String& _name) -> void;
    auto render() -> void override;
    auto update() -> void override;

    auto process_data() -> void;
};
