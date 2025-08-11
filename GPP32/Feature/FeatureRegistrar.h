#pragma once
#include <pch.h>

#include "library/thread_pool.hpp"

class FeatureBase {
public:
    static auto get_count() -> size_t {
        return fetures.size();
    }

    static auto initialize() -> void {
        std::call_once(once_flag,
            [] {
                for (const std::function<void()>& function : creates) {
                    function();
                }
            });
    }

    static auto instance() -> FeatureBase& {
        static FeatureBase that;
        return that;
    }

    virtual auto render() -> void {
        render_tg_.update_start();
        for (const auto& val : fetures | std::views::values) {
            val->render();
        }
        std::unique_lock lock(rw_lock);
        render_time = render_tg_.get_duration<std::chrono::milliseconds>();
    }

    virtual auto update() -> void {
        update_tg_.update_start();
        for (const auto& val : fetures | std::views::values) {
            val->update();
        }
        std::unique_lock lock(rw_lock);
        update_time = update_tg_.get_duration<std::chrono::milliseconds>();
    }

    template<class T>
    static auto get_feature(const std::string& _name) -> std::optional<std::shared_ptr<T>> requires (std::is_base_of_v<FeatureBase, T>) {
        if (!fetures.contains(_name)) {
            return std::nullopt;
        }
        return std::static_pointer_cast<T>(fetures[_name]);
    }

    FeatureBase(const FeatureBase&) = delete;
    auto operator=(const FeatureBase&) -> FeatureBase & = delete;
    FeatureBase(FeatureBase&&) = delete;
    auto operator=(FeatureBase&&) -> FeatureBase & = delete;

    inline static std::shared_mutex rw_lock;
    inline static float update_time;
    inline static float render_time;
protected:
    ~FeatureBase() = default;
    FeatureBase() = default;

    static auto add_feature(const std::string& _name, const std::shared_ptr<FeatureBase>& _that) -> void {
        fetures[_name] = _that;
        LOG_DEBUG << "已添加功能->" << _name;
    }

    inline static std::vector<std::function<void()>> creates;

private:
    tp::TimeGuard update_tg_;
    tp::TimeGuard render_tg_;
    inline static std::once_flag once_flag;
    inline static util::Map<std::string, std::shared_ptr<FeatureBase>> fetures;
};

template<typename T>
class FeatureRegistrar : public FeatureBase {
public:
    static auto instance() -> std::shared_ptr<T> {
        return feature_instance;
    }

    FeatureRegistrar(const FeatureRegistrar&) = delete;
    auto operator=(const FeatureRegistrar&) -> FeatureRegistrar & = delete;
    FeatureRegistrar(FeatureRegistrar&&) = delete;
    auto operator=(FeatureRegistrar&&) -> FeatureRegistrar & = delete;

protected:
    FeatureRegistrar() = default;
    ~FeatureRegistrar() = default;

private:
    class Registrator {
    public:
        Registrator() {
            creates.emplace_back(create);
        }
    };

    inline static Registrator registrator_;
    inline static std::shared_ptr<T> feature_instance;

    virtual auto touch() -> void* {
        return &registrator_;
    }

    static auto create() -> void {
        if (!feature_instance) {
            feature_instance = std::static_pointer_cast<T>(std::make_shared<T>());
            add_feature(std::string(typeid(T).name()).substr(6), feature_instance);
        }
    }

    friend T;
};
