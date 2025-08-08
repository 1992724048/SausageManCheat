// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plugin_registrar.h"

#include <iostream>
#include <map>

#include "binary_messenger_impl.h"
#include "engine_method_result.h"
#include "method_channel.h"
#include "texture_registrar_impl.h"

namespace flutter {
    // ===== PluginRegistrar =====

    PluginRegistrar::PluginRegistrar(FlutterDesktopPluginRegistrarRef registrar) : registrar_(registrar) {
        auto core_messenger = FlutterDesktopPluginRegistrarGetMessenger(registrar_);
        messenger_ = std::make_unique<BinaryMessengerImpl>(core_messenger);

        auto texture_registrar = FlutterDesktopRegistrarGetTextureRegistrar(registrar_);
        texture_registrar_ = std::make_unique<TextureRegistrarImpl>(texture_registrar);
    }

    PluginRegistrar::~PluginRegistrar() {
        // This must always be the first call.
        ClearPlugins();

        // Explicitly cleared to facilitate testing of destruction order.
        messenger_.reset();
    }

    auto PluginRegistrar::AddPlugin(std::unique_ptr<Plugin> plugin) -> void {
        plugins_.insert(std::move(plugin));
    }

    auto PluginRegistrar::ClearPlugins() -> void {
        plugins_.clear();
    }

    // ===== PluginRegistrarManager =====

    // static
    auto PluginRegistrarManager::GetInstance() -> PluginRegistrarManager* {
        static auto instance = new PluginRegistrarManager();
        return instance;
    }

    PluginRegistrarManager::PluginRegistrarManager() = default;

    // static
    auto PluginRegistrarManager::OnRegistrarDestroyed(FlutterDesktopPluginRegistrarRef registrar) -> void {
        GetInstance()->registrars()->erase(registrar);
    }
} // namespace flutter
