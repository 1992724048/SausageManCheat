// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_METHOD_CODEC_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_METHOD_CODEC_H_

#include <memory>
#include <string>
#include <vector>

#include "method_call.h"
#include "method_result.h"

namespace flutter {
    // Translates between a binary message and higher-level method call and
    // response/error objects.
    template<typename T>
    class MethodCodec {
    public:
        MethodCodec() = default;

        virtual ~MethodCodec() = default;

        // Prevent copying.
        MethodCodec(const MethodCodec<T>&) = delete;
        auto operator=(const MethodCodec<T>&) -> MethodCodec& = delete;

        // Returns the MethodCall encoded in |message|, or nullptr if it cannot be
        // decoded.
        auto DecodeMethodCall(const uint8_t* message, size_t message_size) const -> std::unique_ptr<MethodCall<T>> {
            return std::move(DecodeMethodCallInternal(message, message_size));
        }

        // Returns the MethodCall encoded in |message|, or nullptr if it cannot be
        // decoded.
        auto DecodeMethodCall(const std::vector<uint8_t>& message) const -> std::unique_ptr<MethodCall<T>> {
            size_t size = message.size();
            const uint8_t* data = size > 0 ? &message[0] : nullptr;
            return std::move(DecodeMethodCallInternal(data, size));
        }

        // Returns a binary encoding of the given |method_call|, or nullptr if the
        // method call cannot be serialized by this codec.
        auto EncodeMethodCall(const MethodCall<T>& method_call) const -> std::unique_ptr<std::vector<uint8_t>> {
            return std::move(EncodeMethodCallInternal(method_call));
        }

        // Returns a binary encoding of |result|. |result| must be a type supported
        // by the codec.
        auto EncodeSuccessEnvelope(const T* result = nullptr) const -> std::unique_ptr<std::vector<uint8_t>> {
            return std::move(EncodeSuccessEnvelopeInternal(result));
        }

        // Returns a binary encoding of |error|. The |error_details| must be a type
        // supported by the codec.
        auto EncodeErrorEnvelope(const std::string& error_code, const std::string& error_message = "", const T* error_details = nullptr) const -> std::unique_ptr<std::vector<uint8_t>> {
            return std::move(EncodeErrorEnvelopeInternal(error_code, error_message, error_details));
        }

        // Decodes the response envelope encoded in |response|, calling the
        // appropriate method on |result|.
        //
        // Returns false if |response| cannot be decoded. In that case the caller is
        // responsible for calling a |result| method.
        auto DecodeAndProcessResponseEnvelope(const uint8_t* response, size_t response_size, MethodResult<T>* result) const -> bool {
            return DecodeAndProcessResponseEnvelopeInternal(response, response_size, result);
        }

    protected:
        // Implementation of the public interface, to be provided by subclasses.
        virtual auto DecodeMethodCallInternal(const uint8_t* message, size_t message_size) const -> std::unique_ptr<MethodCall<T>> = 0;

        // Implementation of the public interface, to be provided by subclasses.
        virtual auto EncodeMethodCallInternal(const MethodCall<T>& method_call) const -> std::unique_ptr<std::vector<uint8_t>> = 0;

        // Implementation of the public interface, to be provided by subclasses.
        virtual auto EncodeSuccessEnvelopeInternal(const T* result) const -> std::unique_ptr<std::vector<uint8_t>> = 0;

        // Implementation of the public interface, to be provided by subclasses.
        virtual auto EncodeErrorEnvelopeInternal(const std::string& error_code, const std::string& error_message, const T* error_details) const -> std::unique_ptr<std::vector<uint8_t>> = 0;

        // Implementation of the public interface, to be provided by subclasses.
        virtual auto DecodeAndProcessResponseEnvelopeInternal(const uint8_t* response, size_t response_size, MethodResult<T>* result) const -> bool = 0;
    };
} // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_METHOD_CODEC_H_
