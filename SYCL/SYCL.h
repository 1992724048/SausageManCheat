#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#ifdef DPCPP_DLL_EXPORTS
#define DPCPP_DLL_API __declspec(dllexport)
#else
#define DPCPP_DLL_API __declspec(dllimport)
#endif

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include "glm/glm.hpp"

class CL {
public:
    DPCPP_DLL_API static auto instance() -> CL& {
        static CL instance;
        return instance;
    }

    DPCPP_DLL_API auto cl_get_xpu() -> std::map<std::string, std::string>;
    DPCPP_DLL_API auto w2c(const std::vector<std::pair<int, glm::vec3>>& _pos, std::vector<std::pair<int, glm::vec3>>& _result, const glm::mat4x4& _camera_matrix, const glm::vec2& _screen_size) -> void;
    DPCPP_DLL_API auto switch_device(const std::string& _device_type, const std::string& _device_name) -> bool;
};