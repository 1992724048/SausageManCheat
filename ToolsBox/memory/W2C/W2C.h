#pragma once
#include "../SharedMemory.h"

#pragma pack(push, 4)
class W2C final : public SharedMemoryRegistrar<W2C> {
public:
    glm::mat4x4 camera_matrix;
    glm::vec2 screen_size;
    int count;
    glm::vec3 pos[4096];
    glm::vec3 pos_done[4096];

    W2C();
    ~W2C();
    auto config_callback() -> void override;
    auto calc_pos() -> void;
};
#pragma pack(pop)