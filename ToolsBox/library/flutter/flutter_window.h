#ifndef RUNNER_FLUTTER_WINDOW_H_
#define RUNNER_FLUTTER_WINDOW_H_

#include "ephemeral/dart_project.h"
#include "ephemeral/flutter_view_controller.h"

#include <memory>

#include "win32_window.h"

#include "events/event.hpp"

// A window that does nothing but host a Flutter view.
class FlutterWindow : public Win32Window {
public:
    // Creates a new FlutterWindow hosting a Flutter view running |project|.
    explicit FlutterWindow(const flutter::DartProject& project);
    ~FlutterWindow() override;
protected:
    // Win32Window:
    auto OnCreate() -> bool override;
    auto OnDestroy() -> void override;
    auto MessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT override;

private:
    // The project to run.
    flutter::DartProject project_;

    // The Flutter instance hosted by this window.
    std::unique_ptr<flutter::FlutterViewController> flutter_controller_;
};

#endif  // RUNNER_FLUTTER_WINDOW_H_
