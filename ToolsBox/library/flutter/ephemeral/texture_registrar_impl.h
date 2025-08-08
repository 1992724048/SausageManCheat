// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_TEXTURE_REGISTRAR_IMPL_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_TEXTURE_REGISTRAR_IMPL_H_

#include "texture_registrar.h"

namespace flutter {
    // Wrapper around a FlutterDesktopTextureRegistrarRef that implements the
    // TextureRegistrar API.
    class TextureRegistrarImpl : public TextureRegistrar {
    public:
        explicit TextureRegistrarImpl(FlutterDesktopTextureRegistrarRef texture_registrar_ref);
        ~TextureRegistrarImpl() override;

        // Prevent copying.
        TextureRegistrarImpl(const TextureRegistrarImpl&) = delete;
        auto operator=(const TextureRegistrarImpl&) -> TextureRegistrarImpl& = delete;

        // |flutter::TextureRegistrar|
        auto RegisterTexture(TextureVariant* texture) -> int64_t override;

        // |flutter::TextureRegistrar|
        auto MarkTextureFrameAvailable(int64_t texture_id) -> bool override;

        // |flutter::TextureRegistrar|
        auto UnregisterTexture(int64_t texture_id, std::function<void()> callback) -> void override;

        // |flutter::TextureRegistrar|
        auto UnregisterTexture(int64_t texture_id) -> bool override;

    private:
        // Handle for interacting with the C API.
        FlutterDesktopTextureRegistrarRef texture_registrar_ref_;
    };
} // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_TEXTURE_REGISTRAR_IMPL_H_
