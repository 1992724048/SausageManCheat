#include <App.h>

#include "FeatureRegistrar.h"
#include "HardBreakPoint.h"
#include "dxhook.h"

#include "../memory/SharedMemory.h"

#include ".class/ClassRegistrar.h"
#include ".class/CameraController/CameraController.h"

App::App() = default;

auto App::instance() -> App& {
    static App instance;
    return instance;
}

auto App::get_dll_handle() const -> HMODULE {
    return dll_handle;
}

auto App::set_dll_handle(const HMODULE handle) -> void {
    dll_handle = handle;
}

auto App::get_game_handle() const -> HMODULE {
    return game_handle;
}

auto App::path() const -> std::filesystem::path {
    static std::filesystem::path app_path;
    if (app_path.empty()) {
        std::string path_(MAX_PATH, '\0');
        GetModuleFileNameA(dll_handle, path_.data(), MAX_PATH);
        app_path = std::filesystem::path(path_).parent_path();
    }
    return app_path;
}

auto App::run() const -> void try {
    if (!GetModuleHandle(GAME_NAME)) {
        return;
    }

    Crash::init(path());
    HardBreakPoint::initialize();

#ifdef _DEBUG
    Logger::OpenConsole();
    Logger::PrepareFileLogging(path() / "logs");
    Logger::SetLevel(Logger::Level::Debug, Logger::LoggerType::Any);
#endif

    LOG_INFO << "注入成功!";
    LOG_INFO << "系统目录: " << path();

    DXHook::initialize_event += FUNCTION_HANDLER(init_cheat_data);
    DXHook::on_render += FUNCTION_HANDLER(on_rander);
    std::thread(DXHook::initialize).detach();
} catch (const std::exception& exception) {
    LOG_ERROR << exception.what();
    MessageBoxA(nullptr, exception.what(), "错误", 0);
}

auto App::read_resource(const int resource_id, const LPCTSTR resource_type) const -> std::pair<std::shared_ptr<std::uint8_t[]>, std::size_t> {
    if (!dll_handle) {
        return { nullptr, 0 };
    }

    const HRSRC res = ::FindResource(dll_handle, MAKEINTRESOURCE(resource_id), resource_type);
    if (!res) {
        return { nullptr, 0 };
    }

    const DWORD size = SizeofResource(dll_handle, res);
    if (size == 0) {
        return { nullptr, 0 };
    }

    const HGLOBAL global = LoadResource(dll_handle, res);
    if (!global) {
        return { nullptr, 0 };
    }

    void* data = LockResource(global);
    if (!data) {
        return { nullptr, 0 };
    }

    std::shared_ptr<std::uint8_t[]> ptr(static_cast<std::uint8_t*>(data), [](std::uint8_t*) {});
    return { std::move(ptr), size };
}

auto App::init_game_data() -> void {
    LOG_INFO << "Application is running.";
    while (!GetModuleHandle(L"GameAssembly.dll")) {
        LOG_WARNING << "Game not init! wait for 5s";
        std::this_thread::sleep_for(5s);
    }

    game_handle = GetModuleHandle(L"GameAssembly.dll");

    UnityResolve::Init(game_handle, UnityResolve::Mode::Il2Cpp);

}

auto App::init_cheat_data(HWND, ID3D11Device* pDevice, ID3D11DeviceContext*, IDXGISwapChain* pChain) -> void try {
    try {
        instance().init_game_data();
        SharedMemory::initialize(true);
        FeatureBase::initialize();
        ClassBase::initialize();
    } catch (const std::exception& exception) {
        MessageBoxA(nullptr, std::format("{}\n请尝试重启启动器与游戏!", exception.what()).data(), "错误", 0);
    }
    LOG_INFO << "初始化完成!";
} catch (const std::exception& exception) {
    LOG_ERROR << exception.what();
    MessageBoxA(nullptr, exception.what(), "错误", 0);
}

auto App::on_rander(ID3D11DeviceContext*) -> void {
    if (!CameraController::camera_controller) {
        return;
    }
    if (BattleRole::roles.empty()) {
        return;
    }
    FeatureBase::instance().render();
    CameraController::camera_controller = nullptr;
}
