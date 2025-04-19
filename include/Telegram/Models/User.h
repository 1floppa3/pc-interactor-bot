#pragma once

#include <cstdint>
#include <string>

namespace Telegram::Models {

    struct User {
        int64_t id{};
        std::string username{};
        std::string first_name{};
        std::string last_name{};

        User(const int64_t id, std::string username, std::string first_name, std::string last_name):
            id(id), username(std::move(username)), first_name(std::move(first_name)), last_name(std::move(last_name)) {}
        explicit User(const int64_t id): id(id) {}
        User() = default;
    };

}
