#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <ToolsBox.h>
#include "SYCL/SYCL.hpp"
#include "bitsdojo_window_windows/bitsdojo_window_plugin.h"
#include "fetures/FeatureRegistrar.hpp"
#include "window_manager/window_manager_plugin.h"
#include "flutter/flutter_window.h"

#include "ipc/IPC.h"

#include "memory/SharedMemory.h"
#include "opencv2/world.hpp"

class SingleInstance {
public:
    explicit SingleInstance(const wchar_t* name) : m_mutex(CreateMutexW(nullptr, TRUE, name)), m_last_error(GetLastError()) {}

    ~SingleInstance() {
        if (m_mutex) {
            ReleaseMutex(m_mutex);
            CloseHandle(m_mutex);
        }
    }

    auto is_another_running() const -> bool {
        return m_last_error == ERROR_ALREADY_EXISTS;
    }

private:
    HANDLE m_mutex;
    DWORD m_last_error;
};

static auto enable_debug_privilege() -> bool {
    HANDLE hToken = nullptr;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    if (!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)) {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr)) {
        CloseHandle(hToken);
        return false;
    }

    CloseHandle(hToken);
    return GetLastError() == ERROR_SUCCESS;
}

static auto get_real_windows_build_number() -> DWORD {
    using RtlGetVersionPtr = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    const HMODULE h_mod = GetModuleHandleW(L"ntdll.dll");
    if (!h_mod) {
        return 0;
    }

    const auto fx_ptr = reinterpret_cast<RtlGetVersionPtr>(GetProcAddress(h_mod, "RtlGetVersion"));
    if (!fx_ptr) {
        return 0;
    }

    RTL_OSVERSIONINFOW rovi = {};
    rovi.dwOSVersionInfoSize = sizeof(rovi);
    if (fx_ptr(&rovi) != 0) {
        return 0;
    }

    return rovi.dwBuildNumber;
}

auto is_windows_22h2_or_greater() -> bool {
    return get_real_windows_build_number() >= 19045;
}

auto wWinMain(HINSTANCE _instance, HINSTANCE _prev_instance, LPWSTR _cmd_line, int _cmd_show) -> int try {
    SingleInstance guard(L"Global\\ToolsBoxGUI");
    if (guard.is_another_running()) {
        MessageBoxW(nullptr, L"请勿重复启动该程序!", L"提示", MB_ICONINFORMATION);
        return 0;
    }

    if (!is_windows_22h2_or_greater()) {
        MessageBoxW(nullptr, L"本程序需要 Windows 10 22H2 (19045) 或更高版本!", L"提示", MB_ICONINFORMATION);
        return EXIT_FAILURE;
    }

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
    _set_se_translator(ExceptionTranslator::translate_seh_to_ce);
    Crash::init();
#ifdef _DEBUG
    Logger::PrepareFileLogging(util::app_path() / "logs");
    Logger::OpenConsole();
    Logger::SetLevel(Logger::Level::Trace, Logger::LoggerType::Any);
#endif
    LOG_INFO << "提权状态: " << enable_debug_privilege();

    config::initialize(util::app_path() / "config.json");
    ippInit();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    auto& app = App::instance();
    app.initialize();
    SYCL::instance();
    FeatureBase::initialize();
    LOG_DEBUG << "加载功能数量:" << FeatureBase::get_count();
    SharedMemory::initialize(false);
    LOG_DEBUG << "加载内存数量:" << SharedMemory::get_count();
    app.commit_config();

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    [[maybe_unused]] auto bdw = bitsdojo_window_configure(BDW_CUSTOM_FRAME | BDW_HIDE_ON_STARTUP);

    flutter::DartProject project(L"data");
    std::vector<std::string> command_line_arguments;
    project.set_dart_entrypoint_arguments(std::move(command_line_arguments));
    FlutterWindow window(project);

    if (window.Create(L"千禧科技", {100, 100}, {1280, 720})) {
        window.SetQuitOnClose(false);
    }

    Tray::init(L"千禧科技", IDI_ICON1);
    Tray::create_tray(L"ToolsBox 后台服务\n版本: 1.0.0", L"千禧科技", L"后台服务");
    Tray::set_tray_icon(IDI_ICON1);
    Tray::add_menu_item(L"打开界面",
                        [&]() {
                            [[maybe_unused]] auto _ = window.Show(true);
                        });

    Tray::add_menu_item(L"退出服务",
                        [&app]() {
                            if (MessageBoxW(nullptr, L"确定要退出后台服务吗?\n退出后台服务将会导致相关程序无法正常运行!", L"警告!", MB_ICONWARNING | MB_OKCANCEL) == IDOK) {
                                Tray::quit();
                            }
                        });

    IPC::registrar_all();
    IPC::start_all_ipc();
    app.port = app.server.bind_to_any_port("localhost");
    std::thread listener_thread;
    listener_thread = std::thread([&app] {
        app.server.listen_after_bind();
    });
    Tray::run([&app] {});

    IOCPSocket::stop_socket();
    app.server.stop();

    if (listener_thread.joinable()) {
        listener_thread.join();
    }

    window.Destroy();
    CoUninitialize();
    IPC::start_all_ipc();
    SharedMemory::uninitialize();
    EVP_cleanup();
    ERR_free_strings();
    Crash::uninit();
    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
    return 1;
}
