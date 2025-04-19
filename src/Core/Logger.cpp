#include "../../include/Core/Logger.h"

namespace Core {

    void Logger::log(const Level lvl, const std::string &msg) {
        auto pre = "";
        switch (lvl) {
            case Level::INFO:  pre = "[INFO] ";  break;
            case Level::WARN:  pre = "[WARN] ";  break;
            case Level::ERROR: pre = "[ERROR] "; break;
        }
        std::cerr << pre << msg << "\n";
    }

}