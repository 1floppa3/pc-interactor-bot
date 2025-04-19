#pragma once

#include <iostream>
#include <string>

namespace Core {

    enum class Level { INFO, WARN, ERROR };

    class Logger {
    public:
        static void log(Level lvl, const std::string& msg);
    };

#define LOG_INFO(msg)  Core::Logger::log(Core::Level::INFO, msg)
#define LOG_WARN(msg)  Core::Logger::log(Core::Level::WARN, msg)
#define LOG_ERROR(msg) Core::Logger::log(Core::Level::ERROR, msg)

}