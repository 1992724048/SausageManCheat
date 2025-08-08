#pragma once
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <tbb/task_arena.h>
#include <tbb/task_group.h>

namespace tp {
    /**
     * @brief 支持任务窃取, 自动分配线程资源和负载
    */
    class ThreadPool {
    public:
        /**
         * @brief 构造函数，初始化线程池
         *
         * @param num_threads_ 线程池中线程的数量，默认为硬件支持的并发数
         */
        explicit ThreadPool(const int num_threads_ = std::thread::hardware_concurrency()) : f_arena_(num_threads_), f_task_count_(0) {}

        /**
         * @brief 获取线程池中线程的最大数量
         *
         * @return int 线程池中线程的最大数量
         */
        auto max_threads() const -> int {
            return f_arena_.max_concurrency();
        }

        /**
         * @brief 提交一个任务到线程池
         *
         * @tparam F 任务函数类型
         * @tparam Args 任务函数参数类型
         * @param func_ 任务函数
         * @param args_ 任务函数的参数
         * @return std::future<std::invoke_result_t<F, Args...>> 任务的返回值的future对象
         */
        template<typename F, typename... Args>
        auto push(F&& func_, Args&&... args_) -> std::future<std::invoke_result_t<F, Args...>> {
            using ResultType = std::invoke_result_t<F, Args...>;

            auto task_ptr_ = std::make_shared<std::packaged_task<ResultType()>>(
                    [func = std::forward<F>(func_), args_tuple = std::make_tuple(std::forward<Args>(args_)...)]()mutable {
                        return std::apply([&func]<typename... T>(T&&... args_) -> ResultType {
                                              return std::invoke(func, std::forward<T>(args_)...);
                                          },
                                          std::move(args_tuple));
                    });

            std::future<ResultType> res_ = task_ptr_->get_future();

            f_task_count_.fetch_add(1, std::memory_order_relaxed);

            f_arena_.enqueue([this, task_ptr_]() {
                f_group_.run([this, task_ptr_] {
                    (*task_ptr_)();
                    f_task_count_.fetch_sub(1, std::memory_order_relaxed);
                    {
                        std::lock_guard lock_(f_mtx_);
                        f_cv_.notify_all();
                    }
                });
            });

            return res_;
        }

        /**
         * @brief 等待所有任务完成
         */
        auto wait() -> void {
            std::unique_lock lock_(f_mtx_);
            f_cv_.wait(lock_,
                       [this] {
                           return f_task_count_.load(std::memory_order_relaxed) == 0;
                       });
        }

        /**
         * @brief 等待所有任务完成，或者等待指定时间后返回
         *
         * @tparam Rep 持续时间的表示类型
         * @tparam Period 持续时间的周期类型
         * @param duration_ 等待的时间
         * @return bool 如果所有任务完成则返回true，否则返回false
         */
        template<typename Rep, typename Period>
        auto wait_for(const std::chrono::duration<Rep, Period>& duration_) -> bool {
            std::unique_lock lock_(f_mtx_);
            return f_cv_.wait_for(lock_,
                                  duration_,
                                  [this] {
                                      return f_task_count_.load(std::memory_order_relaxed) == 0;
                                  });
        }

        /**
         * @brief 提交一个循环任务到线程池
         *
         * @tparam F 任务函数类型
         * @param count_ 循环次数
         * @param func_ 任务函数
         * @return std::vector<std::future<std::invoke_result_t<F, std::size_t>>> 所有任务的返回值的future对象的向量
         */
        template<typename F>
        auto push_loop(std::size_t count_, F&& func_) -> std::vector<std::future<std::invoke_result_t<F, std::size_t>>> {
            using ResultType = std::invoke_result_t<F, std::size_t>;
            std::vector<std::future<ResultType>> futures_;
            futures_.reserve(count_);
            for (std::size_t i_ = 0; i_ < count_; ++i_) {
                futures_.push_back(push(std::forward<F>(func_), i_));
            }
            return futures_;
        }

        /**
         * @brief 提交一个迭代器范围内的任务到线程池
         *
         * @tparam Iterator 迭代器类型
         * @tparam F 任务函数类型
         * @param begin_ 迭代器范围的起始位置
         * @param end_ 迭代器范围的结束位置
         * @param func_ 任务函数
         * @return std::vector<std::future<std::invoke_result_t<F, typename std::iterator_traits<Iterator>::value_type>>> 所有任务的返回值的future对象的向量
         */
        template<typename Iterator, typename F>
        auto push_loop(Iterator begin_,
                       Iterator end_,
                       F&& func_) -> std::vector<std::future<std::invoke_result_t<F, typename std::iterator_traits<Iterator>::value_type>>> {
            using value_type = typename std::iterator_traits<Iterator>::value_type;
            using ResultType = std::invoke_result_t<F, value_type>;
            std::vector<std::future<ResultType>> futures_;
            for (auto it_ = begin_; it_ != end_; ++it_) {
                futures_.push_back(push(std::forward<F>(func_), *it_));
            }
            return futures_;
        }

    private:
        tbb::task_arena f_arena_;
        tbb::task_group f_group_;
        std::atomic<size_t> f_task_count_;
        std::mutex f_mtx_;
        std::condition_variable f_cv_;
    };

    /**
     * @brief 时间守护结构体，用于记录和计算时间持续
     */
    struct TimeGuard {
        /**
         * @brief 使用std::chrono::steady_clock作为时钟
         */
        using Clock = std::chrono::steady_clock;
        /**
         * @brief 时间点类型
         */
        using TimePoint = Clock::time_point;

        /**
         * @brief 构造函数，初始化时间守护并记录起始时间
         */
        TimeGuard() : f_start_(Clock::now()) {}

        /**
         * @brief 更新起始时间点为当前时间
         */
        auto update_start() -> void {
            f_start_ = Clock::now();
        }

        /**
         * @brief 获取从起始时间点到当前时间点的持续时间，以秒为单位
         *
         * @return double 持续时间，单位为秒
         */
        [[nodiscard]] auto get_duration() const -> double {
            return std::chrono::duration<double>(Clock::now() - f_start_).count();
        }

    private:
        /**
         * @brief 起始时间点
         */
        TimePoint f_start_;
    };
}
