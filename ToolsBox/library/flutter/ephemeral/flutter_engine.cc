// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_engine.h"

#include <algorithm>
#include <iostream>

#include "binary_messenger_impl.h"
#include "flutter_windows.h"

namespace flutter {
    FlutterEngine::FlutterEngine(const DartProject& _project) {
        FlutterDesktopEngineProperties c_engine_properties = {};
        c_engine_properties.assets_path = _project.assets_path().c_str();
        c_engine_properties.icu_data_path = _project.icu_data_path().c_str();
        c_engine_properties.aot_library_path = _project.aot_library_path().c_str();
        c_engine_properties.dart_entrypoint = _project.dart_entrypoint().c_str();
        c_engine_properties.gpu_preference = static_cast<FlutterDesktopGpuPreference>(_project.gpu_preference());
        c_engine_properties.ui_thread_policy = static_cast<FlutterDesktopUIThreadPolicy>(_project.ui_thread_policy());

        const std::vector<std::string>& entrypoint_args = _project.dart_entrypoint_arguments();
        std::vector<const char*> entrypoint_argv;
        std::ranges::transform(entrypoint_args,
                               std::back_inserter(entrypoint_argv),
                               [](const std::string& arg) -> const char* {
                                   return arg.c_str();
                               });

        c_engine_properties.dart_entrypoint_argc = static_cast<int>(entrypoint_argv.size());
        c_engine_properties.dart_entrypoint_argv = entrypoint_argv.empty() ? nullptr : entrypoint_argv.data();

        engine_ = FlutterDesktopEngineCreate(&c_engine_properties);

        auto core_messenger = FlutterDesktopEngineGetMessenger(engine_);
        messenger_ = std::make_unique<BinaryMessengerImpl>(core_messenger);
    }

    FlutterEngine::~FlutterEngine() {
        ShutDown();
    }

    auto FlutterEngine::Run() -> bool {
        return Run(nullptr);
    }

    auto FlutterEngine::Run(const char* entry_point) -> bool {
        if (!engine_) {
            std::cerr << "Cannot run an engine that failed creation." << std::endl;
            return false;
        }
        if (run_succeeded_) {
            std::cerr << "Cannot run an engine more than once." << std::endl;
            return false;
        }
        bool run_succeeded = FlutterDesktopEngineRun(engine_, entry_point);
        if (!run_succeeded) {
            std::cerr << "Failed to start engine." << std::endl;
        }
        run_succeeded_ = true;
        return run_succeeded;
    }

    auto FlutterEngine::ShutDown() -> void {
        if (engine_ && owns_engine_) {
            FlutterDesktopEngineDestroy(engine_);
        }
        engine_ = nullptr;
    }

    auto FlutterEngine::ProcessMessages() const -> std::chrono::nanoseconds {
        return std::chrono::nanoseconds(FlutterDesktopEngineProcessMessages(engine_));
    }

    auto FlutterEngine::ReloadSystemFonts() const -> void {
        FlutterDesktopEngineReloadSystemFonts(engine_);
    }

    auto FlutterEngine::GetRegistrarForPlugin(const std::string& plugin_name) -> FlutterDesktopPluginRegistrarRef {
        if (!engine_) {
            std::cerr << "Cannot get plugin registrar on an engine that isn't running; " "call Run first." << std::endl;
            return nullptr;
        }
        return FlutterDesktopEngineGetPluginRegistrar(engine_, plugin_name.c_str());
    }

    auto FlutterEngine::SetNextFrameCallback(std::function<void()> callback) -> void {
        next_frame_callback_ = std::move(callback);
        FlutterDesktopEngineSetNextFrameCallback(engine_,
                                                 [](void* user_data) {
                                                     auto self = static_cast<FlutterEngine*>(user_data);
                                                     self->next_frame_callback_();
                                                     self->next_frame_callback_ = nullptr;
                                                 },
                                                 this);
    }

    auto FlutterEngine::ProcessExternalWindowMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) const -> std::optional<LRESULT> {
        LRESULT result;
        if (FlutterDesktopEngineProcessExternalWindowMessage(engine_, hwnd, message, wparam, lparam, &result)) {
            return result;
        }
        return std::nullopt;
    }

    auto FlutterEngine::RelinquishEngine() -> FlutterDesktopEngineRef {
        owns_engine_ = false;
        return engine_;
    }
} // namespace flutter
