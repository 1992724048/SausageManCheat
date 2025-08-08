#pragma once
#include <pch.h>

#include <utility>

#include "SYCL/SYCL.hpp"

class Calculation {
public:
    /**
     * @brief 世界坐标转屏幕坐标
     * @param _pos std::vector([id, pos])
     * @return std::vector([id, pos])
     */
    virtual auto w2c(const std::vector<std::pair<int, glm::vec3>>& _pos) -> std::vector<std::pair<int, glm::vec3>> {
        return ptr.load()->w2c(_pos);
    }

    auto update_matrix(const glm::mat4x4& _matrix) -> void {
        camera_matrix.store(_matrix);
    }

    auto update_size(const glm::vec2& _size) -> void {
        screen_size.store(_size);
    }

    static auto instance() -> std::shared_ptr<Calculation> {
        return ptr.load();
    }

    Calculation(const Calculation&) = delete;
    auto operator=(const Calculation&) -> Calculation& = delete;
    Calculation(Calculation&&) = delete;
    auto operator=(Calculation&&) -> Calculation& = delete;

protected:
    Calculation() = default;
    ~Calculation() = default;
    std::atomic<glm::mat4x4> camera_matrix;
    std::atomic<glm::vec2> screen_size;
    glm::vec2 previous_screen_size;
    inline static std::atomic<std::shared_ptr<Calculation>> ptr;
};

template<class T>
class CalculationSingleton : public Calculation {
public:
    static auto instance() -> std::shared_ptr<T> {
        std::shared_ptr<T> instance = std::make_shared<T>();
        ptr.store(instance);
        return instance;
    }

private:
    CalculationSingleton(const CalculationSingleton&) = delete;
    auto operator=(const CalculationSingleton&) -> CalculationSingleton& = delete;
    CalculationSingleton(CalculationSingleton&&) = delete;
    auto operator=(CalculationSingleton&&) -> CalculationSingleton& = delete;

    CalculationSingleton() = default;

protected:
    ~CalculationSingleton() = default;

private:
    inline static std::once_flag once_flag;
    friend T;
};

class CALC_TBB final : public CalculationSingleton<CALC_TBB> {
public:
    CALC_TBB() = default;
    ~CALC_TBB() = default;

    CALC_TBB(const CALC_TBB&) = delete;
    auto operator=(const CALC_TBB&) -> CALC_TBB& = delete;
    CALC_TBB(CALC_TBB&&) = delete;
    auto operator=(CALC_TBB&&) -> CALC_TBB& = delete;

    auto w2c(const std::vector<std::pair<int, glm::vec3>>& _pos) -> std::vector<std::pair<int, glm::vec3>> override {
        const auto camera_matrix = this->camera_matrix.load();
        const auto screen_size = this->screen_size.load();

        std::vector<std::pair<int, glm::vec3>> result(_pos.size());
        if (_pos.empty()) {
            return result;
        }

        tbb::parallel_for(tbb::blocked_range<size_t>(0, _pos.size()),
                          [&](const tbb::blocked_range<size_t>& _r) {
                              for (size_t i = _r.begin(); i != _r.end(); ++i) {
                                  const auto& [id, pos] = _pos[i];

                                  glm::vec4 clip_space_pos = camera_matrix * glm::vec4(pos, 1.0f);
                                  if (clip_space_pos.w == 0.0f) {
                                      continue;
                                  }

                                  glm::vec3 screen;
                                  screen.z = clip_space_pos.w;
                                  clip_space_pos /= clip_space_pos.w;

                                  screen.x = (clip_space_pos.x + 1.0f) * 0.5f * screen_size.x;
                                  screen.y = (clip_space_pos.y + 1.0f) * 0.5f * screen_size.y;
                                  screen.y = screen_size.y - screen.y;

                                  result[i] = {id, screen};
                              }
                          });

        return result;
    }
};

class CALC_SYCL final : public CalculationSingleton<CALC_SYCL> {
public:
    CALC_SYCL() = default;
    ~CALC_SYCL() = default;

    CALC_SYCL(const CALC_SYCL&) = delete;
    auto operator=(const CALC_SYCL&) -> CALC_SYCL& = delete;
    CALC_SYCL(CALC_SYCL&&) = delete;
    auto operator=(CALC_SYCL&&) -> CALC_SYCL& = delete;

    auto w2c(const std::vector<std::pair<int, glm::vec3>>& _pos) -> std::vector<std::pair<int, glm::vec3>> override {
        const auto camera_matrix = this->camera_matrix.load();
        const auto screen_size = this->screen_size.load();

        std::vector<std::pair<int, glm::vec3>> result(_pos.size());
        if (_pos.empty()) {
            return result;
        }

        SYCL::instance().get_cl().w2c(_pos, result, camera_matrix, screen_size);
        return result;
    }
};

class CALC_OpenMP final : public CalculationSingleton<CALC_OpenMP> {
public:
    CALC_OpenMP() = default;
    ~CALC_OpenMP() = default;

    CALC_OpenMP(const CALC_OpenMP&) = delete;
    auto operator=(const CALC_OpenMP&) -> CALC_OpenMP& = delete;
    CALC_OpenMP(CALC_OpenMP&&) = delete;
    auto operator=(CALC_OpenMP&&) -> CALC_OpenMP& = delete;

    auto w2c(const std::vector<std::pair<int, glm::vec3>>& _pos) -> std::vector<std::pair<int, glm::vec3>> override {
        const auto camera_matrix = this->camera_matrix.load();
        const auto screen_size = this->screen_size.load();

        std::vector<std::pair<int, glm::vec3>> result(_pos.size());

        if (_pos.empty()) {
            return result;
        }

#pragma omp parallel for
        for (int i = 0; i < _pos.size(); ++i) {
            const auto& [id, pos] = _pos[i];
            glm::vec4 clip_space_pos = camera_matrix * glm::vec4(pos, 1.0f);

            if (clip_space_pos.w == 0.0f) {
                continue;
            }

            glm::vec3 screen;
            screen.z = clip_space_pos.w;
            clip_space_pos /= clip_space_pos.w;

            screen.x = (clip_space_pos.x + 1.0f) * 0.5f * screen_size.x;
            screen.y = (clip_space_pos.y + 1.0f) * 0.5f * screen_size.y;
            screen.y = screen_size.y - screen.y;
            result[i] = {id, screen};
        }

        return result;
    }
};
