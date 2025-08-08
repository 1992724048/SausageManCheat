// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_view_controller.h"

#include <algorithm>
#include <iostream>

namespace flutter {
    FlutterViewController::FlutterViewController(const int width, const int height, const DartProject& project) {
        engine_ = std::make_shared<FlutterEngine>(project);
        controller_ = FlutterDesktopViewControllerCreate(width, height, engine_->RelinquishEngine());
        if (!controller_) {
            std::cerr << "Failed to create view controller." << std::endl;
            return;
        }
        view_ = std::make_unique<FlutterView>(FlutterDesktopViewControllerGetView(controller_));
    }

    FlutterViewController::~FlutterViewController() {
        if (controller_) {
            FlutterDesktopViewControllerDestroy(controller_);
        }
    }

    auto FlutterViewController::view_id() const -> FlutterViewId {
        auto view_id = FlutterDesktopViewControllerGetViewId(controller_);

        return view_id;
    }

    auto FlutterViewController::ForceRedraw() const -> void {
        FlutterDesktopViewControllerForceRedraw(controller_);
    }

    auto FlutterViewController::HandleTopLevelWindowProc(const HWND hwnd, const UINT message, const WPARAM wparam, const LPARAM lparam) const -> std::optional<LRESULT> {
        LRESULT result;
        const bool handled = FlutterDesktopViewControllerHandleTopLevelWindowProc(controller_, hwnd, message, wparam, lparam, &result);
        return handled ? result : std::optional<LRESULT>(std::nullopt);
    }
} // namespace flutter
