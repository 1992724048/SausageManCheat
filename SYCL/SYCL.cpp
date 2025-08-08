#include "SYCL.h"
#include <sycl/sycl.hpp>
#include "omp.h"
#include <oneapi/dpl/execution>
#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/experimental/kt/esimd_radix_sort.h>

std::shared_ptr<sycl::queue> queue;

auto CL::cl_get_xpu() -> std::map<std::string, std::string> {
    std::map<std::string, std::string> xpus;

    try {
        const std::vector<sycl::device> devices = sycl::device::get_devices();

        for (const auto& dev : devices) {
            auto name = dev.get_info<sycl::info::device::name>();
            const auto type = dev.get_info<sycl::info::device::device_type>();

            if (xpus.contains(name)) {
                continue;
            }

            switch (type) {
                case sycl::info::device_type::cpu:
                    xpus[name] = "CPU";
                    break;
                case sycl::info::device_type::gpu:
                    xpus[name] = "GPU";
                    break;
                case sycl::info::device_type::accelerator:
                    xpus[name] = "FPGA";
                    break;
                case sycl::info::device_type::custom:
                    xpus[name] = "CUSTOM";
                    break;
                case sycl::info::device_type::host:
                    xpus[name] = "HOST";
                    break;
                case sycl::info::device_type::automatic:
                    xpus[name] = "AUTOMATIC";
                    break;
                case sycl::info::device_type::all:
                    xpus[name] = "ALL";
                    break;
                default:
                    xpus[name] = "UNKNOWN";
                    break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "SYCL device enumeration failed: " << e.what() << "\n";
    }

    return xpus;
}

auto CL::switch_device(const std::string& _device_type, const std::string& _device_name) -> bool {
    std::vector<sycl::device> devices = sycl::device::get_devices();
    for (const auto& dev : devices) {
        auto name = dev.get_info<sycl::info::device::name>();
        const auto type = dev.get_info<sycl::info::device::device_type>();
        std::string type_str;
        switch (type) {
            case sycl::info::device_type::cpu:
                type_str = "CPU";
                break;
            case sycl::info::device_type::gpu:
                type_str = "GPU";
                break;
            case sycl::info::device_type::accelerator:
                type_str = "FPGA";
                break;
            case sycl::info::device_type::custom:
                type_str = "CUSTOM";
                break;
            case sycl::info::device_type::automatic:
                type_str = "AUTOMATIC";
                break;
            case sycl::info::device_type::host:
                type_str = "HOST";
                break;
            case sycl::info::device_type::all:
                type_str = "ALL";
                break;
            default:
                type_str = "UNKNOWN";
                break;
        }
        if (type_str == _device_type && name == _device_name) {
            queue = std::make_shared<sycl::queue>(dev);
            return true;
        }
    }
    return false;
}

auto CL::w2c(const std::vector<std::pair<int, glm::vec3>>& _pos, std::vector<std::pair<int, glm::vec3>>& _result, const glm::mat4x4& _camera_matrix, const glm::vec2& _screen_size) -> void {
    if (_pos.empty()) {
        return;
    }

    const glm::mat4x4 matrix = _camera_matrix;
    const glm::vec2 size = _screen_size;
    const size_t pos_size = _pos.size();

    sycl::buffer in_buf(_pos.data(), sycl::range<1>(_pos.size()));
    sycl::buffer out_buf(_result.data(), sycl::range<1>(_result.size()));

    queue->submit([&](sycl::handler& _h) {
        const auto in = in_buf.get_access<sycl::access::mode::read>(_h);
        const auto out = out_buf.get_access<sycl::access::mode::write>(_h);

        _h.parallel_for(sycl::range<1>(pos_size),
                        [=](sycl::id<1> _idx) {
                            const int i = _idx[0];
                            auto& [id, pos] = in[i];

                            glm::vec4 clip_space_pos = _camera_matrix * glm::vec4(pos, 1.0f);

                            if (clip_space_pos.w == 0.0f) {
                                return;
                            }

                            glm::vec3 screen;
                            screen.z = clip_space_pos.w;

                            clip_space_pos /= clip_space_pos.w;

                            screen.x = (clip_space_pos.x + 1.0f) * 0.5f * _screen_size.x;
                            screen.y = (clip_space_pos.y + 1.0f) * 0.5f * _screen_size.y;
                            screen.y = _screen_size.y - screen.y;

                            out[i] = {id, screen};
                        });
    });
    queue->wait();
}
