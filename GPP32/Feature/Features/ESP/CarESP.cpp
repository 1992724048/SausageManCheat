#include "CarESP.h"

#include <execution>

#include ".class/CameraController/CameraController.h"

#include "memory/ESP/ESPConfig.h"
#include "memory/W2C/W2C.h"

CarEsp::CarEsp() = default;

auto CarEsp::render() -> void try {
    std::vector<Car, mi_stl_allocator<Car>> temp;
    {
        std::lock_guard lock_s(mutex);
        temp = std::move(cars);
    }

    if (temp.empty()) {
        return;
    }

    const auto esp = ESPConfig::instance();
    auto lock = esp->mutex();

    if (!esp->enable) {
        return;
    }

    if (!esp->show_cars) {
        return;
    }

    for (auto& [_id, screen_pos, hp, hp_max, oil, oil_max] : temp) {
        auto& [pos, id] = screen_pos;

        if (pos.z < 0.f) {
            continue;
        }

        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        draw_info(bg, pos, _id, hp, hp_max, oil, oil_max);
    }
} catch (...) {
    MessageBoxA(nullptr, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}

auto CarEsp::update() -> void try {
    process_data();

    int size = 0;
    II::List<::Car*>* cars_data = nullptr;
    try {
        if (util::is_bad_ptr(CameraController::battle_world)) {
            return;
        }

        const auto car_manager = BattleWorld::client_car_manager[CameraController::battle_world];
        if (util::is_bad_ptr(car_manager)) {
            return;
        }

        cars_data = ClientCarFeatureManager::show_car_list[car_manager];
        if (util::is_bad_ptr(cars_data)) {
            return;
        }

        size = cars_data->size;
    } catch (...) {}

    if (cars_data == nullptr) {
        return;
    }

    cars_commit.clear();
    cars_commit.resize(size);
    const auto w2c = W2C::instance();

    for (int i = 0; i < size; i++) {
        auto& car_ = cars_commit[i];
        try {
            const auto car = cars_data->operator[](i);
            if (util::is_bad_ptr(car)) {
                continue;
            }

            const auto car_net = ::Car::car_net[car];
            const auto car_transform = ::Car::transform[car];
            const auto car_config = ::Car::so_car_data[car];

            if (util::is_bad_ptr(car_net) || util::is_bad_ptr(car_transform) || util::is_bad_ptr(car_config)) {
                continue;
            }

            const auto car_mirror = CarNet::mirror[car_net];
            if (util::is_bad_ptr(car_mirror)) {
                continue;
            }

            auto pos = car_transform->GetPosition();
            car_.screen_pos.first = pos;
            car_.screen_pos.second = w2c->commit(pos);

            car_.hp = CarNetMirror::sync_hp[car_mirror];
            car_.hp_max = CarNetMirror::sync_max_hp[car_mirror];
            car_.oil = CarNetMirror::sync_oil_consumption[car_mirror];
            car_.oil_max = CarNetMirror::sync_max_oil_consumption[car_mirror];
            car_.name = std::move(::Car::car_data_id[car]->to_string());
        } catch (...) {}
    }
} catch (...) {
    MessageBoxA(nullptr, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}

auto CarEsp::process_data() -> void try {
    if (cars_commit.empty()) {
        return;
    }

    const auto w2c = W2C::instance();
    for (auto& value : cars_commit) {
        value.screen_pos.first = w2c->pos_done[value.screen_pos.second];
    }

    std::lock_guard lock(mutex);
    cars = std::move(cars_commit);
} catch (...) {
    MessageBoxA(nullptr, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}

auto CarEsp::draw_info(ImDrawList* _bg,
                       const std::conditional_t<true, glm::vec<3, float>, int>& _screen_pos,
                       const util::String& _id,
                       const float _hp,
                       const float _hp_max,
                       const float _oil,
                       const float _oil_max) -> void {
    const auto& name = id_to_name[_id];
    constexpr auto color = ImColor(48, 137, 252, 150);
    const ImVec2 name_text_size = ImGui::CalcTextSize(name.data());

    const glm::vec2 size = {name_text_size.x + 2.f * 2.0f, name_text_size.y + 2.f * 2.0f};
    const glm::vec2 min = {_screen_pos.x - size.x / 2.0f, _screen_pos.y - size.y / 2.0f};
    const glm::vec2 max = {_screen_pos.x + size.x / 2.0f, _screen_pos.y + size.y / 2.0f};

    const float height = max.y - min.y;
    const float hp_bar = height - height * (_hp / _hp_max);
    const float oil_bar = height - height * (_oil / _oil_max);

    _bg->AddRect({min.x - 1, min.y - 1}, {max.x + 1, max.y + 1}, ImColor(0.f, 0.f, 0.f, color.Value.w));
    _bg->AddRectFilled({min.x, min.y}, {max.x, max.y}, color);
    _bg->AddRectFilled({max.x, min.y - 1}, {max.x + 12, max.y + 1}, ImColor(0.f, 0.f, 0.f, color.Value.w));

    _bg->AddRectFilled({max.x + 1, min.y}, {max.x + 5, max.y}, ImColor(1.f, 0.f, 0.f, color.Value.w));
    _bg->AddRectFilled({max.x + 7, min.y}, {max.x + 11, max.y}, ImColor(1.f, 0.f, 0.f, color.Value.w));

    _bg->AddRectFilled({max.x + 1, min.y + hp_bar}, {max.x + 5, max.y}, ImColor(0.f, 1.f, 0.f, color.Value.w));
    _bg->AddRectFilled({max.x + 7, min.y + oil_bar}, {max.x + 11, max.y}, ImColor(1.f, 1.f, 0.f, color.Value.w));

    const ImVec2 text_pos = {_screen_pos.x - name_text_size.x / 2.0f, _screen_pos.y - name_text_size.y / 2.0f};
    _bg->AddText(text_pos, ImColor(255, 255, 255), name.data());
}
