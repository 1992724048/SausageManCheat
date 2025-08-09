#pragma once
#include "../SharedMemory.h"

#include "libipc/ipc.h"

#pragma pack(push, 4)
class W2C final : public SharedMemoryRegistrar<W2C> {
public:
    char space[4];
    glm::mat4x4 camera_matrix;
    glm::vec2 screen_size;
    int count;
    glm::vec3 pos[8192];
    glm::vec3 pos_done[8192];

    W2C();
    ~W2C();

    auto start() const -> void;
    auto wait() const -> void;
    auto clear() -> void;
    auto commit(const glm::vec3& _pos) -> int;

    inline static std::shared_ptr<ipc::channel> ipc;
};
#pragma pack(pop)