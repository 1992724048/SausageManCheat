#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <format>
#include <string_view>
#include <windows.h>

class Logger {
public:
    enum class Level { None, Critical, Error, Warning, Info, Debug, Trace };

    enum class LoggerType { Any, ConsoleLogger, FileLogger };

    static auto SetLevel(const Level level, const LoggerType type = LoggerType::Any) -> void {
        std::scoped_lock lock(m_mutex_);
        switch (type) {
            case LoggerType::Any:
                s_console_log_level_ = level;
                s_file_log_level_ = level;
                break;
            case LoggerType::ConsoleLogger:
                s_console_log_level_ = level;
                break;
            case LoggerType::FileLogger:
                s_file_log_level_ = level;
                break;
        }
    }

    static auto GetLevel(const LoggerType type) -> Level {
        std::scoped_lock lock(m_mutex_);
        switch (type) {
            case LoggerType::Any:
                return s_file_log_level_ < s_console_log_level_ ? s_file_log_level_ : s_console_log_level_;
            case LoggerType::ConsoleLogger:
                return s_console_log_level_;
            case LoggerType::FileLogger:
                return s_file_log_level_;
        }
        return Level::None;
    }

    static auto PrepareFileLogging(const std::filesystem::path& dir) -> void {
        std::scoped_lock lock(m_mutex_);
        s_directory_ = dir;
        if (!is_directory(dir)) {
            create_directories(dir);
        }
        const auto now = std::chrono::system_clock::now();
        s_log_file_path_ = std::format("{}/log_{}.txt", dir.string(), now.time_since_epoch().count());
    }

    class LogMessage {
    public:
        LogMessage(const Level level, const std::string_view file, const int line) : m_level_(level), m_file_(file), m_line_(line) {}

        ~LogMessage() {
#ifdef _DEBUG
            commit();
#endif
        }

        template<typename T>
        auto operator<<(const T& value) -> LogMessage& {
#ifdef _DEBUG
            m_stream_ << value;
#endif
            return *this;
        }

        auto operator<<(std::ostream& (*manip)(std::ostream&)) -> LogMessage& {
#ifdef _DEBUG
            m_stream_ << manip;
#endif
            return *this;
        }

    private:
        auto commit() const -> void {
#ifdef _DEBUG
            Log(m_level_, std::string(m_file_), m_line_, m_stream_.str());
#endif
        }

        Level m_level_;
        std::string m_file_;
        int m_line_;
        std::ostringstream m_stream_;
    };

    static auto OpenConsole() -> void {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();
        }

        FILE* fp_out_;
        freopen_s(&fp_out_, "CONOUT$", "w", stdout);
        freopen_s(&fp_out_, "CONOUT$", "w", stderr);
        freopen_s(&fp_out_, "CONIN$", "r", stdin);
        _wfreopen_s(&fp_out_, L"CONOUT$", L"w", stdout);
        _wfreopen_s(&fp_out_, L"CONOUT$", L"w", stderr);
        _wfreopen_s(&fp_out_, L"CONIN$", L"r", stdin);

        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);

        SetConsoleTitleA("日志");

        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);

        std::ios::sync_with_stdio(false);
        std::cout.setf(std::ios::unitbuf);
        std::cerr.setf(std::ios::unitbuf);
    }

    static auto Log(Level level, const std::string& file, int line, const std::string& msg) -> void {
        bool log_to_console = (s_console_log_level_ != Level::None && s_console_log_level_ >= level);
        bool log_to_file = (s_file_log_level_ != Level::None && s_file_log_level_ >= level);

        std::filesystem::path p(file);
        std::string filename = p.filename().string();

        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        tm tm_time;
        gmtime_s(&tm_time, &t);
        std::ostringstream time_stream;
        time_stream << std::setw(2) << std::setfill('0') << tm_time.tm_hour << ":" << std::setw(2) << std::setfill('0') << tm_time.tm_min << ":" << std::setw(2) << std::setfill('0') << tm_time.tm_sec;

        static const auto kStartTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - kStartTime;
        std::ostringstream runtime_stream;
        runtime_stream << std::fixed << std::setprecision(3) << elapsed.count() << "s";

        const char* level_text = getLevelText(level);

        std::lock_guard lock(m_mutex_);
        if (log_to_console) {
            WORD color;
            switch (level) {
                case Level::Critical:
                    color = FOREGROUND_RED | FOREGROUND_INTENSITY;
                    break;
                case Level::Error:
                    color = FOREGROUND_RED;
                    break;
                case Level::Warning:
                    color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                    break;
                case Level::Info:
                    color = FOREGROUND_GREEN;
                    break;
                case Level::Debug:
                    color = FOREGROUND_BLUE | FOREGROUND_GREEN;
                    break;
                case Level::Trace:
                    color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                    break;
                default:
                    color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                    break;
                case Level::None:
                    break;
            }

            SetConsoleColor(color);
            std::cout << "[" << level_text << "] ";
            SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            std::cout << "[" << runtime_stream.str() << "] ";
            std::cout << "[TID:" << std::to_string(GetCurrentThreadId()) << "] ";
            SetConsoleColor(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            std::cout << "[" << filename << ":" << line << "] ";
            SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            std::cout << ">> " << msg << '\n';
        }

        if (log_to_file) {
            std::ostringstream file_stream;
            file_stream << "[" << time_stream.str() << "] " << "[" << level_text << "] " << "[" << filename << ":" << line << "] " << msg;
            std::ofstream ofs(s_log_file_path_, std::ios::out | std::ios::app | std::ios::binary);
            if (ofs) {
                ofs << file_stream.str() << '\n';
            }
        }
    }

private:
    static constexpr auto getLevelText(const Level level) -> const char* {
        switch (level) {
            case Level::Critical:
                return "CRITICAL";
            case Level::Error:
                return "ERROR";
            case Level::Warning:
                return "WARNING";
            case Level::Info:
                return "INFO";
            case Level::Debug:
                return "DEBUG";
            case Level::Trace:
                return "TRACE";
            default:
                return "";
        }
    }

    static auto SetConsoleColor(const WORD color) -> void {
        const HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(h_console, color);
    }

    inline static auto s_file_log_level_ = Level::None;
    inline static auto s_console_log_level_ = Level::None;
    inline static std::string s_log_file_path_;
    inline static std::filesystem::path s_directory_;
    inline static std::mutex m_mutex_;
};

#define LOG_CRIT ::Logger::LogMessage(::Logger::Level::Critical, __FILE__, __LINE__)
#define LOG_ERROR ::Logger::LogMessage(::Logger::Level::Error, __FILE__, __LINE__)
#define LOG_WARNING ::Logger::LogMessage(::Logger::Level::Warning, __FILE__, __LINE__)
#define LOG_INFO ::Logger::LogMessage(::Logger::Level::Info, __FILE__, __LINE__)
#define LOG_DEBUG ::Logger::LogMessage(::Logger::Level::Debug, __FILE__, __LINE__)
#define LOG_TRACE ::Logger::LogMessage(::Logger::Level::Trace, __FILE__, __LINE__)
