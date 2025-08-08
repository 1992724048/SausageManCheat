#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <App.h>
#include "HardBreakPoint.h"

HMODULE g_dll_handle = nullptr;
auto APIENTRY DllMain(const HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lpReserved) -> BOOL {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            g_dll_handle = hModule;
            if (GetModuleHandle(GAME_NAME)) {
                auto& app = App::instance();
                app.set_dll_handle(hModule);
                app.run();
            }
            break;
        }
        case DLL_THREAD_ATTACH: {
            I::ThreadAttach();
            CONTEXT ctx = {0};
            GetThreadContext(GetCurrentThread(), &ctx);
            ctx.Dr7 = HardBreakPoint::dr7_.raw;
            SetThreadContext(GetCurrentThread(), &ctx);
            break;
        }
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            TerminateProcess(GetCurrentProcess(), 0);
            break;
    }
    return TRUE;
}

extern "C" _declspec(dllexport) auto CALLBACK hook_callback(const int _code, const WPARAM wParam, const LPARAM lParam) -> LRESULT {
    return CallNextHookEx(nullptr, _code, wParam, lParam);
}