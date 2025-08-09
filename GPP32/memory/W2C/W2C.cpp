#include "W2C.h"
#include <utility>

W2C::W2C(): camera_matrix{}, screen_size{}, count{0}, pos{} {
    ipc = std::make_shared<ipc::channel>("ipc_channel_W2C_IPC", ipc::sender | ipc::receiver);
}

W2C::~W2C() {
    
}

auto W2C::start() const -> void {
    if (!count) {
        return;
    }

    ipc->send("start_calc");
}

auto W2C::wait() const -> void {
    if (!count) {
        return;
    }

    auto lock = mutex();

    /*bool is_done = false;
    do {
        const auto msg = ipc->recv(3);
        if (!msg.empty()) {
            is_done = std::string(msg.get<const char*>()) == "done";
        } else if (ipc->send("is_done")) {} else {
            is_done = true;
        }
    } while (!is_done);*/
}

auto W2C::clear() -> void {
    count = 0;
}

auto W2C::commit(const glm::vec3& _pos) -> int {
    static std::mutex mutex;
    std::lock_guard lock(mutex);

    if (count >= 8192) {
        return 0;
    }

    const decltype(count) index = count++;
    pos[index] = _pos;
    return index;
}
