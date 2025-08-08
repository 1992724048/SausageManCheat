#pragma once
#include "../pch.h"
#include "flutter/ephemeral/method_channel.h"
#include "flutter/ephemeral/plugin_registrar.h"
#include "flutter/ephemeral/standard_method_codec.h"

#define METHODS_BEGIN auto add_methods() -> void override {
#define METHOD_ADD(func) add_method(#func, func);
#define METHOD_ARGS const flutter::MethodCall<>& _method_call, std::unique_ptr<flutter::MethodResult<>>& _result
#define METHODS_END }

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

    template<class T>
    static auto get_feature(const std::string& _name) -> std::optional<std::shared_ptr<T>> requires (std::is_base_of_v<FeatureBase, T>) {
        if (!fetures.contains(_name)) {
            return std::nullopt;
        }
        return std::static_pointer_cast<T>(fetures[_name]);
    }

    static auto add_method(const std::string& _name, const std::function<void(const flutter::MethodCall<>&, std::unique_ptr<flutter::MethodResult<>>&)>& _method) -> void {
        methods[_name] = _method;
        LOG_DEBUG << "已添加方法->" << _name;
    }

    static auto get_method(const std::string& _name) -> std::optional<std::function<void(const flutter::MethodCall<>&, std::unique_ptr<flutter::MethodResult<>>&)>> {
        if (!methods.contains(_name)) {
            return std::nullopt;
        }
        return methods[_name];
    }

    FeatureBase(const FeatureBase&) = delete;
    auto operator=(const FeatureBase&) -> FeatureBase& = delete;
    FeatureBase(FeatureBase&&) = delete;
    auto operator=(FeatureBase&&) -> FeatureBase& = delete;

protected:
    ~FeatureBase() = default;
    FeatureBase() = default;

    static auto add_feature(const std::string& _name, const std::shared_ptr<FeatureBase>& _that) -> void {
        fetures[_name] = _that;
        LOG_DEBUG << "已添加功能->" << _name;
    }

    inline static std::vector<std::function<void()>> creates;

private:
    inline static std::once_flag once_flag;
    inline static util::Map<std::string, std::shared_ptr<FeatureBase>> fetures;
    inline static util::Map<std::string, std::function<void(const flutter::MethodCall<>&, std::unique_ptr<flutter::MethodResult<>>&)>> methods;
};

template<typename T>
class FeatureRegistrar : public FeatureBase {
public:
    static auto instance() -> std::shared_ptr<T> {
        return feature_instance;
    }

    FeatureRegistrar(const FeatureRegistrar&) = delete;
    auto operator=(const FeatureRegistrar&) -> FeatureRegistrar& = delete;
    FeatureRegistrar(FeatureRegistrar&&) = delete;
    auto operator=(FeatureRegistrar&&) -> FeatureRegistrar& = delete;

protected:
    FeatureRegistrar() = default;
    ~FeatureRegistrar() = default;

    virtual auto add_methods() -> void {}

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
            feature_instance->add_methods();
        }
    }

    friend T;
};
