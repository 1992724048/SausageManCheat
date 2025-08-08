// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_METHOD_CODEC_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_METHOD_CODEC_H_

#include <memory>

#include "encodable_value.h"
#include "method_call.h"
#include "method_codec.h"
#include "standard_codec_serializer.h"

namespace flutter {
    // An implementation of MethodCodec that uses a binary serialization.
    class StandardMethodCodec : public MethodCodec<EncodableValue> {
    public:
        // Returns an instance of the codec, optionally using a custom serializer to
        // add support for more types.
        //
        // If provided, |serializer| must be long-lived. If no serializer is provided,
        // the default will be used.
        //
        // The instance returned for a given |extension| will be shared, and
        // any instance returned from this will be long-lived, and can be safely
        // passed to, e.g., channel constructors.
        static auto GetInstance(const StandardCodecSerializer* serializer = nullptr) -> const StandardMethodCodec&;

        ~StandardMethodCodec() override;

        // Prevent copying.
        StandardMethodCodec(const StandardMethodCodec&) = delete;
        auto operator=(const StandardMethodCodec&) -> StandardMethodCodec& = delete;

    protected:
        // |flutter::MethodCodec|
        auto DecodeMethodCallInternal(const uint8_t* message, size_t message_size) const -> std::unique_ptr<MethodCall<EncodableValue>> override;

        // |flutter::MethodCodec|
        auto EncodeMethodCallInternal(const MethodCall<EncodableValue>& method_call) const -> std::unique_ptr<std::vector<uint8_t>> override;

        // |flutter::MethodCodec|
        auto EncodeSuccessEnvelopeInternal(const EncodableValue* result) const -> std::unique_ptr<std::vector<uint8_t>> override;

        // |flutter::MethodCodec|
        auto EncodeErrorEnvelopeInternal(const std::string& error_code, const std::string& error_message, const EncodableValue* error_details) const -> std::unique_ptr<std::vector<uint8_t>> override;

        // |flutter::MethodCodec|
        auto DecodeAndProcessResponseEnvelopeInternal(const uint8_t* response, size_t response_size, MethodResult<EncodableValue>* result) const -> bool override;

    private:
        // Instances should be obtained via GetInstance.
        explicit StandardMethodCodec(const StandardCodecSerializer* serializer);

        const StandardCodecSerializer* serializer_;
    };
} // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_METHOD_CODEC_H_
