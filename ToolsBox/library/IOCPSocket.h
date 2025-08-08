#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <WinSock2.h>
#include <mswsock.h>
#include <fstream>
#include <memory>
#include <filesystem>
#include <parallel_hashmap/phmap.h>
#include <minmalloc/mimalloc.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <mutex>
#include <tbb/task_arena.h>
#include <tbb/task_group.h>

#include "Logger.h"

using namespace std::chrono_literals;

#pragma comment(lib, "Mswsock.lib")

enum IOCP_TYPE {
    RECV,
    SEND,
    ACCEPT,
    NONE,
};

class IOContext {
public:
    OVERLAPPED overlapped{};
    SOCKET client;
    IOCP_TYPE type;
    char buff[2048];

    IOContext() : client(INVALID_SOCKET), type(NONE), buff{} {}

    ~IOContext() {
        shutdown(client, SD_BOTH);
        closesocket(client);
    }

    auto rest() -> void {
        if (HasOverlappedIoCompleted(&overlapped)) {
            type = NONE;
            closesocket(client);
            std::memset(&overlapped, 0, sizeof(OVERLAPPED));
        }
    }
};

class IOCPSocket {
    template <typename T>
    class MinmallocAllocator {
    public:
        using value_type = T;

        MinmallocAllocator() noexcept = default;

        template <typename U>
        explicit MinmallocAllocator(const MinmallocAllocator<U>&) noexcept {}

        auto allocate(const std::size_t n) -> T* {
            if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
                throw std::bad_alloc();
            }
            T* p = static_cast<T*>(mi_malloc(n * sizeof(T)));
            if (!p) {
                throw std::bad_alloc();
            }
            return p;
        }

        static auto deallocate(T* const p, const size_t size) noexcept -> void {
            mi_free(p);
        }
    };

    template <typename T, typename... Args>
    static auto mimalloc_shared(Args&&... args) -> std::shared_ptr<T> {
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

    template <typename K, typename V>
    using SafeMap = phmap::parallel_flat_hash_map<K, V, phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, MinmallocAllocator<std::pair<K, V>>, 4, std::mutex>;

public:
    inline static HANDLE iocp = nullptr;
    inline static SOCKET listen_sock = INVALID_SOCKET;
    inline static tbb::task_arena ta;
    inline static tbb::task_group tg;
    inline static std::atomic_bool stop = false;
    inline static SafeMap<SOCKET, std::shared_ptr<IOContext>> io_contexts;

    inline static LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
    inline static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = nullptr;

    static auto build(const socket_t sock_, const int threads = std::thread::hardware_concurrency()) -> void {
        listen_sock = sock_;
        ta.initialize(threads);

        iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
        if (!iocp) {
            LOG_ERROR << "创建IOCP端口失败!";
            return;
        }
        if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_sock), iocp, 0, 0)) {
            LOG_ERROR << "绑定IOCP端口失败!";
            return;
        }
        unsigned long ul = 1;
        ioctlsocket(sock_, FIONBIO, &ul);
    }

    static auto post_accepts() -> void {
        for (int i = 0; i < ta.max_concurrency() * 3; ++i) {
            if (!post_accept()) {
                LOG_ERROR << "投放AcceptEx失败!";
                return;
            }
        }
        tg.wait();
    }

    static auto task_works(const std::function<void(SOCKET, std::string&&, int)>& call_back) -> void {
        for (int i = 0; i < ta.max_concurrency(); ++i) {
            ta.execute([call_back] {
                tg.run([call_back] {
                    while (!stop.load()) {
                        DWORD bytes = 0;
                        ULONG_PTR key = 0;
                        LPOVERLAPPED overlapped = nullptr;

                        const int ret = GetQueuedCompletionStatus(iocp, &bytes, &key, &overlapped, INFINITE);
                        const auto io_context = CONTAINING_RECORD(overlapped, IOContext, overlapped);

                        if (io_context->client == INVALID_SOCKET) {
                            continue;
                        }

                        if (!ret || overlapped == nullptr) {
                            const auto error = GetLastError();
                            if (error == WAIT_TIMEOUT || error == ERROR_NETNAME_DELETED || error == ERROR_OPERATION_ABORTED) {
                                io_contexts.erase(io_context->client);
                                while (!post_accept()) {
                                    std::this_thread::sleep_for(1ms);
                                }
                            }
                            continue;
                        }

                        if (io_context->type == ACCEPT) {
                            sockaddr *local_addr = nullptr, *remote_addr = nullptr;
                            int local_len = 0, remote_len = 0;

                            GetAcceptExSockaddrs(io_context->buff,
                                                 0,
                                                 sizeof(SOCKADDR_STORAGE) + 16,
                                                 sizeof(SOCKADDR_STORAGE) + 16,
                                                 &local_addr,
                                                 &local_len,
                                                 &remote_addr,
                                                 &remote_len);

                            std::string ip_str(INET6_ADDRSTRLEN + 1, '\0');
                            unsigned short port = 0;
                            if (remote_addr->sa_family == AF_INET) {
                                const sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(remote_addr);
                                inet_ntop(AF_INET, &addr_in->sin_addr, ip_str.data(), sizeof(ip_str));
                                port = ntohs(addr_in->sin_port);
                            } else if (remote_addr->sa_family == AF_INET6) {
                                const sockaddr_in6* addr_in6 = reinterpret_cast<sockaddr_in6*>(remote_addr);
                                inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str.data(), sizeof(ip_str));
                                port = ntohs(addr_in6->sin6_port);
                            }

                            call_back(io_context->client, std::move(ip_str), port);

                            while (!post_accept()) {
                                std::this_thread::sleep_for(1ms);
                            }
                            continue;
                        }

                        if (io_context->type == RECV) {
                            continue;
                        }

                        if (io_context->type == SEND) {}
                    }
                });
            });
        }
    }

    static auto get_io_context(const socket_t sock) -> std::shared_ptr<IOContext> {
        if (io_contexts.contains(sock)) {
            return io_contexts[sock];
        }
        return nullptr;
    }

    static auto close_client(const socket_t sock) -> bool {
        return io_contexts.erase(sock);
    }

    static auto is_build(const socket_t sock) -> bool {
        return listen_sock == sock;
    }

    static auto stop_socket() -> void {
        stop = true;
    }
private:
    static auto post_accept() -> bool {
        if (stop) {
            return true;
        }

        const SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
        if (sock == INVALID_SOCKET) {
            LOG_ERROR << "创建套接字失败!";
            return false;
        }

        std::shared_ptr<IOContext> io_context = mimalloc_shared<IOContext>();
        io_context->type = ACCEPT;
        io_context->client = sock;

        DWORD bytes;
        if (!AcceptEx(listen_sock, io_context->client, io_context->buff, 0, sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &bytes, &io_context->overlapped)) {
            const int err = WSAGetLastError();
            if (err != ERROR_IO_PENDING) {
                LOG_ERROR << "接收失败!";
                return false;
            }
        }

        if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(io_context->client), iocp, 0, 0)) {
            LOG_ERROR << "绑定IOCP端口失败!";
            return false;
        }

        io_contexts[io_context->client] = std::move(io_context);
        return true;
    }
};
