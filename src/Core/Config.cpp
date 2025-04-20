#include <Core/Config.h>

namespace Core {

    nlohmann::json Config::config_;

    void Config::load(const std::string& path) {
        std::ifstream in(path);
        if (!in.is_open())
            throw std::runtime_error("Can't open " + path);
        in >> config_;
    }

}
