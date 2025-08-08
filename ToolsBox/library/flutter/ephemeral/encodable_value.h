#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_ENCODABLE_VALUE_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_ENCODABLE_VALUE_H_

#include <any>
#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <typeinfo>
#include <variant>
#include <vector>
#include <utility>

namespace flutter {
    // Forward declaration
    class EncodableValue;

    // Aliases for convenience
    using EncodableList = std::vector<EncodableValue>;
    using EncodableMap = std::map<EncodableValue, EncodableValue>;

    // Custom container for user-defined types
    class CustomEncodableValue {
    public:
        explicit CustomEncodableValue(const std::any& value) : value_(value) {}
        ~CustomEncodableValue() = default;

        // Access underlying value
        operator std::any&() {
            return value_;
        }

        operator const std::any&() const {
            return value_;
        }

        auto type() const noexcept -> const std::type_info& {
            return value_.type();
        }

        // For use in std::map keys only — not meaningful comparisons
        auto operator<(const CustomEncodableValue& other) const -> bool {
            return this < &other;
        }

        auto operator==(const CustomEncodableValue& other) const -> bool {
            return this == &other;
        }

    private:
        std::any value_;
    };

    // Internal variant type
    namespace internal {
        using EncodableValueVariant = std::variant<
            std::monostate,
            bool,
            int32_t,
            int64_t,
            double,
            std::string,
            std::vector<uint8_t>,
            std::vector<int32_t>,
            std::vector<int64_t>,
            std::vector<double>,
            EncodableList,
            EncodableMap,
            CustomEncodableValue,
            std::vector<float>>;
    } // namespace internal

    // A Flutter codec-compatible value container
    class EncodableValue {
    public:
        using VariantType = internal::EncodableValueVariant;

        EncodableValue() = default;

        // Implicit conversion for CustomEncodableValue
        EncodableValue(const CustomEncodableValue& v) : value_(v) {}

        // Avoid const char* -> bool conversion
        explicit EncodableValue(const char* str) : value_(std::string(str)) {}

        auto operator=(const char* str) -> EncodableValue& {
            value_ = std::string(str);
            return *this;
        }

        // Perfect forwarding constructor
        template<typename T> requires (!std::is_same_v<std::decay_t<T>, EncodableValue>)
        explicit EncodableValue(T&& val) : value_(std::forward<T>(val)) {}

        // Access variant
        [[nodiscard]] auto value() const -> const VariantType& {
            return value_;
        }

        auto value() -> VariantType& {
            return value_;
        }

        // Convenient holds/get access
        template<typename T>
        [[nodiscard]] auto is() const -> bool {
            return std::holds_alternative<T>(value_);
        }

        template<typename T>
        [[nodiscard]] auto get() const -> const T& {
            return std::get<T>(value_);
        }

        template<typename T>
        [[nodiscard]] auto get() -> T& {
            return std::get<T>(value_);
        }

        template<typename T>
        [[nodiscard]] auto get_if() -> T* {
            return std::get_if<T>(&value_);
        }

        [[nodiscard]] auto IsNull() const -> bool {
            return std::holds_alternative<std::monostate>(value_);
        }

        [[nodiscard]] auto LongValue() const -> int64_t {
            if (std::holds_alternative<int32_t>(value_)) {
                return std::get<int32_t>(value_);
            }
            return std::get<int64_t>(value_);
        }

        // Comparisons (needed for map key)
        friend auto operator<(const EncodableValue& lhs, const EncodableValue& rhs) -> bool {
            return lhs.value_ < rhs.value_;
        }

        friend auto operator==(const EncodableValue& lhs, const EncodableValue& rhs) -> bool {
            return lhs.value_ == rhs.value_;
        }

    private:
        VariantType value_;
    };
} // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_ENCODABLE_VALUE_H_
