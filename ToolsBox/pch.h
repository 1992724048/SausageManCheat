#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_ZLIB_SUPPORT
#define GLM_ENABLE_EXPERIMENTAL
#include <minmalloc/mimalloc-override.h>

// Windows 头文件
#include <windows.h>
#include <windowsx.h>
#include <SDKDDKVer.h>
#include <CommCtrl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <wrl.h>
#include <shellapi.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <WinSock2.h>
#include <mswsock.h>
#include <Winternl.h>
// C/C++ 运行时头文件
#include <cstdlib>
#include <malloc.h>
#include <memory.h>
#include <memory>
#include <tchar.h>
#include <vector>
#include <complex>
#include <numbers>
#include <iostream>
#include <fstream>
#include <format>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <map>
#include <locale>
#include <unordered_set>
#include <shared_mutex>
#include <array>
#include <functional>
#include <cstddef>
#include <iomanip>
#include <algorithm>
#include <coroutine>
#include <mutex>
#include <limits>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <stacktrace>
#include <future>
#include <condition_variable>
#include <atomic>
#include <codecvt>
// oneapi
#include <mkl.h>
#include <mpi.h>
#include <ipp.h>
#include <tbb/tbb.h>
#include <omp.h>
// 第三方库
#include <units.h>
#include <phmap.h>
#include <httplib.h>
#include <magic_enum/magic_enum_all.hpp>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <libipc/ipc.h>
// 自己的库
#include <thread_pool.hpp>
#include <encode.h>
#include <crash_dump.h>
#include <tray.h>
#include <util.h>
#include <events/event.hpp>
#include <resource.h>
#include <config/Config.h>

using namespace units::literals;
using namespace units::length;
using namespace units::time;
using namespace units::area;
using namespace units::velocity;
using namespace std::chrono_literals;