#pragma once
#include <pch.h>
#include "../../SYCL/SYCL.h"

enum XPU : char {
    CPU,
    GPU,
    FPGA,
    CUSTOM,
    AUTOMATIC,
    HOST,
    ALL
};

class SYCL {
public:
    static auto instance() -> SYCL& {
        static SYCL that;
        return that;
    }

    auto get_cl() const -> CL& {
        return cl;
    }

    [[nodiscard]] auto get_xpus() const -> std::map<std::string, std::string> {
        return xpus;
    }

    auto switch_device(const XPU _device_type, const std::string& _device_name) const -> bool {
        return cl.switch_device(magic_enum::enum_name<XPU>(_device_type).data(), _device_name);
    }
private:
    SYCL() : cl{CL::instance()} {
        xpus = cl.cl_get_xpu();
    }

    CL& cl;
    std::map<std::string, std::string> xpus;
};
