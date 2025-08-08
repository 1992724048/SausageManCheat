#pragma once
#pragma comment(lib, "mimalloc.dll.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")

#define GAME_NAME L"Sausage Man.exe"
#define GLM_ENABLE_EXPERIMENTAL
#define USE_GLM

#include <Windows.h>
#include <d3d11.h>
#include <TlHelp32.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "minmalloc/mimalloc.h"
#include "parallel_hashmap/phmap.h"
#include <events/event.hpp>
#include <mutex>
#include <memory>
#include <omp.h>
#include <optional>
#include <exception>
#include <stdexcept>
#include <functional>
#include <utility>
#include <Logger.h>
#include <encode.h>
#include <UnityResolve.hpp>
#include <crash_dump.h>
#include <resource.h>
#include "util.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "tbb/oneapi/tbb.h"

import glm;
using namespace std::chrono_literals;

using I = UnityResolve;
using IM = UnityResolve::Method;
using IC = UnityResolve::Class;
using IT = UnityResolve::Type;
using IF = UnityResolve::Field;
using IA = UnityResolve::Assembly;
using II = UnityResolve::UnityType;