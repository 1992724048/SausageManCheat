#pragma once
#include <pch.h>

class App {
public:
    static auto instance() -> App& {
        static App that;
        return that;
    }

    auto commit_config() -> void {
        config_update();
    }

    auto get_port() const -> int {
        return port;
    }

    TEvent<> config_update;
private:
    std::once_flag once_flag;
    httplib::Server server;
    int port;

    auto initialize() -> void {
        std::call_once(once_flag, init);
    }

    static auto init() -> void {
        auto& that = instance();
        config::setup_update(&that.config_update);
    }

    friend auto wWinMain(HINSTANCE _instance, HINSTANCE _prev_instance, LPWSTR _cmd_line, int _cmd_show) -> int;
};
