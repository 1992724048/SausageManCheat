#include "W2C.h"

#include <utility>

#include "fetures/CalculationSingleton.h"

W2C::W2C(): camera_matrix{}, screen_size{}, count{0}, pos{} {}

W2C::~W2C() {}

auto W2C::config_callback() -> void {}

auto W2C::calc_pos() -> void {
    auto lock = mutex();
    if (!count) {
        return;
    }

    std::vector<std::pair<int, glm::vec3>> indexed_pos;
    indexed_pos.resize(count);

    tbb::parallel_for(tbb::blocked_range(0, count),
                      [&](const tbb::blocked_range<int>& _range) {
                          for (int i = _range.begin(); i != _range.end(); ++i) {
                              indexed_pos[i] = std::make_pair(i, pos[i]);
                          }
                      });

    const auto calc = Calculation::instance();
    calc->update_size(screen_size);
    calc->update_matrix(camera_matrix);

    const std::vector<std::pair<int, glm::vec3>> transformed = calc->w2c(indexed_pos);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, transformed.size()),
                      [&](const tbb::blocked_range<size_t>& _range) {
                          for (size_t k = _range.begin(); k != _range.end(); ++k) {
                              const auto& [id, position] = transformed[k];
                              if (id < 0 || id >= 8192) {
                                  continue;
                              }
                              pos_done[id] = position;
                          }
                      });
}
