#include "RoleESP.h"

#include <execution>
#include <random>

#include "AIESP.h"
#include "AIESP.h"
#include "AIESP.h"
#include "AIESP.h"
#include "AIESP.h"
#include "AIESP.h"
#include "AIESP.h"
#include "AIESP.h"

#include ".class/CameraController/CameraController.h"

#include "magic_enum/magic_enum.hpp"

#include "memory/ESP/ESPConfig.h"
#include "memory/W2C/W2C.h"

ESP::ESP() {}

auto ESP::render() -> void {
    std::vector<Role, mi_stl_allocator<Role>> temp;
    {
        std::lock_guard lock_s(mutex);
        temp = std::move(roles);
        roles.clear();
    }

    if (temp.empty()) {
        return;
    }

    const auto esp = ESPConfig::instance();
    auto lock = esp->mutex();

    if (!esp->enable) {
        return;
    }

    if (temp.empty()) {
        return;
    }

    const auto local = local_role.load();
    if (!local) {
        return;
    }

    for (auto& role : temp) {
        auto& [screen_pos, pos] = role.screen_pos;
        if (screen_pos.z < 0.f) {
            continue;
        }

        if (role.local || role.dead) {
            continue;
        }

        if (!esp->show_team && local->team == role.team) {
            continue;
        }

        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        if (esp->show_role) {
            /*if (role.hide) {
                bg->AddText({role.screen_pos_top.first.x, role.screen_pos_top.first.y}, ImColor(255, 0, 0), reinterpret_cast<const char*>(u8"掩体后"));
            }*/

            if (esp->show_info) {
                screen_pos.y += 15;
                draw_info(bg, screen_pos, role.name, role.team, role.weak, role.hp, role.falling);
            }

            if (esp->show_bone && !role.falling) {
                auto& [head, _id] = role.screen_pos_top;
                auto& [neck, _id2] = role.screen_pos_neck;
                const glm::vec2 pos_on_line = glm::mix(neck, head, 0.55);
                bg->AddCircle({pos_on_line.x, pos_on_line.y}, glm::distance(neck, head) / 2, ImColor(25, 255, 25));
                draw_bone(bg, role.bones);
            }

            if (esp->show_box && !role.falling) {
                draw_box(bg, role.rect_pos_upper, role.rect_pos_lower);
            }
        }
    }
}

auto ESP::update() -> void {
    if (util::is_bad_ptr(local_role.load())) {
        return;
    }

    const auto w2c = W2C::instance();
    process_data();
    roles_commit.clear();
    roles_commit.resize(BattleRole::roles.size());

    int index = 0;
    for (const auto& role : BattleRole::roles) {
        try {
            Role& _i = roles_commit[index++];
            _i.role = role;

            if (util::is_bad_ptr(role)) {
                continue;
            }

            _i.role_control = BattleRole::role_control[role];
            if (util::is_bad_ptr(_i.role_control)) {
                continue;
            }

            const auto animator_control = RoleControl::animator_control[_i.role_control];
            if (util::is_bad_ptr(animator_control)) {
                continue;
            }

            const auto animator = AnimatorControl::animator[animator_control];
            if (util::is_bad_ptr(animator_control)) {
                continue;
            }

            const auto head = AnimatorControl::head[animator_control];
            if (util::is_bad_ptr(head)) {
                continue;
            }

            const auto neck = animator->GetBoneTransform(II::Animator::HumanBodyBones::Neck);
            if (util::is_bad_ptr(neck)) {
                continue;
            }

            const auto pos_trans = BattleRole::pos[role];
            if (util::is_bad_ptr(pos_trans)) {
                continue;
            }

            _i.pos = pos_trans->GetPosition();
            _i.scale = pos_trans->GetLocalScale();
            std::pair<glm::vec3, int> pos_bottom;
            process_bone(animator, _i.bones, pos_bottom);
            _i.screen_pos_top.first = head->GetPosition() + glm::vec3(0, 0.1f, 0);
            _i.screen_pos_top.second = w2c->commit(_i.screen_pos_top.first);
            _i.pos.y = pos_bottom.first.y - 0.1f;
            _i.screen_pos.second = w2c->commit(_i.pos);
            _i.screen_pos_neck.first = neck->GetPosition();
            _i.screen_pos_neck.second = w2c->commit(_i.screen_pos_neck.first);
        } catch (...) {}
    }

    tbb::parallel_for(tbb::blocked_range<size_t>(0, roles_commit.size()),
                      [&](const tbb::blocked_range<size_t>& _range) {
                          for (size_t i = _range.begin(); i != _range.end(); i++) {
                              Role& _i = roles_commit[i];
                              try {
                                  BattleRoleLogic* role_logic = BattleRole::role_logic[_i.role];
                                  _i.name = std::move(BattleRoleLogic::name[role_logic]->ToString());
                                  _i.team = BattleRoleLogic::team[role_logic];
                                  _i.hp = BattleRoleLogic::hp[role_logic];
                                  _i.weak = BattleRoleLogic::weak[role_logic];
                                  _i.x_rot = RoleControl::x_cam_rot[_i.role_control];
                                  _i.y_rot = RoleControl::y_cam_rot[_i.role_control];
                                  _i.local = BattleRoleLogic::local[role_logic];
                                  _i.falling = _i.weak != 100 && _i.hp == 0;
                                  _i.dead = _i.weak == 0 && _i.hp == 0;
                              } catch (...) {}

                              process_box(_i.scale, _i.pos, _i.screen_pos_top, _i.rect_pos_upper, _i.rect_pos_lower, _i.y_rot);
                          }
                      });
}

auto ESP::process_data() -> void {
    if (roles_commit.empty()) {
        return;
    }

    const auto w2c = W2C::instance();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, roles_commit.size()),
                      [&](const tbb::blocked_range<size_t>& _range) {
                          for (size_t i = _range.begin(); i != _range.end(); ++i) {
                              Role& value = roles_commit[i];

                              value.screen_pos.first = w2c->pos_done[value.screen_pos.second];
                              value.screen_pos_top.first = w2c->pos_done[value.screen_pos_top.second];
                              value.screen_pos_neck.first = w2c->pos_done[value.screen_pos_neck.second];

                              for (auto enum_value : magic_enum::enum_values<II::Animator::HumanBodyBones>()) {
                                  auto& [screen_pos, pos_id] = value.bones[enum_value];
                                  screen_pos = w2c->pos_done[pos_id];
                              }

                              for (auto& [pos, id] : value.rect_pos_upper) {
                                  pos = w2c->pos_done[id];
                              }

                              for (auto& [pos, id] : value.rect_pos_lower) {
                                  pos = w2c->pos_done[id];
                              }

                              if (value.local) {
                                  local_role = &value;
                              }
                          }
                      });

    /*const auto local = local_role.load();
    for (auto& role : roles_commit) {
        try {
            role.hide = II::Physics::raycast(local->pos,
                                             role.pos,
                                             glm::distance(local->bones[II::Animator::HumanBodyBones::Head].first, role.bones[II::Animator::HumanBodyBones::Head].first),
                                             1 << 18 | 1 << 23 | 1 << 4);
        } catch (...) {}
    }*/

    std::lock_guard lock_s(mutex);
    roles = std::move(roles_commit);
}

auto ESP::process_box(const glm::vec3& _scale_,
                      const glm::vec3& _pos_,
                      const std::pair<glm::vec3, int>& _pos_top,
                      std::array<std::pair<glm::vec3, int>, 16>& _rect_pos_upper,
                      std::array<std::pair<glm::vec3, int>, 16>& _rect_pos_lower,
                      float _rot) -> void {
    const auto w2c = W2C::instance();
    const float weight = 0.8f * _scale_.x;

    float half_w = weight * 0.5f;
    float half_d = weight * 0.5f;
    float fixed_len = 0.15f;

    const glm::vec3 base_offsets[4] = {{-half_w, 0, -half_d}, {half_w, 0, -half_d}, {half_w, 0, half_d}, {-half_w, 0, half_d}};

    const glm::vec3 side_offsets[4][3] = {
        {{fixed_len, 0, 0}, {0, 0, fixed_len}, {0, fixed_len, 0},},
        {{0, 0, fixed_len}, {-fixed_len, 0, 0}, {0, fixed_len, 0},},
        {{0, 0, -fixed_len}, {-fixed_len, 0, 0}, {0, fixed_len, 0},},
        {{fixed_len, 0, 0}, {0, 0, -fixed_len}, {0, fixed_len, 0},}
    };

    float rot_rad = glm::radians(_rot);
    glm::mat4 rot_y_mat = glm::rotate(glm::mat4(1.0f), rot_rad, glm::vec3(0, 1, 0));

    for (int i = 0; i < 4; ++i) {
        auto base_offset_rotated = glm::vec3(rot_y_mat * glm::vec4(base_offsets[i], 1.0f));
        glm::vec3 base_pos_upper = _pos_top.first + base_offset_rotated;
        glm::vec3 base_pos_lower = _pos_ + base_offset_rotated;

        _rect_pos_lower[i] = {base_pos_lower, w2c->commit(base_pos_lower)};
        _rect_pos_upper[i] = {base_pos_upper, w2c->commit(base_pos_upper)};

        auto side0 = glm::vec3(rot_y_mat * glm::vec4(side_offsets[i][0], 0.0f));
        auto side1 = glm::vec3(rot_y_mat * glm::vec4(side_offsets[i][1], 0.0f));
        auto side2 = glm::vec3(rot_y_mat * glm::vec4(side_offsets[i][2], 0.0f));

        _rect_pos_lower[i * 3 + 4] = {{}, w2c->commit(base_pos_lower + side0)};
        _rect_pos_lower[i * 3 + 5] = {{}, w2c->commit(base_pos_lower + side1)};
        _rect_pos_lower[i * 3 + 6] = {{}, w2c->commit(base_pos_lower + side2)};

        _rect_pos_upper[i * 3 + 4] = {{}, w2c->commit(base_pos_upper + side0)};
        _rect_pos_upper[i * 3 + 5] = {{}, w2c->commit(base_pos_upper + side1)};
        _rect_pos_upper[i * 3 + 6] = {{}, w2c->commit(base_pos_upper - side2)};
    }
}

auto ESP::process_bone(UnityResolve::UnityType::Animator* _animator, util::Map<II::Animator::HumanBodyBones, std::pair<glm::vec3, int>>& _bones, std::pair<glm::vec3, int>& _pos_bottom) const -> void {
    const auto w2c = W2C::instance();
    for (auto& enum_value : stick_figure_bones) {
        try {
            if (!_bones.contains(enum_value)) {
                const auto trans = _animator->GetBoneTransform(enum_value);
                if (util::is_bad_ptr(trans)) {
                    continue;
                }

                glm::vec3 vec = trans->GetPosition();
                if (enum_value == II::Animator::HumanBodyBones::RightFoot) {
                    _pos_bottom.first = vec;
                }

                _bones[enum_value] = {{}, w2c->commit(vec)};
            }
        } catch (...) {}
    }
}

namespace {
    auto random_color(int _value, const float _alpha) -> ImColor {
        _value = std::clamp(_value, 0, 100);

        const uint32_t h = static_cast<uint32_t>(_value) * 1664525u + 1013904223u;
        auto byte = [&](const int _shift) {
            return (h >> _shift & 0xFF) / 255.0f;
        };

        return ImColor{byte(0), byte(8), byte(16), _alpha};
    }
}

auto ESP::draw_info(ImDrawList* _bg,
                    const std::conditional_t<true, glm::vec3, int>& _screen_pos,
                    const util::String& _name,
                    const int64_t _team,
                    const float _weak,
                    const float _hp,
                    const bool _falling) -> void {
    const ImColor color(_falling ? ImColor(1.f, 0.f, 0.f, 0.5f) : random_color(_team, 0.5));
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

    _bg->AddRect({min.x - 1, min.y - 1}, {max.x + 1, max.y + 1}, ImColor(0.f, 0.f, 0.f, color.Value.w));
    _bg->AddRectFilled({min.x, min.y}, {max.x, max.y}, color);
    _bg->AddRectFilled({min.x, min.y}, {min_.x, max.y}, ImColor(color.Value.x, color.Value.y, color.Value.z, 1.f));

    _bg->AddRectFilled({min_.x, min_.y}, {max_.x, max_.y}, ImColor(0.f, 0.f, 0.f, color.Value.w));
    _bg->AddRectFilled({min_.x + 1, min_.y + weak_bar}, {max_.x - 1, max_.y}, ImColor(1.f, 0.f, 0.f, color.Value.w));
    _bg->AddRectFilled({min_.x + 1, min_.y + hp_bar}, {max_.x - 1, max_.y}, ImColor(0.f, 1.f, 0.f, color.Value.w));

    const std::string team = std::to_string(_team);
    const ImVec2 team_text_size = ImGui::CalcTextSize(team.data());

    const ImVec2 box_center = {(min.x + min_.x) / 2.0f, (min.y + max.y) / 2.0f};
    const ImVec2 team_text_pos = {box_center.x - team_text_size.x / 2.0f, box_center.y - team_text_size.y / 2.0f};

    const float name_text_y = (min.y + max.y) / 2.0f - name_text_size.y / 2.0f;
    _bg->AddText({max_.x + 5, name_text_y}, ImColor(255, 255, 255), _name.data());
    _bg->AddText(team_text_pos, ImColor(255, 255, 255), team.data());
}

auto ESP::draw_bone(ImDrawList* _bg, util::Map<II::Animator::HumanBodyBones, std::pair<glm::vec3, int>>& _bones) -> void {
    for (auto& [enum_value, enum_value2] : stick_figure_connections) {
        const auto& [pos, id] = _bones[enum_value];
        const auto& [pos2, id2] = _bones[enum_value2];

        if (pos.z < 0.f || pos2.z < 0.f) {
            continue;
        }

        _bg->AddLine({pos.x, pos.y}, {pos2.x, pos2.y}, ImColor(255, 255, 255));
    }
}

auto ESP::draw_box(ImDrawList* _bg, const std::array<std::pair<glm::vec3, int>, 16>& _rect_pos_upper, const std::array<std::pair<glm::vec3, int>, 16>& _rect_pos_lower) -> void {
    constexpr auto white = ImColor(255, 255, 255);

    for (int i = 0; i < 4; ++i) {
        const glm::vec3& base_lower = _rect_pos_lower[i].first;
        const glm::vec3& base_upper = _rect_pos_upper[i].first;
        if (base_lower.z < 0.f || base_upper.z < 0.f) {
            continue;
        }

        for (int j = 0; j < 3; ++j) {
            const glm::vec3& offset_lower = _rect_pos_lower[4 + i * 3 + j].first;
            const glm::vec3& offset_upper = _rect_pos_upper[4 + i * 3 + j].first;
            if (offset_lower.z < 0.f || offset_upper.z < 0.f) {
                continue;
            }
            _bg->AddLine({base_lower.x, base_lower.y}, {offset_lower.x, offset_lower.y}, white);
            _bg->AddLine({base_upper.x, base_upper.y}, {offset_upper.x, offset_upper.y}, white);
        }
    }
}
