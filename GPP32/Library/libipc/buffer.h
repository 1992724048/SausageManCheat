#pragma once

#include <cstddef>
#include <tuple>
#include <vector>
#include <type_traits>

#include "libipc/export.h"
#include "libipc/def.h"

namespace ipc {
    class IPC_EXPORT buffer {
    public:
        using destructor_t = void (*)(void*, std::size_t);

        buffer();

        buffer(void* p, std::size_t s, destructor_t d);
        buffer(void* p, std::size_t s, destructor_t d, void* additional);
        buffer(void* p, std::size_t s);

        template<std::size_t N>
        explicit buffer(const byte_t (&data)[N]) : buffer(data, sizeof(data)) {}

        explicit buffer(const char& c);

        buffer(buffer&& rhs);
        ~buffer();

        auto swap(buffer& rhs) -> void;
        auto operator=(buffer rhs) -> buffer&;

        auto empty() const noexcept -> bool;

        auto data() noexcept -> void*;
        auto data() const noexcept -> void const*;

        template<typename T>
        auto get() const -> T {
            return T(data());
        }

        auto size() const noexcept -> std::size_t;

        auto to_tuple() -> std::tuple<void*, std::size_t> {
            return std::make_tuple(data(), size());
        }

        auto to_tuple() const -> std::tuple<const void*, std::size_t> {
            return std::make_tuple(data(), size());
        }

        auto to_vector() const -> std::vector<byte_t> {
            return {get<const byte_t*>(), get<const byte_t*>() + size()};
        }

        friend auto operator==(const buffer& b1, const buffer& b2) -> bool;
        friend auto operator!=(const buffer& b1, const buffer& b2) -> bool;

    private:
        class buffer_;
        buffer_* p_;
    };
} // namespace ipc
