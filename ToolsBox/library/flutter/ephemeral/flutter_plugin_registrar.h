// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_PUBLIC_FLUTTER_PLUGIN_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_PUBLIC_FLUTTER_PLUGIN_REGISTRAR_H_

#include <stddef.h>
#include <stdint.h>

#include "flutter_export.h"
#include "flutter_messenger.h"
#include "flutter_texture_registrar.h"

#if defined(__cplusplus)
extern "C" {
#endif  // defined(__cplusplus)

    // Opaque reference to a plugin registrar.
    using FlutterDesktopPluginRegistrarRef = struct FlutterDesktopPluginRegistrar*;

    // Function pointer type for registrar destruction callback.
    using FlutterDesktopOnPluginRegistrarDestroyed = void(*)(FlutterDesktopPluginRegistrarRef);

    // Returns the engine messenger associated with this registrar.
    FLUTTER_EXPORT auto FlutterDesktopPluginRegistrarGetMessenger(FlutterDesktopPluginRegistrarRef registrar) -> FlutterDesktopMessengerRef;

    // Returns the texture registrar associated with this registrar.
    FLUTTER_EXPORT auto FlutterDesktopRegistrarGetTextureRegistrar(FlutterDesktopPluginRegistrarRef registrar) -> FlutterDesktopTextureRegistrarRef;

    // Registers a callback to be called when the plugin registrar is destroyed.
    FLUTTER_EXPORT auto FlutterDesktopPluginRegistrarSetDestructionHandler(FlutterDesktopPluginRegistrarRef registrar, FlutterDesktopOnPluginRegistrarDestroyed callback) -> void;

#if defined(__cplusplus)
} // extern "C"
#endif

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_PUBLIC_FLUTTER_PLUGIN_REGISTRAR_H_
