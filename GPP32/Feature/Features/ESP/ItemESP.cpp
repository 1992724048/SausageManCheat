#include "ItemESP.h"

#include <execution>

#include ".class/CameraController/CameraController.h"
#include "memory/ESP/ESPConfig.h"

ItemEsp::ItemEsp() = default;

auto ItemEsp::render() -> void try {
    std::vector<Item, mi_stl_allocator<Item>> temp;
    {
        std::lock_guard lock_s(mutex);
        temp = std::move(items);
    }

    if (temp.empty()) {
        return;
    }

    const auto esp = ESPConfig::instance();
    auto lock = esp->mutex();

    if (!esp->enable) {
        return;
    }

    if (!esp->show_item) {
        return;
    }

    for (auto& [name, screen_pos, item_id, type, num] : temp) {
        auto& [pos, id] = screen_pos;

        if (pos.z < 0.f) {
            continue;
        }

        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        draw_info(bg, pos, name, item_id, type, num);
    }
} catch (...) {
    MessageBoxA(nullptr, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}

auto ItemEsp::update() -> void try {
    process_data();

    int size = 0;
    II::Dictionary<int, UnityResolve::UnityType::List<PickItem*>*>* items_data = nullptr;

    try {
        if (util::is_bad_ptr(CameraController::battle_world)) {
            return;
        }

        const auto item_manager = BattleWorld::item_client_manager[CameraController::battle_world];
        if (util::is_bad_ptr(item_manager)) {
            return;
        }

        items_data = ClientItemFeatureManager::pick_items_point_data[item_manager];
        if (util::is_bad_ptr(items_data)) {
            return;
        }

        size = items_data->iCount;
    } catch (...) {}

    if (items_data == nullptr) {
        return;
    }

    const auto w2c = W2C::instance();
    items_commit.clear();
    items_commit.reserve(size * 10);

    std::vector<std::vector<Item, mi_stl_allocator<Item>>, mi_stl_allocator<std::vector<Item, mi_stl_allocator<Item>>>> per_index_results(size);

    tbb::parallel_for(0,
                      size,
                      [&](const int _i) {
                          try {
                              const auto item_list = items_data->GetValueByIndex(_i);
                              if (util::is_bad_ptr(item_list)) {
                                  return;
                              }

                              const auto pickitems = item_list->ToArray();
                              per_index_results[_i].resize(item_list->size);
                              for (int j = 0; j < item_list->size; j++) {
                                  auto& [name, screen_pos, id, type, num] = per_index_results[_i][j];
                                  const auto address = pickitems->At(j);
                                  if (util::is_bad_ptr(address)) {
                                      continue;
                                  }

                                  auto pos = PickItem::pos[address];
                                  const auto item_net = PickItem::my_pick_item_net[address];
                                  const auto item_config = PickItem::pick_item_data_config[address];
                                  if (util::is_bad_ptr(item_net) || util::is_bad_ptr(item_config)) {
                                      continue;
                                  }

                                  screen_pos.first = pos;
                                  screen_pos.second = w2c->commit(pos);

                                  id = PickItemDataConfig::item_id[item_config];
                                  type = PickItemDataConfig::item_type[item_config];
                                  num = PickItemNet::num[item_net];
                                  name = std::move(PickItemDataConfig::item_name[item_config]->ToString());
                              }
                          } catch (...) {}
                      });

    for (const auto& vec : per_index_results) {
        items_commit.insert(items_commit.end(), std::make_move_iterator(vec.begin()), std::make_move_iterator(vec.end()));
    }
} catch (...) {
    MessageBoxA(nullptr, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}

auto ItemEsp::process_data() -> void try {
    if (items_commit.empty()) {
        return;
    }

    const auto w2c = W2C::instance();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, items_commit.size()),
                      [&](const tbb::blocked_range<size_t>& _range) {
                          for (size_t i = _range.begin(); i != _range.end(); ++i) {
                              Item& value = items_commit[i];
                              value.screen_pos.first = w2c->pos_done[value.screen_pos.second];
                          }
                      });


    std::lock_guard lock(mutex);
    items = std::move(items_commit);
} catch (...) {
    MessageBoxA(nullptr, "未经处理的异常!\n" __FUNCTION__, "致命错误!", MB_ICONERROR);
}

auto ItemEsp::draw_info(ImDrawList* _bg, const std::conditional_t<true, glm::vec3, int>& _screen_pos, util::String& _name, const int64_t _id, int64_t _type, const int _num) -> void {
    if (_name.empty()) {
        return;
    }

    ItemInfo*& item = info_list_id[_id];
    if (!item) {
        item = &info_list[_name];
    }

    auto& [group, type, color, show] = *item;
    color.Value.w = 0.4f;

    if (_num > 1) {
        _name += " x";
        _name += std::to_string(_num);
    }
    const ImVec2 name_text_size = ImGui::CalcTextSize(_name.data());

    const glm::vec2 size = {name_text_size.x + 2.f * 2.0f, name_text_size.y + 2.f * 2.0f};
    const glm::vec2 min = {_screen_pos.x - size.x / 2.0f, _screen_pos.y - size.y / 2.0f};
    const glm::vec2 max = {_screen_pos.x + size.x / 2.0f, _screen_pos.y + size.y / 2.0f};

    _bg->AddRect({min.x - 1, min.y - 1}, {max.x + 1, max.y + 1}, ImColor(0.f, 0.f, 0.f, color.Value.w));
    _bg->AddRectFilled({min.x, min.y}, {max.x, max.y}, color);

    const ImVec2 text_pos = {_screen_pos.x - name_text_size.x / 2.0f, _screen_pos.y - name_text_size.y / 2.0f};
    _bg->AddText(text_pos, ImColor(255, 255, 255), _name.data());
}
