// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_CODEC_SERIALIZER_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_CODEC_SERIALIZER_H_

#include "byte_streams.h"
#include "encodable_value.h"

namespace flutter {
    // Encapsulates the logic for encoding/decoding EncodableValues to/from the
    // standard codec binary representation.
    //
    // This can be subclassed to extend the standard codec with support for new
    // types.
    class StandardCodecSerializer {
    public:
        virtual ~StandardCodecSerializer();

        // Returns the shared serializer instance.
        static auto GetInstance() -> const StandardCodecSerializer&;

        // Prevent copying.
        StandardCodecSerializer(const StandardCodecSerializer&) = delete;
        auto operator=(const StandardCodecSerializer&) -> StandardCodecSerializer& = delete;

        // Reads and returns the next value from |stream|.
        auto ReadValue(ByteStreamReader* stream) const -> EncodableValue;

        // Writes the encoding of |value| to |stream|, including the initial type
        // discrimination byte.
        //
        // Can be overridden by a subclass to extend the codec.
        virtual auto WriteValue(const EncodableValue& value, ByteStreamWriter* stream) const -> void;

    protected:
        // Codecs require long-lived serializers, so clients should always use
        // GetInstance().
        StandardCodecSerializer();

        // Reads and returns the next value from |stream|, whose discrimination byte
        // was |type|.
        //
        // The discrimination byte will already have been read from the stream when
        // this is called.
        //
        // Can be overridden by a subclass to extend the codec.
        virtual auto ReadValueOfType(uint8_t type, ByteStreamReader* stream) const -> EncodableValue;

        // Reads the variable-length size from the current position in |stream|.
        auto ReadSize(ByteStreamReader* stream) const -> size_t;

        // Writes the variable-length size encoding to |stream|.
        auto WriteSize(size_t size, ByteStreamWriter* stream) const -> void;

    private:
        // Reads a fixed-type list whose values are of type T from the current
        // position in |stream|, and returns it as the corresponding EncodableValue.
        // |T| must correspond to one of the supported list value types of
        // EncodableValue.
        template<typename T>
        auto ReadVector(ByteStreamReader* stream) const -> EncodableValue;

        // Writes |vector| to |stream| as a fixed-type list. |T| must correspond to
        // one of the supported list value types of EncodableValue.
        template<typename T>
        auto WriteVector(std::vector<T> vector, ByteStreamWriter* stream) const -> void;
    };
} // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_STANDARD_CODEC_SERIALIZER_H_
