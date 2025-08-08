// This must be included before many other Windows headers.
#include "window_manager_plugin.h"

auto IsWindows11OrGreater() -> bool {
    DWORD dwVersion = 0;
    DWORD dwBuild = 0;

#pragma warning(push)
#pragma warning(disable : 4996)
    dwVersion = GetVersion();
    // Get the build number.
    if (dwVersion < 0x80000000) {
        dwBuild = static_cast<DWORD>((HIWORD(dwVersion)));
    }
#pragma warning(pop)

    return dwBuild < 22000;
}

// static
auto WindowManagerPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows* _registrar) -> void {
    auto plugin = std::make_unique<WindowManagerPlugin>(_registrar);
    _registrar->AddPlugin(std::move(plugin));
}

WindowManagerPlugin::WindowManagerPlugin() : registrar{nullptr} {}

WindowManagerPlugin::WindowManagerPlugin(flutter::PluginRegistrarWindows* _registrar) : registrar(_registrar) {
    window_manager = new WindowManager();
    window_proc_id = _registrar->RegisterTopLevelWindowProcDelegate([this](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        return HandleWindowProc(hWnd, message, wParam, lParam);
    });
    channel = std::make_unique<flutter::MethodChannel<>>(_registrar->messenger(), "window_manager", &flutter::StandardMethodCodec::GetInstance());

    channel->SetMethodCallHandler([this](const auto& call, auto result) {
        HandleMethodCall(call, std::move(result));
    });
}

WindowManagerPlugin::~WindowManagerPlugin() {
    registrar->UnregisterTopLevelWindowProcDelegate(window_proc_id);
    channel = nullptr;
}

auto WindowManagerPlugin::_EmitEvent(std::string eventName) const -> void {
    if (channel == nullptr) {
        return;
    }
    auto args = flutter::EncodableMap();
    args[flutter::EncodableValue("eventName")] = flutter::EncodableValue(eventName);
    channel->InvokeMethod("onEvent", std::make_unique<flutter::EncodableValue>(args));
}

auto WindowManagerPlugin::HandleWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> std::optional<LRESULT> {
    std::optional<LRESULT> result = std::nullopt;

    if (message == WM_DPICHANGED) {
        window_manager->pixel_ratio_ = static_cast<float>(LOWORD(wParam)) / USER_DEFAULT_SCREEN_DPI;
        window_manager->ForceChildRefresh();
    }

    if (wParam && message == WM_NCCALCSIZE) {
        if (window_manager->IsFullScreen() && window_manager->title_bar_style_ != "normal") {
            if (window_manager->is_frameless_) {
                adjust_nccalcsize(hWnd, reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam));
            }
            return 0;
        }
        // This must always be before handling title_bar_style_ == "hidden" so
        // the `if TitleBarStyle.hidden` doesn't get executed.
        if (window_manager->is_frameless_) {
            if (window_manager->IsMaximized()) {
                adjust_nccalcsize(hWnd, reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam));
            }
            return 0;
        }

        // This must always be last.
        if (wParam && window_manager->title_bar_style_ == "hidden") {
            if (window_manager->IsMaximized()) {
                // Adjust the borders when maximized so the app isn't cut off
                adjust_nccalcsize(hWnd, reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam));
            } else {
                const auto sz = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                // on windows 10, if set to 0, there's a white line at the top
                // of the app and I've yet to find a way to remove that.
                sz->rgrc[0].top += IsWindows11OrGreater() ? 0 : 1;
                // The following lines are required for resizing the window.
                // https://github.com/leanflutter/window_manager/issues/483  
                sz->rgrc[0].right -= 8;
                sz->rgrc[0].bottom -= 8;
                sz->rgrc[0].left -= -8;
            }

            // Previously (WVR_HREDRAW | WVR_VREDRAW), but returning 0 or 1 doesn't
            // actually break anything so I've set it to 0. Unless someone pointed a
            // problem in the future.
            return 0;
        }
    } else if (message == WM_NCHITTEST) {
        if (!window_manager->is_resizable_) {
            return HTNOWHERE;
        }
    } else if (message == WM_GETMINMAXINFO) {
        const auto info = reinterpret_cast<MINMAXINFO*>(lParam);
        // For the special "unconstrained" values, leave the defaults.
        if (window_manager->minimum_size_.x != 0) {
            info->ptMinTrackSize.x = static_cast<LONG>(window_manager->minimum_size_.x * window_manager->pixel_ratio_);
        }
        if (window_manager->minimum_size_.y != 0) {
            info->ptMinTrackSize.y = static_cast<LONG>(window_manager->minimum_size_.y * window_manager->pixel_ratio_);
        }
        if (window_manager->maximum_size_.x != -1) {
            info->ptMaxTrackSize.x = static_cast<LONG>(window_manager->maximum_size_.x * window_manager->pixel_ratio_);
        }
        if (window_manager->maximum_size_.y != -1) {
            info->ptMaxTrackSize.y = static_cast<LONG>(window_manager->maximum_size_.y * window_manager->pixel_ratio_);
        }
        result = 0;
    } else if (message == WM_NCACTIVATE) {
        if (wParam != 0) {
            _EmitEvent("focus");
        } else {
            _EmitEvent("blur");
        }

        if (window_manager->title_bar_style_ == "hidden" || window_manager->is_frameless_) {
            return 1;
        }
    } else if (message == WM_EXITSIZEMOVE) {
        if (window_manager->is_resizing_) {
            _EmitEvent("resized");
            window_manager->is_resizing_ = false;
        }
        if (window_manager->is_moving_) {
            _EmitEvent("moved");
            window_manager->is_moving_ = false;
        }
        return false;
    } else if (message == WM_MOVING) {
        window_manager->is_moving_ = true;
        _EmitEvent("move");
        return false;
    } else if (message == WM_SIZING) {
        window_manager->is_resizing_ = true;
        _EmitEvent("resize");

        if (window_manager->aspect_ratio_ > 0) {
            const auto rect = (LPRECT)lParam;

            const double aspect_ratio = window_manager->aspect_ratio_;

            int new_width = static_cast<int>(rect->right - rect->left);
            int new_height = static_cast<int>(rect->bottom - rect->top);

            const bool is_resizing_horizontally = wParam == WMSZ_LEFT || wParam == WMSZ_RIGHT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT;

            if (is_resizing_horizontally) {
                new_height = static_cast<int>(new_width / aspect_ratio);
            } else {
                new_width = static_cast<int>(new_height * aspect_ratio);
            }

            int left = rect->left;
            int top = rect->top;
            int right = rect->right;
            int bottom = rect->bottom;

            switch (wParam) {
                case WMSZ_RIGHT:
                case WMSZ_BOTTOM:
                    right = new_width + left;
                    bottom = top + new_height;
                    break;
                case WMSZ_TOP:
                    right = new_width + left;
                    top = bottom - new_height;
                    break;
                case WMSZ_LEFT:
                case WMSZ_TOPLEFT:
                    left = right - new_width;
                    top = bottom - new_height;
                    break;
                case WMSZ_TOPRIGHT:
                    right = left + new_width;
                    top = bottom - new_height;
                    break;
                case WMSZ_BOTTOMLEFT:
                    left = right - new_width;
                    bottom = top + new_height;
                    break;
                case WMSZ_BOTTOMRIGHT:
                    right = left + new_width;
                    bottom = top + new_height;
                    break;
            }

            rect->left = left;
            rect->top = top;
            rect->right = right;
            rect->bottom = bottom;
        }
    } else if (message == WM_SIZE) {
        if (window_manager->IsFullScreen() && wParam == SIZE_MAXIMIZED && window_manager->last_state != STATE_FULLSCREEN_ENTERED) {
            _EmitEvent("enter-full-screen");
            window_manager->last_state = STATE_FULLSCREEN_ENTERED;
        } else if (!window_manager->IsFullScreen() && wParam == SIZE_RESTORED && window_manager->last_state == STATE_FULLSCREEN_ENTERED) {
            window_manager->ForceChildRefresh();
            _EmitEvent("leave-full-screen");
            window_manager->last_state = STATE_NORMAL;
        } else if (window_manager->last_state != STATE_FULLSCREEN_ENTERED) {
            if (wParam == SIZE_MAXIMIZED) {
                _EmitEvent("maximize");
                window_manager->last_state = STATE_MAXIMIZED;
            } else if (wParam == SIZE_MINIMIZED) {
                _EmitEvent("minimize");
                window_manager->last_state = STATE_MINIMIZED;
                return 0;
            } else if (wParam == SIZE_RESTORED) {
                if (window_manager->last_state == STATE_MAXIMIZED) {
                    _EmitEvent("unmaximize");
                    window_manager->last_state = STATE_NORMAL;
                } else if (window_manager->last_state == STATE_MINIMIZED) {
                    _EmitEvent("restore");
                    window_manager->last_state = STATE_NORMAL;
                }
            }
        }
    } else if (message == WM_CLOSE) {
        _EmitEvent("close");
        if (window_manager->IsPreventClose()) {
            return -1;
        }
    } else if (message == WM_SHOWWINDOW) {
        if (wParam == TRUE) {
            _EmitEvent("show");
        } else {
            _EmitEvent("hide");
        }
    } else if (message == WM_WINDOWPOSCHANGED) {
        if (window_manager->IsAlwaysOnBottom()) {
            const flutter::EncodableMap& args = {{flutter::EncodableValue("isAlwaysOnBottom"), flutter::EncodableValue(true)}};
            window_manager->SetAlwaysOnBottom(args);
        }
    }

    return result;
}

auto WindowManagerPlugin::HandleMethodCall(const flutter::MethodCall<>& method_call, const std::unique_ptr<flutter::MethodResult<>>& _result) const -> void {
    const std::string& method_name = method_call.method_name();

    if (method_name == "ensureIntialized") {
        window_manager->native_window = GetAncestor(registrar->GetView()->GetNativeWindow(), GA_ROOT);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "waitUntilReadyToShow") {
        window_manager->WaitUntilReadyToShow();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "getId") {
        _result->Success(flutter::EncodableValue(reinterpret_cast<__int64>(window_manager->GetMainWindow())));
    } else if (method_name == "setAsFrameless") {
        window_manager->SetAsFrameless();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "destroy") {
        WindowManager::Destroy();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "close") {
        window_manager->Close();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isPreventClose") {
        auto value = window_manager->IsPreventClose();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setPreventClose") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetPreventClose(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "focus") {
        window_manager->Focus();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "blur") {
        window_manager->Blur();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isFocused") {
        bool value = window_manager->IsFocused();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "show") {
        window_manager->Show();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "hide") {
        window_manager->Hide();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isVisible") {
        bool value = window_manager->IsVisible();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "isMaximized") {
        bool value = window_manager->IsMaximized();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "maximize") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->Maximize(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "unmaximize") {
        window_manager->Unmaximize();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isMinimized") {
        bool value = window_manager->IsMinimized();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "minimize") {
        window_manager->Minimize();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "restore") {
        window_manager->Restore();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isDockable") {
        _result->Success(flutter::EncodableValue(false));
    } else if (method_name == "isDocked") {
        _result->Success(flutter::EncodableValue(false));
    } else if (method_name == "dock") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->Dock(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "undock") {
        bool value = window_manager->Undock();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "isFullScreen") {
        bool value = window_manager->IsFullScreen();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setFullScreen") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetFullScreen(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setAspectRatio") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetAspectRatio(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setBackgroundColor") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetBackgroundColor(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "getBounds") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        flutter::EncodableMap value = window_manager->GetBounds(args);
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setBounds") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetBounds(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setMinimumSize") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetMinimumSize(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setMaximumSize") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetMaximumSize(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isResizable") {
        bool value = window_manager->IsResizable();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setResizable") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetResizable(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isMinimizable") {
        bool value = window_manager->IsMinimizable();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setMinimizable") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetMinimizable(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isMaximizable") {
        bool value = window_manager->IsMaximizable();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setMaximizable") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetMaximizable(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isClosable") {
        bool value = window_manager->IsClosable();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setClosable") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetClosable(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isAlwaysOnTop") {
        bool value = window_manager->IsAlwaysOnTop();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setAlwaysOnTop") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetAlwaysOnTop(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "isAlwaysOnBottom") {
        bool value = window_manager->IsAlwaysOnBottom();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setAlwaysOnBottom") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetAlwaysOnBottom(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "getTitle") {
        std::string value = window_manager->GetTitle();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setTitle") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetTitle(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setTitleBarStyle") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetTitleBarStyle(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "getTitleBarHeight") {
        int value = window_manager->GetTitleBarHeight();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "isSkipTaskbar") {
        bool value = window_manager->IsSkipTaskbar();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setSkipTaskbar") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetSkipTaskbar(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setProgressBar") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetProgressBar(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setIcon") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetIcon(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "hasShadow") {
        bool value = window_manager->HasShadow();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setHasShadow") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetHasShadow(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "getOpacity") {
        double value = window_manager->GetOpacity();
        _result->Success(flutter::EncodableValue(value));
    } else if (method_name == "setOpacity") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetOpacity(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setBrightness") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetBrightness(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "setIgnoreMouseEvents") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->SetIgnoreMouseEvents(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "popUpWindowMenu") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->PopUpWindowMenu(args);
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "startDragging") {
        window_manager->StartDragging();
        _result->Success(flutter::EncodableValue(true));
    } else if (method_name == "startResizing") {
        const flutter::EncodableMap& args = method_call.arguments()->get<flutter::EncodableMap>();
        window_manager->StartResizing(args);
        _result->Success(flutter::EncodableValue(true));
    } else {
        _result->NotImplemented();
    }
}
