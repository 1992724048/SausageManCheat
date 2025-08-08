#pragma once
#include "pch.h"

class App final {
public:
    App();
    App(const App&) = delete;
    auto operator=(const App&) -> App& = delete;
    App(App&&) = delete;
    auto operator=(App&&) -> App& = delete;

    static auto instance() -> App&;

    [[nodiscard]] auto get_dll_handle() const -> HMODULE;

    auto set_dll_handle(HMODULE handle) -> void;

    [[nodiscard]] auto get_game_handle() const -> HMODULE;

    auto path() const -> std::filesystem::path;

    auto run() const -> void;

    auto read_resource(const int resource_id, const LPCTSTR resource_type) const -> std::pair<std::shared_ptr<std::uint8_t[]>, std::size_t> ;

private:
    HMODULE dll_handle;
    HMODULE game_handle;

    auto init_game_data() -> void;

    static auto init_cheat_data(HWND, ID3D11Device*, ID3D11DeviceContext*, IDXGISwapChain*) -> void;
    static auto on_rander(ID3D11DeviceContext*) -> void;
};
