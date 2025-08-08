// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_SINK_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_SINK_H_

namespace flutter {
    class EncodableValue;

    // Event callback. Events to be sent to Flutter application
    // act as clients of this interface for sending events.
    template<typename T = EncodableValue>
    class EventSink {
    public:
        EventSink() = default;
        virtual ~EventSink() = default;

        // Prevent copying.
        EventSink(const EventSink&) = delete;
        auto operator=(const EventSink&) -> EventSink& = delete;

        // Consumes a successful event
        auto Success(const T& event) -> void {
            SuccessInternal(&event);
        }

        // Consumes a successful event.
        auto Success() -> void {
            SuccessInternal(nullptr);
        }

        // Consumes an error event.
        auto Error(const std::string& error_code, const std::string& error_message, const T& error_details) -> void {
            ErrorInternal(error_code, error_message, &error_details);
        }

        // Consumes an error event.
        auto Error(const std::string& error_code, const std::string& error_message = "") -> void {
            ErrorInternal(error_code, error_message, nullptr);
        }

        // Consumes end of stream. Ensuing calls to Success() or
        // Error(), if any, are ignored.
        auto EndOfStream() -> void {
            EndOfStreamInternal();
        }

    protected:
        // Implementation of the public interface, to be provided by subclasses.
        virtual auto SuccessInternal(const T* event = nullptr) -> void = 0;

        // Implementation of the public interface, to be provided by subclasses.
        virtual auto ErrorInternal(const std::string& error_code, const std::string& error_message, const T* error_details) -> void = 0;

        // Implementation of the public interface, to be provided by subclasses.
        virtual auto EndOfStreamInternal() -> void = 0;
    };
} // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_EVENT_SINK_H_
