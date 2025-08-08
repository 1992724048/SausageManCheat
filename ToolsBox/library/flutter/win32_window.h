#ifndef RUNNER_WIN32_WINDOW_H_
#define RUNNER_WIN32_WINDOW_H_

#include <windows.h>

#include <functional>
#include <memory>
#include <string>

// A class abstraction for a high DPI-aware Win32 Window. Intended to be
// inherited from by classes that wish to specialize with custom
// rendering and input handling
class Win32Window {
public:
    struct Point {
        unsigned int x;
        unsigned int y;
        Point(unsigned int x, unsigned int y) : x(x), y(y) {}
    };

    struct Size {
        unsigned int width;
        unsigned int height;
        Size(unsigned int width, unsigned int height) : width(width), height(height) {}
    };

    Win32Window();
    virtual ~Win32Window();

    // Creates a win32 window with |title| that is positioned and sized using
    // |origin| and |size|. New windows are created on the default monitor. Window
    // sizes are specified to the OS in physical pixels, hence to ensure a
    // consistent size this function will scale the inputted width and height as
    // as appropriate for the default monitor. The window is invisible until
    // |Show| is called. Returns true if the window was created successfully.
    auto Create(const std::wstring& _title, const Point& _origin, const Size& _size) -> bool;

    // Show the current window. Returns true if the window was successfully shown.
    auto Show() const -> bool;

    auto Show(bool show) const -> bool;

    // Release OS resources associated with window.
    auto Destroy() -> void;

    // Inserts |content| into the window tree.
    auto SetChildContent(HWND content) -> void;

    // Returns the backing Window handle to enable clients to set icon and other
    // window properties. Returns nullptr if the window has been destroyed.
    auto GetHandle() const -> HWND;

    // If true, closing this window will quit the application.
    auto SetQuitOnClose(bool quit_on_close) -> void;

    // Return a RECT representing the bounds of the current client area.
    auto GetClientArea() const -> RECT;

protected:
    // Processes and route salient window messages for mouse handling,
    // size change and DPI. Delegates handling of these to member overloads that
    // inheriting classes can handle.
    virtual auto MessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT;

    // Called when CreateAndShow is called, allowing subclass window-related
    // setup. Subclasses should return false if setup fails.
    virtual auto OnCreate() -> bool;

    // Called when Destroy is called.
    virtual auto OnDestroy() -> void;

private:
    friend class WindowClassRegistrar;

    // OS callback called by message pump. Handles the WM_NCCREATE message which
    // is passed when the non-client area is being created and enables automatic
    // non-client DPI scaling so that the non-client area automatically
    // responds to changes in DPI. All other messages are handled by
    // MessageHandler.
    static auto CALLBACK WndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT;

    // Retrieves a class instance pointer for |window|
    static auto GetThisFromHandle(HWND window) noexcept -> Win32Window*;

    // Update the window frame's theme to match the system theme.
    static auto UpdateTheme(HWND window) -> void;

    bool quit_on_close_ = false;

    // window handle for top level window.
    HWND window_handle_ = nullptr;

    // window handle for hosted content.
    HWND child_content_ = nullptr;
};

#endif  // RUNNER_WIN32_WINDOW_H_
