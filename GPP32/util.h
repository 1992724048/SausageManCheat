#pragma once
#include "pch.h"

namespace util {
    template<typename T, typename... Args>
    static auto make_mimalloc_shared(Args&&... args) -> std::shared_ptr<T> {
        void* mem = mi_malloc(sizeof(T));
        try {
            new(mem) T(std::forward<Args>(args)...);
            return std::shared_ptr<T>(static_cast<T*>(mem),
                                      [](T* ptr) {
                                          ptr->~T();
                                          mi_free(ptr);
                                      });
        } catch (...) {
            mi_free(mem);
            throw;
        }
    }

    template<typename T, typename... Args>
    static auto make_mimalloc_unique(Args&&... args_) -> std::unique_ptr<T, void(*)(T*)> {
        void* mem_ = mi_malloc(sizeof(T));
        if (!mem_) {
            throw std::bad_alloc();
        }

        try {
            new(mem_) T(std::forward<Args>(args_)...);
        } catch (...) {
            mi_free(mem_);
            throw;
        }

        auto deleter_ = [](T* ptr_) {
            ptr_->~T();
            mi_free(ptr_);
        };

        return std::unique_ptr<T, void(*)(T*)>(static_cast<T*>(mem_), deleter_);
    }

    template<typename K, typename V>
    using Map = phmap::parallel_flat_hash_map<K, V, phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, mi_stl_allocator<std::pair<K, V>>, 4, std::shared_mutex>;
    using String = std::basic_string<char, std::char_traits<char>, mi_stl_allocator<char>>;

    inline auto is_bad_ptr(void* _address, const int _size = 1) -> bool {
        if (reinterpret_cast<std::uintptr_t>(_address) < 0xF000) {
            return true;
        }

        try {
            return *static_cast<char*>(_address) = *static_cast<char*>(_address);
        } catch (...) {
            return true;
        }
    }

    template<typename T>
    auto read_memory(void* _address, std::vector<int> _offset = {}) -> std::optional<T> {
        try {
            std::uintptr_t temp = reinterpret_cast<std::uintptr_t>(_address);

            if (!_offset.empty()) {
                int end_offset = _offset[_offset.size() - 1];
                _offset.pop_back();

                for (const int& v : _offset) {
                    temp = *reinterpret_cast<std::uintptr_t*>(temp + v);
                }

                return *reinterpret_cast<T*>(temp + end_offset);
            }

            return *reinterpret_cast<T*>(temp);
        } catch (...) {
            return std::nullopt;
        }
    }
}
