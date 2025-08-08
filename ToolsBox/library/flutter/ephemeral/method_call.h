// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_METHOD_CALL_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_METHOD_CALL_H_

#include <memory>
#include <string>

namespace flutter {
    class EncodableValue;

    // An object encapsulating a method call from Flutter whose arguments are of
    // type T.
    template<typename T = EncodableValue>
    class MethodCall {
    public:
        // Creates a MethodCall with the given name and arguments.
        MethodCall(std::string method_name, std::unique_ptr<T> arguments) : method_name_(std::move(method_name)), arguments_(std::move(arguments)) {}

        virtual ~MethodCall() = default;

        // Prevent copying.
        MethodCall(const MethodCall<T>&) = delete;
        auto operator=(const MethodCall<T>&) -> MethodCall& = delete;

        // The name of the method being called.
        [[nodiscard]] auto method_name() const -> const std::string& {
            return method_name_;
        }

        // The arguments to the method call, or NULL if there are none.
        [[nodiscard]] auto arguments() const -> const T* {
            return arguments_.get();
        }

    private:
        std::string method_name_;
        std::unique_ptr<T> arguments_;
    };
} // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_METHOD_CALL_H_
