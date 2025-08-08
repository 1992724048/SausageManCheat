// FlutterPluginRegistry.h
#ifndef FLUTTER_PLUGIN_REGISTRY_H_
#define FLUTTER_PLUGIN_REGISTRY_H_

#include <string>
#include <vector>
#include <functional>

#include "flutter/ephemeral/plugin_registrar_windows.h"

class PluginRegistry {
public:
    using RegistrarCallback = std::function<void(FlutterDesktopPluginRegistrarRef)>;

    static auto GetInstance() -> PluginRegistry& {
        static PluginRegistry instance;
        return instance;
    }

    static auto Register(const std::string& _name, RegistrarCallback _callback) -> void {
        plugins_.emplace_back(_name, std::move(_callback));
    }

    static auto GetPlugins() -> const std::vector<std::pair<std::string, RegistrarCallback>>& {
        return plugins_;
    }
protected:
    PluginRegistry() = default;
private:
    inline static std::vector<std::pair<std::string, RegistrarCallback>> plugins_;
};

template<typename T, bool AutoCreation = true>
class FlutterPlugin : public PluginRegistry {
    FlutterPlugin() = default;

public:
    static constexpr bool isAutoCreation = AutoCreation;
protected:
    ~FlutterPlugin() = default;
private:
    class MethodRegistrator {
    public:
        MethodRegistrator() {
            if (isAutoCreation) {
                Register(
                    T::PluginName,
                    [](const FlutterDesktopPluginRegistrarRef _registrar) {
                        T::RegisterWithRegistrar(
                            flutter::PluginRegistrarManager::GetInstance()->GetRegistrar<flutter::PluginRegistrarWindows>(_registrar)
                        );
                    }
                );
            }
        }
    };

    static MethodRegistrator registrator_;
    static FlutterPlugin<T> this_;

    virtual auto touch() -> void* {
        return &registrator_;
    }

    friend T;
};

template<typename T, bool AutoCreation>
typename FlutterPlugin<T, AutoCreation>::MethodRegistrator FlutterPlugin<T, AutoCreation>::registrator_;

#endif  // FLUTTER_PLUGIN_REGISTRY_H_
