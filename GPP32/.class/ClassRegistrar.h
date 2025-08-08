#pragma once

#pragma once
#include <pch.h>

class ClassBase {
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

    virtual auto init() -> void {};

    template<class T>
    static auto get_feature(const std::string& _name) -> std::optional<std::shared_ptr<T>> requires (std::is_base_of_v<ClassBase, T>) {
        if (!fetures.contains(_name)) {
            return std::nullopt;
        }
        return std::static_pointer_cast<T>(fetures[_name]);
    }

    ClassBase(const ClassBase&) = delete;
    auto operator=(const ClassBase&) -> ClassBase & = delete;
    ClassBase(ClassBase&&) = delete;
    auto operator=(ClassBase&&) -> ClassBase & = delete;

protected:
    ~ClassBase() = default;
    ClassBase() = default;

    static auto add_feature(const std::string& _name, const std::shared_ptr<ClassBase>& _that) -> void {
        fetures[_name] = _that;
        LOG_DEBUG << "已添加类型->" << _name;
    }

    inline static std::vector<std::function<void()>> creates;

private:
    inline static std::once_flag once_flag;
    inline static util::Map<std::string, std::shared_ptr<ClassBase>> fetures;
};

template<typename T>
class ClassRegistrar : public ClassBase {
public:
    static auto instance() -> std::shared_ptr<T> {
        return feature_instance;
    }

    ClassRegistrar(const ClassRegistrar&) = delete;
    auto operator=(const ClassRegistrar&) -> ClassRegistrar & = delete;
    ClassRegistrar(ClassRegistrar&&) = delete;
    auto operator=(ClassRegistrar&&) -> ClassRegistrar & = delete;

protected:
    ClassRegistrar() = default;
    ~ClassRegistrar() = default;

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
            feature_instance->init();
        }
    }

    friend T;
};
