#include "W2C_IPC.h"

#include "memory/W2C/W2C.h"

W2C_IPC::W2C_IPC() {
  
}

void W2C_IPC::start_calc(ipc::channel& _ch) {
    const auto i = instance();
    i->isDone = false;
    W2C::instance()->calc_pos();
    _ch.send("done");
    i->isDone = true;
}

auto W2C_IPC::is_done(ipc::channel& _ch) -> void {
    const auto i = instance();
    i->isDone ? _ch.send("done") : _ch.send("no");
}
