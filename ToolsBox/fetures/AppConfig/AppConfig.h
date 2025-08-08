#pragma once
#include <fetures/FeatureRegistrar.hpp>

#include "SYCL/SYCL.hpp"

class AppConfig final : public FeatureRegistrar<AppConfig> {
public:
    AppConfig();
    ~AppConfig() = default;

    config::Field<bool> f_auto_chek_update;
    config::Field<bool> f_check_file;
    config::Field<config::Enum<XPU>> f_calc_xpu;
    config::Field<std::string> f_calc_process;
    config::Field<bool> f_multi_thread;
    config::Field<bool> f_no_sycl;

    METHODS_BEGIN
        METHOD_ADD(config_get)
        METHOD_ADD(config_set)
        METHOD_ADD(invoke)
    METHODS_END

    auto switch_device() const -> bool;

    static auto config_get(METHOD_ARGS) -> void ;
    static auto config_set(METHOD_ARGS) -> void ;
    static auto invoke(METHOD_ARGS) -> void ;

};
