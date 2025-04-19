#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

namespace Core {

    class Config {
        static nlohmann::json config_;
    public:
        static void load(const std::string& path = "config.json");

        template<typename T>
        static T get(const std::string& key) {
            if (!config_.contains(key)) {
                throw std::runtime_error("Config: key not found: " + key);
            }
            try {
                return config_.at(key).get<T>();
            }
            catch (const std::exception& ex) {
                throw std::runtime_error("Config: key \"" + key + "\": " + ex.what());
            }
        }
    };

}