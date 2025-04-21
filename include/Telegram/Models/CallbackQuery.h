#pragma once

#include <string>

namespace Telegram::Models {

    struct CallbackQuery {
        std::string id;
        User from;
        std::string data;
        Message message;
        explicit CallbackQuery(const nlohmann::json &json) {
            id = json["id"].get<std::string>();
            from = User(json["from"]);
            data = json["data"].get<std::string>();
            message = Message(json["message"]);
        }
    };

}