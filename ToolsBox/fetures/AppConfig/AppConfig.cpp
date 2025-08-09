#include <fetures/AppConfig/AppConfig.h>

#include "ToolsBox.h"
#include "fetures/CalculationSingleton.h"
#include <TlHelp32.h>
#include <cwctype>

namespace {
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

    DWORD g_injector_pid = 0;
    HANDLE g_injector_handle = nullptr;

    auto launch_injector() -> void {
        SHELLEXECUTEINFOW sei = {sizeof(sei)};
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = L"runas";
        sei.lpFile = L"GamePlusPlus\\GamePPLite.exe";
        sei.nShow = SW_HIDE;

        if (ShellExecuteExW(&sei)) {
            g_injector_handle = sei.hProcess;
            g_injector_pid = GetProcessId(sei.hProcess);
        }
    }
}

AppConfig::AppConfig() : NFS(f_auto_chek_update, "", "AppConfig", true),
                         NFS(f_check_file, "", "AppConfig", true),
                         NFB(f_calc_process, "", "AppConfig", "", true),
                         NFB(f_calc_xpu, "", "AppConfig", XPU::CPU, true),
                         NFB(f_no_sycl, "", "AppConfig", true, true),
                         NFS(f_multi_thread, "", "AppConfig", true) {
    if (!f_no_sycl) {
        try {
            if (!SYCL::instance().get_xpus().empty()) {
                if (f_calc_process.value().empty()) {
                    f_no_sycl = false;
                    f_calc_process = SYCL::instance().get_xpus().begin()->first;
                    f_calc_xpu = magic_enum::enum_cast<XPU>(SYCL::instance().get_xpus().begin()->second).value();
                    App::instance().commit_config();
                }
                goto has_device;
            }
        } catch (...) {}
        f_no_sycl = true;
        f_calc_process = "No device!";
        f_calc_xpu = CPU;
    }

has_device:
    if (!switch_device()) {
        MessageBeep(MB_ICONERROR);
    }
}

auto AppConfig::switch_device() const -> bool {
    if (f_no_sycl) {
        CALC_TBB::instance();
        return true;
    }

    if (!SYCL::instance().switch_device(f_calc_xpu, f_calc_process)) {
        LOG_WARNING << "选择硬件失败 -> " << magic_enum::enum_name<XPU>(f_calc_xpu) << "|" << f_calc_process.value();
        CALC_TBB::instance();
        return false;
    }

    LOG_INFO << "选择硬件成功 -> " << magic_enum::enum_name<XPU>(f_calc_xpu) << "|" << f_calc_process.value();
    CALC_SYCL::instance();
    return true;
}

auto AppConfig::config_get(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
    static util::Map<std::string, std::function<void()>> functions;
    static std::once_flag once;

    const flutter::EncodableMap& args = _method_call.arguments()->get<flutter::EncodableMap>();
    auto& field_name = args.at(flutter::EncodableValue("field_name")).get<std::string>();
    const auto that = instance();

    std::call_once(once,
                   [&] {
                       functions["f_auto_chek_update"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_auto_chek_update));
                       };

                       functions["f_check_file"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_check_file));
                       };

                       functions["f_calc_xpu"] = [&] {
                           _result->Success(flutter::EncodableValue(magic_enum::enum_name<XPU>(that->f_calc_xpu).data()));
                       };

                       functions["f_calc_process"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_calc_process));
                       };

                       functions["f_calc_xpu_options"] = [&] {
                           flutter::EncodableList list;
                           if (SYCL::instance().get_xpus().empty()) {
                               list.emplace_back("CPU");
                           } else {
                               std::vector<std::string> types;
                               for (auto& type : SYCL::instance().get_xpus() | std::views::values) {
                                   if (std::ranges::find(types, type) == types.end()) {
                                       list.emplace_back(type.data());
                                       types.push_back(type);
                                   }
                               }
                           }
                           _result->Success(flutter::EncodableValue(list));
                       };

                       functions["f_calc_process_options"] = [&] {
                           flutter::EncodableList list;
                           if (SYCL::instance().get_xpus().empty()) {
                               list.emplace_back("未找到受支持设备!");
                           } else {
                               for (auto& [name, type] : SYCL::instance().get_xpus()) {
                                   if (type == magic_enum::enum_name<XPU>(that->f_calc_xpu)) {
                                       list.emplace_back(name.data());
                                   }
                               }
                           }
                           _result->Success(flutter::EncodableValue(list));
                       };

                       functions["f_multi_thread"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_multi_thread));
                       };

                       functions["f_no_sycl"] = [&] {
                           _result->Success(flutter::EncodableValue(that->f_no_sycl));
                       };
                   });

    if (functions.contains(field_name)) {
        return functions[field_name]();
    }
    _result->NotImplemented();
}

auto AppConfig::config_set(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
    const flutter::EncodableMap& args = _method_call.arguments()->get<flutter::EncodableMap>();
    auto& field_name = args.at(flutter::EncodableValue("field_name")).get<std::string>();
    const auto that = instance();

    if (field_name == "f_auto_chek_update") {
        that->f_auto_chek_update = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_check_file") {
        that->f_check_file = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_calc_xpu") {
        const auto temp_name = that->f_calc_process.value();
        const auto temp_type = that->f_calc_xpu.value().value();
        const auto opt = magic_enum::enum_cast<XPU>(args.at(flutter::EncodableValue("value")).get<std::string>());
        if (!opt) {
            return _result->Success(flutter::EncodableValue(false));
        }
        that->f_calc_xpu = opt.value();

        for (auto& [name, type] : SYCL::instance().get_xpus()) {
            if (type == magic_enum::enum_name<XPU>(that->f_calc_xpu)) {
                that->f_calc_process = name;
            }
        }

        if (that->switch_device()) {
            _result->Success(flutter::EncodableValue(true));
        } else {
            that->f_calc_process = temp_name;
            that->f_calc_xpu = temp_type;
            _result->Success(flutter::EncodableValue(false));
        }
        goto _return_cs;
    }

    if (field_name == "f_calc_process") {
        const auto temp = that->f_calc_process.value();
        that->f_calc_process = args.at(flutter::EncodableValue("value")).get<std::string>();

        if (that->switch_device()) {
            _result->Success(flutter::EncodableValue(true));
        } else {
            that->f_calc_process = temp;
            _result->Success(flutter::EncodableValue(false));
        }
        goto _return_cs;
    }

    if (field_name == "f_multi_thread") {
        that->f_multi_thread = args.at(flutter::EncodableValue("value")).get<bool>();
        goto _return;
    }

    if (field_name == "f_no_sycl") {
        that->f_no_sycl = args.at(flutter::EncodableValue("value")).get<bool>();
        if (that->f_no_sycl) {
            CALC_TBB::instance();
        } else {
            try {
                if (!SYCL::instance().get_xpus().empty()) {
                    that->f_no_sycl = false;
                    that->f_calc_process = SYCL::instance().get_xpus().begin()->first;
                    that->f_calc_xpu = magic_enum::enum_cast<XPU>(SYCL::instance().get_xpus().begin()->second).value();
                    if (!that->switch_device()) {
                        MessageBeep(MB_ICONERROR);
                        goto _return_false;
                    }
                    goto _return;
                }
            } catch (...) {}
            goto _return_false;
        }
        goto _return;
    }

_return:
    _result->Success(flutter::EncodableValue(true));
_return_cs:
    App::instance().commit_config();
    return;
_return_false:
    _result->Success(flutter::EncodableValue(false));
}

auto AppConfig::invoke(const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result) -> void {
    static util::Map<std::string, std::function<void()>> functions;
    static std::once_flag once;

    const flutter::EncodableMap& args = _method_call.arguments()->get<flutter::EncodableMap>();
    auto& func_name = args.at(flutter::EncodableValue("func_name")).get<std::string>();

    std::call_once(once,
                   [&] {
                       functions["game_is_run"] = [&] {
                           bool found = is_process_running(L"Sausage Man.exe");
                           _result->Success(flutter::EncodableValue(found));
                       };

                       functions["inject"] = [&] {
                           launch_injector();
                           _result->Success();
                       };

                       functions["close"] = [&] {
                           if (g_injector_handle) {
                               TerminateProcess(g_injector_handle, 0);
                               CloseHandle(g_injector_handle);
                               g_injector_handle = nullptr;
                           }
                           g_injector_pid = 0;
                           _result->Success(flutter::EncodableValue(true));
                       };
                   });

    if (functions.contains(func_name)) {
        return functions[func_name]();
    }
    _result->NotImplemented();
}
