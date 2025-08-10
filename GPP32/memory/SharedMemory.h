#pragma once
#include "../pch.h"

template<class T>
class SharedMemoryMutex {
public:
    std::shared_ptr<void> mutex;
    std::shared_ptr<T> instance;
};

class SharedMemoryLock {
    std::shared_ptr<void> mutex;
    bool locked = false;

public:
    explicit SharedMemoryLock(const std::shared_ptr<void>& _mutex) : mutex(_mutex) {
        if (!mutex) {
            throw std::runtime_error("Invalid mutex handle");
        }
        switch (const DWORD ret = WaitForSingleObject(mutex.get(), INFINITE)) {
            case WAIT_OBJECT_0:
                locked = true;
                break;
            case WAIT_ABANDONED:
                locked = true;
                break;
            case WAIT_FAILED:
                throw std::runtime_error("WaitForSingleObject failed: " + std::to_string(GetLastError()));
            default:
                throw std::runtime_error("Unexpected WaitForSingleObject result: " + std::to_string(ret));
        }
    }

    ~SharedMemoryLock() {
        if (locked && mutex) {
            ReleaseMutex(mutex.get());
        }
    }

    SharedMemoryLock(const SharedMemoryLock&) = delete;
    auto operator=(const SharedMemoryLock&) -> SharedMemoryLock& = delete;
};

class SharedMemory {
public:
    [[nodiscard]] auto get_byte_szie() const -> size_t {
        return byte_szie;
    }

    static auto initialize(const bool _open) -> void {
        std::call_once(once_flag,
                       [&] {
                           open_mmap = _open;
                           for (const std::function<void()>& function : creates) {
                               function();
                           }
                       });
    }

    static auto uninitialize() -> void {
        memorys.clear();
    }

    static auto get_count() -> size_t {
        return memorys.size();
    }

    template<class T>
    static auto get_memory(const std::string& _name) -> std::optional<std::shared_ptr<T>> requires (std::is_base_of_v<SharedMemory, T>) {
        if (!memorys.contains(_name)) {
            return std::nullopt;
        }
        return std::static_pointer_cast<T>(memorys[_name].second);
    }

    SharedMemory(const SharedMemory&) = delete;
    auto operator=(const SharedMemory&) -> SharedMemory& = delete;
    SharedMemory(SharedMemory&&) = delete;
    auto operator=(SharedMemory&&) -> SharedMemory& = delete;

protected:
    SharedMemory() = default;
    ~SharedMemory() = default;

    template<class T>
    static auto create_mapping(const std::string& _name) -> std::pair<std::shared_ptr<T>, std::shared_ptr<void>> {
        const std::string memory_name = "memroy_" + _name;
        const std::string mutex_name = "mutex_" + _name;

        auto create_mapping = [&] {
            HANDLE h = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(T), memory_name.data());
            if (!h) {
                throw std::runtime_error("CreateFileMapping failed");
            }
            return h;
        };

        auto open_mapping = [&] {
            HANDLE h = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, memory_name.data());
            if (!h) {
                throw std::runtime_error("OpenFileMapping failed");
            }
            return h;
        };

        auto create_mutex = [&] {
            HANDLE h = CreateMutexA(nullptr, FALSE, mutex_name.data());
            if (!h) {
                throw std::runtime_error("CreateMutex failed");
            }
            return h;
        };

        auto open_mutex = [&] {
            HANDLE h = OpenMutexA(SYNCHRONIZE, FALSE, mutex_name.data());
            if (!h) {
                throw std::runtime_error("OpenMutex failed");
            }
            ReleaseMutex(h);
            return h;
        };

        auto map_file = std::shared_ptr<void>(open_mmap ? open_mapping() : create_mapping(),
                                              [](void* _h) {
                                                  CloseHandle(_h);
                                              });

        auto mutex = std::shared_ptr<void>(open_mmap ? open_mutex() : create_mutex(),
                                           [](void* _h) {
                                               CloseHandle(_h);
                                           });

        auto mem = memory_mapping<T>(map_file);
        memorys[_name] = {{map_file, std::static_pointer_cast<SharedMemory>(mem)}, {map_file, mutex}};
        LOG_DEBUG << "已添加内存->" << _name << "|0x" << std::uppercase << std::hex << reinterpret_cast<std::uintptr_t>(mem.get());
        return {mem, mutex};
    }

    char space[4];
    int byte_szie{0};
    inline static std::vector<std::function<void()>> creates;

private:
    inline static bool open_mmap;
    inline static std::once_flag once_flag;
    inline static util::Map<std::string, std::pair<std::pair<std::shared_ptr<void>, std::shared_ptr<SharedMemory>>, std::pair<std::shared_ptr<void>, std::shared_ptr<void>>>> memorys;

    template<class T>
    static auto memory_mapping(const std::shared_ptr<void>& _map_file) -> std::shared_ptr<T> {
        return std::shared_ptr<T>(static_cast<T*>(MapViewOfFile(_map_file.get(), FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T))),
                                  [](const T* _buff) {
                                      UnmapViewOfFile(_buff);
                                  });
    }
};

template<class T>
class SharedMemoryRegistrar : public SharedMemory {
public:
    SharedMemoryRegistrar() {
        static_assert(sizeof(T) > 0);
        byte_szie = sizeof(T);
    }

    static auto instance() -> std::shared_ptr<T> {
        return memory_instance;
    }

    static auto mutex() -> SharedMemoryLock {
        return SharedMemoryLock(memory_mutex.mutex);
    }

    SharedMemoryRegistrar(const SharedMemoryRegistrar&) = delete;
    auto operator=(const SharedMemoryRegistrar&) -> SharedMemoryRegistrar& = delete;
    SharedMemoryRegistrar(SharedMemoryRegistrar&&) = delete;
    auto operator=(SharedMemoryRegistrar&&) -> SharedMemoryRegistrar& = delete;

protected:
    ~SharedMemoryRegistrar() = default;

private:
    class Registrator {
    public:
        Registrator() {
            creates.emplace_back(create);
        }
    };

    inline static Registrator registrator_;
    inline static std::shared_ptr<T> memory_instance;
    inline static SharedMemoryMutex<T> memory_mutex;

    virtual auto touch() -> void* {
        return &registrator_;
    }

    static auto create() -> void {
        if (!memory_instance) {
            auto [ins, mutex] = create_mapping<T>(std::string(typeid(T).name()).substr(6));
            memory_instance = ins;
            memory_mutex.mutex = mutex;
            memory_mutex.instance = memory_instance;
            new(memory_instance.get()) T();
        }
    }

    friend T;
};
