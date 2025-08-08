#include "flutter_window.h"

#include <optional>

#include "FlutterPluginRegistry.h"

#include "../../../GPP32/Library/Logger.h"

FlutterWindow::FlutterWindow(const flutter::DartProject& project) : project_(project) {}

FlutterWindow::~FlutterWindow() {}

auto FlutterWindow::OnCreate() -> bool {
    if (!Win32Window::OnCreate()) {
        return false;
    }

    const auto [left, top, right, bottom] = GetClientArea();
    flutter_controller_ = std::make_unique<flutter::FlutterViewController>(right - left, bottom - top, project_);
    if (!flutter_controller_->engine() || !flutter_controller_->view()) {
        return false;
    }
   
    for (const auto& [name, callback] : PluginRegistry::GetInstance().GetPlugins()) {
        LOG_DEBUG << "registry pulgin -> " << name;
        callback(flutter_controller_->engine()->GetRegistrarForPlugin(name));
    }
    SetChildContent(flutter_controller_->view()->GetNativeWindow());

    flutter_controller_->engine()->SetNextFrameCallback([&]() {
        this->Show();
    });

    flutter_controller_->ForceRedraw();
    return true;
}

auto FlutterWindow::OnDestroy() -> void {
    if (flutter_controller_) {
        flutter_controller_ = nullptr;
    }

    Win32Window::OnDestroy();
}

auto FlutterWindow::MessageHandler(HWND hwnd, const UINT message, const WPARAM wparam, const LPARAM lparam) noexcept -> LRESULT {
    if (flutter_controller_) {
        const std::optional<LRESULT> result = flutter_controller_->HandleTopLevelWindowProc(hwnd, message, wparam, lparam);
        if (result) {
            return *result;
        }
    }

    switch (message) {
        case WM_FONTCHANGE:
            flutter_controller_->engine()->ReloadSystemFonts();
            break;
        default: ;
    }

    return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}
