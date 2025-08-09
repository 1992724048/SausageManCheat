#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <bitset>

class HardBreakPoint {
public:
    struct BreakPoint {
        int id;
        void* original;
        void* replacement;
    };

    struct DR7 {
        uint32_t raw = 0;

        auto set(const int _id, void* _address) -> void {
            raw |= (1 << (_id * 2));
            raw |= (0b00 << (16 + _id * 4));
            raw |= (0b00 << (18 + _id * 4));
        }

        auto clear(const int _id) -> void {
            raw &= ~(1 << (_id * 2));
            raw &= ~(0b11 << (16 + _id * 4));
            raw &= ~(0b11 << (18 + _id * 4));
        }
    };

    static auto initialize() -> void {
        AddVectoredExceptionHandler(1, VectoredExceptionHandler);
    }

    template<typename R, typename... Args>
    static auto set_break_point(R (*_target)(Args...), R (*_replacement)(Args...)) -> bool {
        std::unique_lock lock(mutex_);

        for (int i = 0; i < 4; ++i) {
            if (!bp_status_[i]) {
                bp_status_[i] = true;

                BreakPoint bp{i, reinterpret_cast<void*>(_target), reinterpret_cast<void*>(_replacement)};
                breakpoints_[_target] = bp;
                reverse_map_[_replacement] = _target;
                dr7_.set(i, reinterpret_cast<void*>(_target));

                apply_to_all_threads([&](const HANDLE _thread) {
                    apply_break_point_to_thread(_thread, bp);
                });

                return true;
            }
        }
        return false;
    }

    template<typename R, typename... Args>
    static auto remove_break_point(R (*_target)(Args...)) -> bool {
        std::unique_lock lock(mutex_);

        auto it = breakpoints_.find(_target);
        if (it == breakpoints_.end()) {
            return false;
        }

        const BreakPoint& bp = it->second;
        bp_status_[bp.id] = false;
        dr7_.clear(bp.id);
        reverse_map_.erase(bp.replacement);
        breakpoints_.erase(it);

        apply_to_all_threads([&](const HANDLE _thread) {
            remove_break_point_from_thread(_thread, bp);
        });

        return true;
    }

    template<typename R, typename... Args>
    static auto call_origin(R (*_replacement)(Args...), Args... _args) -> R {
        std::shared_lock lock(mutex_);

        auto orig = reverse_map_[_replacement];
        const auto& bp = breakpoints_[orig];

        HANDLE h_thread = GetCurrentThread();
        remove_break_point_from_thread(h_thread, bp);

        if constexpr (std::is_void_v<R>) {
            reinterpret_cast<R(*)(Args...)>(bp.original)(_args...);
        } else {
            R ret = reinterpret_cast<R(*)(Args...)>(bp.original)(_args...);
            apply_break_point_to_thread(h_thread, bp);
            return ret;
        }

        apply_break_point_to_thread(h_thread, bp);
        return R();
    }

    inline static DR7 dr7_;
private:
    inline static std::bitset<4> bp_status_;
    inline static std::shared_mutex mutex_;
    inline static phmap::parallel_flat_hash_map<void*, BreakPoint, phmap::priv::hash_default_hash<void*>, phmap::priv::hash_default_eq<void*>, mi_stl_allocator<std::pair<void*, BreakPoint>>, 4,
                                                std::shared_mutex> breakpoints_;
    inline static phmap::parallel_flat_hash_map<void*, void*, phmap::priv::hash_default_hash<void*>, phmap::priv::hash_default_eq<void*>, mi_stl_allocator<std::pair<void*, void*>>, 4,
                                                std::shared_mutex> reverse_map_;

    class ThreadHandle {
    public:
        explicit ThreadHandle(const DWORD _tid) {
            h_thread_ = OpenThread(THREAD_ALL_ACCESS, FALSE, _tid);
        }

        ~ThreadHandle() {
            if (h_thread_) {
                CloseHandle(h_thread_);
            }
        }

        auto valid() const -> bool {
            return h_thread_ != nullptr;
        }

        auto get() const -> HANDLE {
            return h_thread_;
        }

        auto suspend_resume(const std::function<void()>& _action) const -> void {
            if (!valid()) {
                return;
            }
            SuspendThread(h_thread_);
            _action();
            ResumeThread(h_thread_);
        }

    private:
        HANDLE h_thread_;
    };

    static auto apply_to_all_threads(const std::function<void(HANDLE)>& _func) -> void {
        const DWORD self = GetCurrentThreadId();
        for (const DWORD tid : get_all_thread_ids()) {
            if (tid == self) {
                continue;
            }
            ThreadHandle th(tid);
            if (th.valid()) {
                th.suspend_resume([&]() {
                    _func(th.get());
                });
            }
        }
    }

    static auto get_all_thread_ids() -> std::vector<DWORD> {
        std::vector<DWORD> ids;
        const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return ids;
        }

        THREADENTRY32 te{};
        te.dwSize = sizeof(te);
        if (Thread32First(snapshot, &te)) {
            do {
                if (te.th32OwnerProcessID == GetCurrentProcessId()) {
                    ids.push_back(te.th32ThreadID);
                }
            } while (Thread32Next(snapshot, &te));
        }
        CloseHandle(snapshot);
        return ids;
    }

    static auto apply_break_point_to_thread(const HANDLE _thread, const BreakPoint& _bp) -> void {
        CONTEXT ctx{};
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (!GetThreadContext(_thread, &ctx)) {
            return;
        }

        switch (_bp.id) {
            case 0:
                ctx.Dr0 = reinterpret_cast<DWORD_PTR>(_bp.original);
                break;
            case 1:
                ctx.Dr1 = reinterpret_cast<DWORD_PTR>(_bp.original);
                break;
            case 2:
                ctx.Dr2 = reinterpret_cast<DWORD_PTR>(_bp.original);
                break;
            case 3:
                ctx.Dr3 = reinterpret_cast<DWORD_PTR>(_bp.original);
                break;
            default: ;
        }
        ctx.Dr7 = dr7_.raw;
        SetThreadContext(_thread, &ctx);
    }

    static auto remove_break_point_from_thread(const HANDLE _thread, const BreakPoint& _bp) -> void {
        CONTEXT ctx{};
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (!GetThreadContext(_thread, &ctx)) {
            return;
        }

        switch (_bp.id) {
            case 0:
                ctx.Dr0 = 0;
                break;
            case 1:
                ctx.Dr1 = 0;
                break;
            case 2:
                ctx.Dr2 = 0;
                break;
            case 3:
                ctx.Dr3 = 0;
                break;
            default: ;
        }
        ctx.Dr7 = dr7_.raw;
        SetThreadContext(_thread, &ctx);
    }

    static auto WINAPI VectoredExceptionHandler(PEXCEPTION_POINTERS _ep) -> LONG {
        if (_ep->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
            std::shared_lock lock(mutex_);
            void* addr = _ep->ExceptionRecord->ExceptionAddress;
            auto bp = breakpoints_[addr];
#ifdef _WIN64
            ep->ContextRecord->Rip = reinterpret_cast<DWORD64>(bp.replacement);
#else
            _ep->ContextRecord->Eip = reinterpret_cast<DWORD>(bp.replacement);
#endif
            _ep->ContextRecord->Dr7 = dr7_.raw;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
};
