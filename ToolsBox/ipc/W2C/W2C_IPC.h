#pragma once

#include <ipc/IPC.h>

class W2C_IPC final : public IPCRegistrar<W2C_IPC> {
public:
    IPC_MSG_BEGIN
        IPC_MSG_ADD(start_calc)
    IPC_MSG_END

    W2C_IPC();

    bool isDone;
private:
    static auto start_calc(IPC_MSG_ARGS) -> void;
    static auto is_done(IPC_MSG_ARGS) -> void;
};
