#pragma once

#include <sstream>
#include <fmt/color.h>

namespace Core {

    class Logger {
    public:
        enum class Level { L_INFO, L_WARN, L_ERROR, L_USERLOG, L_RAW };

        template<typename... Args>
        static void log(const Level lvl, Args&&... args) {
            std::ostringstream oss;
            (oss << ... << std::forward<Args>(args));

            auto clr = fmt::color::white;
            auto tag = "";
            switch (lvl) {
                case Level::L_INFO:    clr = fmt::color::light_blue;  tag = "[INFO] ";        break;
                case Level::L_WARN:    clr = fmt::color::orange;      tag = "[WARN] ";        break;
                case Level::L_ERROR:   clr = fmt::color::indian_red;  tag = "[FATAL ERROR] "; break;
                case Level::L_USERLOG: clr = fmt::color::light_green; tag = "[User] ";        break;
                case Level::L_RAW:     clr = fmt::color::light_blue;                         break;
            }

            fmt::print(stderr, "{}{}\n", styled(tag, fmt::emphasis::bold | fg(clr)), styled(oss.str(), fg(clr)));
        }
    };
}

#define LOG_INFO(...)  Core::Logger::log(Core::Logger::Level::L_INFO,    __VA_ARGS__)
#define LOG_WARN(...)  Core::Logger::log(Core::Logger::Level::L_WARN,    __VA_ARGS__)
#define LOG_ERROR(...) Core::Logger::log(Core::Logger::Level::L_ERROR,   __VA_ARGS__)
#define LOG_USER(...)  Core::Logger::log(Core::Logger::Level::L_USERLOG, __VA_ARGS__)
#define LOG_RAW(...)   Core::Logger::log(Core::Logger::Level::L_RAW,     __VA_ARGS__)
