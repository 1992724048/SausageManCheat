#pragma once
#include <pch.h>
#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>
#include <typeindex>

#define IPC_MSG_BEGIN auto register_ipc_messages() -> void override {
#define IPC_MSG_ADD(func) register_ipc_handler(#func, [this](ipc::channel& _ch) { func(_ch); });
#define IPC_MSG_ARGS ipc::channel& _ch
#define IPC_MSG_END }

class IPC {
public:
    virtual ~IPC() {
        stop_listening();
    }

    auto start_listening(const std::string& _channel_name) -> void {
        std::call_once(start_once_flag,
                       [&] {
                           this->channel_name = _channel_name;
                           channel = std::make_unique<ipc::channel>();
                           channel->connect(_channel_name.data(), ipc::sender | ipc::receiver);

                           listening.store(true);
                           listening_thread = std::thread([this] {
                               while (listening.load()) {
                                   auto event = channel->recv();
                                   if (!listening.load()) {
                                       break;
                                   }

                                   if (const auto raw = event.get<const char*>()) {
                                       dispatch_message(std::string(raw));
                                   }
                               }
                           });
                       });
    }

    auto stop_listening() -> void {
        listening.store(false);
        if (listening_thread.joinable()) {
            if (channel) {
                channel->send("stop");
            }
            listening_thread.join();
        }
    }

    static auto registrar_all() -> void {
        for (auto& registration_callback : registration_callbacks) {
            registration_callback();
        }
    }

    static auto start_all_ipc() -> void {
        for (auto& [type, inst] : ipc_instances) {
            if (inst) {
                inst->start_listening("ipc_channel_" + std::string(type.name()).substr(6));
            }
        }
    }

    static auto stop_all_ipc() -> void {
        for (auto& inst : ipc_instances | std::views::values) {
            if (inst) {
                inst->stop_listening();
            }
        }
    }

    template<typename T>
    static auto get() -> std::shared_ptr<T> {
        if (ipc_instances.contains(std::type_index(typeid(T)))) {
            return std::static_pointer_cast<T>(ipc_instances[std::type_index(typeid(T))]);
        }
        return nullptr;
    }

protected:
    std::unique_ptr<ipc::channel> channel;
    std::string channel_name;
    std::once_flag start_once_flag;
    std::thread listening_thread;
    std::atomic_bool listening = false;

    util::Map<std::string, std::function<void(ipc::channel&)>> message_handlers;

    static inline util::Map<std::type_index, std::shared_ptr<IPC>> ipc_instances;
    inline static std::vector<std::function<void()>> registration_callbacks;

    auto register_ipc_handler(const std::string& _name, const std::function<void(ipc::channel&)>& _handler) -> void {
        message_handlers[_name] = _handler;
    }

    auto dispatch_message(const std::string& _msg_name) -> void {
        if (message_handlers.contains(_msg_name)) {
            message_handlers[_msg_name](*channel);
        }
    }

    virtual auto register_ipc_messages() -> void = 0;

    template<typename T>
    static auto register_instance(const std::shared_ptr<T>& _instance) -> void {
        ipc_instances[std::type_index(typeid(T))] = _instance;
        LOG_DEBUG << "注册IPC: " << std::string(std::type_index(typeid(T)).name()).substr(6);
    }

    template<typename T>
    friend class IPCRegistrar;
};

template<class T>
class IPCRegistrar : public IPC {
public:
    static auto instance() -> std::shared_ptr<T> {
        return ipc_instance;
    }

    IPCRegistrar(const IPCRegistrar&) = delete;
    auto operator=(const IPCRegistrar&) -> IPCRegistrar& = delete;
    IPCRegistrar(IPCRegistrar&&) = delete;
    auto operator=(IPCRegistrar&&) -> IPCRegistrar& = delete;

protected:
    IPCRegistrar() = default;
    ~IPCRegistrar() override = default;

private:
    class Registrar {
    public:
        Registrar() {
            registration_callbacks.emplace_back(create_instance);
        }
    };

    inline static Registrar auto_registrar;
    inline static std::shared_ptr<T> ipc_instance;

    virtual auto touch() -> void* {
        return &auto_registrar;
    }

    static auto create_instance() -> void {
        if (!ipc_instance) {
            ipc_instance = std::static_pointer_cast<T>(std::make_shared<T>());
            register_instance<T>(ipc_instance);
            ipc_instance->register_ipc_messages();
        }
    }

    friend T;
};
