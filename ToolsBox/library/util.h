#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <coroutine>
#include <minmalloc/mimalloc.h>
#include <memory>


namespace util {
    inline auto app_path() -> std::filesystem::path {
        static std::filesystem::path app_path;
        if (app_path.empty()) {
            char path_out[MAX_PATH] = {};
            GetModuleFileNameA(GetModuleHandleA(nullptr), path_out, MAX_PATH);
            app_path = std::filesystem::path(path_out).parent_path();
        }
        return app_path;
    }

    inline auto read_file(const std::filesystem::path& path) -> std::pair<std::shared_ptr<std::uint8_t[]>, std::size_t> {
        auto close_handle = [](const HANDLE h) {
            if (h && h != INVALID_HANDLE_VALUE) {
                CloseHandle(h);
            }
        };

        const std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(close_handle)> file{
            CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ, nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, nullptr),
            close_handle
        };
        if (!file || file.get() == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to open file: " + path.string());
        }

        LARGE_INTEGER sz{};
        if (!GetFileSizeEx(file.get(), &sz) || sz.QuadPart <= 0) {
            return {nullptr, 0};
        }
        const std::size_t size = static_cast<std::size_t>(sz.QuadPart);
        const std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(close_handle)> mapping{CreateFileMappingW(file.get(), nullptr, PAGE_READONLY, 0, 0, nullptr), close_handle};
        if (!mapping) {
            throw std::runtime_error("CreateFileMapping failed: " + path.string());
        }

        void* base = MapViewOfFile(mapping.get(), FILE_MAP_READ, 0, 0, 0);
        if (!base) {
            throw std::runtime_error("MapViewOfFile failed: " + path.string());
        }

        auto deleter = [](const std::uint8_t* p) {
            if (p) {
                UnmapViewOfFile(p);
            }
        };
        std::shared_ptr<std::uint8_t[]> ptr(static_cast<std::uint8_t*>(base), deleter);
        return {std::move(ptr), size};
    }

    inline auto read_resource(const int resource_id, const LPCTSTR resource_type) -> std::pair<std::shared_ptr<std::uint8_t[]>, std::size_t> {
        const HMODULE instance = ::GetModuleHandle(nullptr);
        if (!instance) {
            return {nullptr, 0};
        }

        const HRSRC res = ::FindResource(instance,MAKEINTRESOURCE(resource_id), resource_type);
        if (!res) {
            return {nullptr, 0};
        }

        const DWORD size = SizeofResource(instance, res);
        if (size == 0) {
            return {nullptr, 0};
        }

        const HGLOBAL global = LoadResource(instance, res);
        if (!global) {
            return {nullptr, 0};
        }

        void* data = LockResource(global);
        if (!data) {
            return {nullptr, 0};
        }

        std::shared_ptr<std::uint8_t[]> ptr(static_cast<std::uint8_t*>(data), [](std::uint8_t*) {});
        return {std::move(ptr), size};
    }

    template<typename Ret>
    struct CoroWrapper {
        struct promise_type {
            Ret value_;
            std::exception_ptr exception_;

            auto get_return_object() {
                return CoroWrapper(std::coroutine_handle<promise_type>::from_promise(*this));
            }

            static auto initial_suspend() noexcept -> std::suspend_always {
                return {};
            }

            static auto final_suspend() noexcept -> std::suspend_always {
                return {};
            }

            auto return_value(Ret&& v) -> void {
                value_ = std::forward<decltype(v)>(v);
            }

            auto yield_value(Ret&& v) -> std::suspend_always {
                value_ = std::forward<decltype(v)>(v);
                return {};
            }

            static auto unhandled_exception() -> void {
                std::terminate();
            }

            auto store_exception() -> void {
                exception_ = std::current_exception();
            }
        };

        explicit CoroWrapper(std::coroutine_handle<promise_type> h) : handle(h) {}

        ~CoroWrapper() {
            if (handle) {
                handle.destroy();
            }
        }

        auto operator()() -> void {
            handle.resume();
        }

        auto value() -> Ret {
            return handle.promise().value_;
        }

        [[nodiscard]] auto done() const -> bool {
            return !handle || handle.done();
        }

    private:
        std::coroutine_handle<promise_type> handle;
    };

    template<>
    struct CoroWrapper<void> {
        struct promise_type {
            std::exception_ptr exception_;

            auto get_return_object() {
                return CoroWrapper(std::coroutine_handle<promise_type>::from_promise(*this));
            }

            static auto initial_suspend() noexcept -> std::suspend_always {
                return {};
            }

            static auto final_suspend() noexcept -> std::suspend_always {
                return {};
            }

            static auto unhandled_exception() -> void {
                std::terminate();
            }

            auto store_exception() -> void {
                exception_ = std::current_exception();
            }
        };

        explicit CoroWrapper(const std::coroutine_handle<promise_type> h) : handle(h) {}

        ~CoroWrapper() {
            if (handle) {
                handle.destroy();
            }
        }

        auto operator()() const -> void {
            handle.resume();
        }

        [[nodiscard]] auto done() const -> bool {
            return !handle || handle.done();
        }

    private:
        std::coroutine_handle<promise_type> handle;
    };

    class Timer {
        std::jthread thread_;

    public:
        Timer(const Timer&) = delete;
        auto operator=(const Timer&) -> Timer& = delete;
        Timer() = default;

        auto start(int ms, const std::function<void()>& task) -> void {
            if (thread_.joinable()) {
                thread_.request_stop();
                thread_.join();
            }

            thread_ = std::jthread([this, ms, task](const std::stop_token& st) {
                while (!st.stop_requested()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                    if (!st.stop_requested()) {
                        task();
                    }
                }
            });
        }

        auto stop() -> void {
            if (thread_.joinable()) {
                thread_.request_stop();
                thread_.join();
            }
        }
    };

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

        return std::unique_ptr<T, void(*)(T*)>(
                static_cast<T*>(mem_),
                deleter_
                );
    }

    template<typename K, typename V>
    using Map = phmap::parallel_flat_hash_map<K, V, phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, mi_stl_allocator<std::pair<K, V>>, 4, std::shared_mutex>;
}
