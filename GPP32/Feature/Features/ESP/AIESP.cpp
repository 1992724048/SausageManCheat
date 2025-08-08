#include "AIESP.h"

#include <execution>

#include ".class/CameraController/CameraController.h"

#include "memory/ESP/ESPConfig.h"

AIESP::AIESP() {}

auto AIESP::render() -> void {
    std::vector<AI, mi_stl_allocator<AI>> temp;
    {
        std::lock_guard lock_s(mutex);
        temp = std::move(ais);
    }

    const auto esp = ESPConfig::instance();
    auto lock = esp->mutex();

    if (!esp->enable) {
        return;
    }

    if (!esp->show_ai) {
        return;
    }

    for (auto& [name, hp, weak, screen_pos, pos_] : temp) {
        auto& [pos, id] = screen_pos;

        if (pos.z < 0.f) {
            continue;
        }

        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        draw_info(bg, screen_pos, hp, weak, name);
    }
}

auto AIESP::update() -> void {
    process_data();

    {
        const auto esp = ESPConfig::instance();
        auto lock = esp->mutex();

        if (!esp->enable) {
            return;
        }

        if (!esp->show_ai) {
            return;
        }
    }

    int size = 0;
    II::List<RoleAILogic*>* ais_data = nullptr;
    try {
        if (util::is_bad_ptr(CameraController::battle_world)) {
            return;
        }

        const auto ai_manager = BattleWorld::role_ai_manager[CameraController::battle_world];
        if (util::is_bad_ptr(ai_manager)) {
            return;
        }

        ais_data = RoleAIManager::logic_list[ai_manager];
        if (util::is_bad_ptr(ais_data)) {
            return;
        }

        size = ais_data->size;
    } catch (...) {}

    if (ais_data == nullptr) {
        return;
    }

    ais_commit.clear();
    ais_commit.reserve(size);
    const auto w2c = W2C::instance();

    for (int i = 0; i < size; i++) {
        try {
            const auto ai = ais_data->operator[](i);
            if (util::is_bad_ptr(ai)) {
                continue;
            }

            std::pair<glm::vec3, int> pos_;
            auto pos = RoleAILogic::position[ai];
            pos_.first = pos;
            pos_.second = w2c->commit(pos);

            float hp = RoleAILogic::hp[ai];
            float weak = RoleAILogic::weak[ai];

            util::String name = std::move(RoleAILogic::name[ai]->ToString());
            if (name.empty()) {
                continue;
            }

            ais_commit.emplace_back(std::move(name), hp, weak, pos_, pos);
        } catch (...) {}
    }
}

auto AIESP::process_data() -> void {
    std::vector<AI, mi_stl_allocator<AI>> temp = std::move(ais_commit);
    if (temp.empty()) {
        return;
    }

    const auto w2c = W2C::instance();
    for (auto& value : temp) {
        value.screen_pos.first = w2c->pos_done[value.screen_pos.second];
    }


    std::lock_guard lock(mutex);
    ais = std::move(temp);
}

auto AIESP::draw_info(ImDrawList* _bg, std::pair<glm::vec3, int> _screen_pos_, float& _hp, float& _weak, const util::String& _name) -> void {
    auto& [_screen_pos, _id] = _screen_pos_;

    constexpr ImColor color(0.f, 0.f, 0.f, 0.5f);
    const ImVec2 name_text_size = ImGui::CalcTextSize(_name.data());

    glm::vec2 size(125, 25);
    size /= 2;

    const glm::vec2 min = {_screen_pos.x - size.x, _screen_pos.y - size.y};
    const glm::vec2 max = {_screen_pos.x + size.x, _screen_pos.y + size.y};

    const glm::vec2 min_ = {min.x + size.y * 2, min.y};
    const glm::vec2 max_ = {min_.x + 6, max.y};

    const float h = max.y - min.y;
    const float hp_bar = h - h * (_hp / 100.f);
    const float weak_bar = h - h * (_weak / 100.f);

    _bg->AddRect({min.x - 1, min.y - 1}, {max.x + 1, max.y + 1}, ImColor(0.f, 0.f, 0.f, color.Value.w + 0.5f));
    _bg->AddRectFilled({min.x, min.y}, {max.x, max.y}, color);
    _bg->AddRectFilled({min.x, min.y}, {min_.x, max.y}, ImColor(color.Value.x, color.Value.y, color.Value.z, 1.f));

    _bg->AddRectFilled({min_.x, min_.y}, {max_.x, max_.y}, ImColor(0.f, 0.f, 0.f, color.Value.w));
    _bg->AddRectFilled({min_.x + 1, min_.y + weak_bar}, {max_.x - 1, max_.y}, ImColor(1.f, 0.f, 0.f, color.Value.w));
    _bg->AddRectFilled({min_.x + 1, min_.y + hp_bar}, {max_.x - 1, max_.y}, ImColor(0.f, 1.f, 0.f, color.Value.w));

    const std::string team = std::to_string(0);
    const ImVec2 team_text_size = ImGui::CalcTextSize(team.data());

    const ImVec2 box_center = {(min.x + min_.x) / 2.0f, (min.y + max.y) / 2.0f};
    const ImVec2 team_text_pos = {box_center.x - team_text_size.x / 2.0f, box_center.y - team_text_size.y / 2.0f};

    const float name_text_y = (min.y + max.y) / 2.0f - name_text_size.y / 2.0f;
    _bg->AddText({max_.x + 5, name_text_y}, ImColor(255, 255, 255), _name.data());
    _bg->AddText(team_text_pos, ImColor(255, 255, 255), team.data());
}
