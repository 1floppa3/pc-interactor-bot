#pragma once

#include <sstream>
#include <fmt/color.h>

#include "windows.h"

namespace Core {

    class Logger {
        static void init_console() {
            static bool inited = false;
            if (!inited) {
                const auto h = GetStdHandle(STD_ERROR_HANDLE);
                DWORD mode;
                GetConsoleMode(h, &mode);
                SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
                inited = true;
            }
        }
    public:
        enum class Level { L_INFO, L_WARN, L_ERROR, L_USERLOG };

        template<typename... Args>
        static void log(const Level lvl, Args&&... args) {
            init_console();

            std::ostringstream oss;
            (oss << ... << std::forward<Args>(args));

            auto clr = fmt::color::white;
            auto tag = "";
            switch (lvl) {
                case Level::L_INFO:    clr = fmt::color::light_blue;  tag = "INFO";  break;
                case Level::L_WARN:    clr = fmt::color::orange;      tag = "WARN";  break;
                case Level::L_ERROR:   clr = fmt::color::indian_red;  tag = "ERROR"; break;
                case Level::L_USERLOG: clr = fmt::color::light_green; tag = "User";  break;
            }

            fmt::print(stderr, "[{}] {}\n", styled(tag, fmt::emphasis::bold | fg(clr)), styled(oss.str(), fg(clr)));
        }
    };
}

#define LOG_INFO(...)  Core::Logger::log(Core::Logger::Level::L_INFO,    __VA_ARGS__)
#define LOG_WARN(...)  Core::Logger::log(Core::Logger::Level::L_WARN,    __VA_ARGS__)
#define LOG_ERROR(...) Core::Logger::log(Core::Logger::Level::L_ERROR,   __VA_ARGS__)
#define LOG_USER(...)  Core::Logger::log(Core::Logger::Level::L_USERLOG, __VA_ARGS__)
