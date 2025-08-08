#include <Windows.h>
#include <string>
#include <string_view>
#include <TlHelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <cwctype>
#include <functional>

auto iequals(std::wstring_view _a, std::wstring_view _b) -> bool {
    return _a.size() == _b.size() && std::ranges::equal(_a,
        _b,
        [](const wchar_t _c1, const wchar_t _c2) {
            return std::towlower(_c1) == std::towlower(_c2);
        });
}

auto for_each_process(const std::wstring_view _target_name, const std::function<bool(const PROCESSENTRY32W&)>& _on_match) -> bool {
    const auto snapshot = std::shared_ptr<void>(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), CloseHandle);
    if (snapshot.get() == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(PROCESSENTRY32W);
    bool result = false;

    if (Process32FirstW(snapshot.get(), &entry)) {
        do {
            if (iequals(entry.szExeFile, _target_name)) {
                result = _on_match(entry);
                break;
            }
        } while (Process32NextW(snapshot.get(), &entry));
    }
    return result;
}

auto is_process_running(const std::wstring_view _process_name) -> bool {
    return for_each_process(_process_name,
        [](const PROCESSENTRY32W&) {
            return true;
        });
}

#define GAME_NAME L"Sausage Man.exe"
#define DLL_PATH  L"GPP32.dll"

auto enable_debug_privilege() -> bool {
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

auto get_process_ids_by_name(const std::wstring& name) -> std::vector<DWORD> {
    std::vector<DWORD> pids;
    auto snapshot = std::shared_ptr<void>(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), CloseHandle);
    if (snapshot.get() == INVALID_HANDLE_VALUE) {
        return pids;
    }

    PROCESSENTRY32 pe32 = {sizeof(PROCESSENTRY32)};
    for (BOOL has_proc = Process32First(snapshot.get(), &pe32); has_proc; has_proc = Process32Next(snapshot.get(), &pe32)) {
        if (name == pe32.szExeFile) {
            pids.push_back(pe32.th32ProcessID);
        }
    }

    return pids;
}

auto get_threads_by_pid(DWORD pid) -> std::vector<DWORD> {
    std::vector<DWORD> threads;
    EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL {
                    DWORD dwProcessId;
                    DWORD threadId = GetWindowThreadProcessId(hWnd, &dwProcessId);
                    if (dwProcessId == static_cast<DWORD>(lParam) && IsWindowVisible(hWnd)) {
                        reinterpret_cast<std::vector<DWORD>*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))->push_back(threadId);
                        return FALSE;
                    }
                    return TRUE;
                },
                static_cast<LPARAM>(pid));
    return threads;
}

std::vector<DWORD> threadIds;
BOOL CALLBACK enum_window_callback(HWND hWnd, LPARAM lParam) {
    DWORD dwProcessId;
    DWORD threadId = GetWindowThreadProcessId(hWnd, &dwProcessId);
    if (dwProcessId == static_cast<DWORD>(lParam) && IsWindowVisible(hWnd)) {
        threadIds.push_back(threadId);
    }
    return TRUE;
}

auto main() -> int {
    if (!enable_debug_privilege()) {
        std::wcerr << L"Failed to enable debug privilege.\n";
        return EXIT_FAILURE;
    }

    const auto dll_handle = LoadLibraryW(DLL_PATH);
    if (!dll_handle) {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            std::wcerr << L"DLL not found: " << DLL_PATH << L"\n";
        } else {
            std::wcerr << L"Failed to load DLL: " << DLL_PATH << L", Error: " << error << L"\n";
        }
        std::wcerr << L"Failed to load DLL for hook: " << DLL_PATH << L"\n";
        return EXIT_FAILURE;
    }

    auto module = GetModuleHandleW(L"GPP32.dll");
    if (!module) {
        std::wcerr << L"Could not get module handle after LoadLibrary.\n";
        return EXIT_FAILURE;
    }

    const auto pids = get_process_ids_by_name(GAME_NAME);
    if (pids.empty()) {
        std::wcerr << L"Game process not found: " << GAME_NAME << L"\n";
        return EXIT_FAILURE;
    }

    auto hook_callback = reinterpret_cast<HOOKPROC>(GetProcAddress(dll_handle, "_hook_callback@12"));
    if (!hook_callback) {
        std::wcerr << L"Could not get callback address." << std::endl;
        FreeLibrary(dll_handle);
        return EXIT_FAILURE;
    }

    for (DWORD pid : pids) {
        threadIds.clear();
        EnumWindows(enum_window_callback, pid);

        for (DWORD threadId : threadIds) {
            HHOOK hook = SetWindowsHookExW(WH_GETMESSAGE, hook_callback, module, threadId);
            if (hook) {
                while (true) {
                    PostThreadMessageW(threadId, WM_NULL, 0, 0);
                    Sleep(10000);
                    if (!is_process_running(L"ToolsBox.exe") || !is_process_running(GAME_NAME)) {
                        exit(0);
                    }
                }
            }
            std::wcerr << L"Failed to hook thread: " << threadId << std::endl;
        }
    }

    return EXIT_FAILURE;
}
