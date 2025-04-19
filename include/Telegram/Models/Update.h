#pragma once

#include "Message.h"
#include <nlohmann/json.hpp>

namespace Telegram::Models {

    struct Update {
        int64_t update_id{};
        Message message{};

        Update(const int64_t update_id, Message message):
            update_id(update_id), message(std::move(message)) {}
        explicit Update(const nlohmann::json &json) {
            update_id = json["update_id"].get<int64_t>();

            auto &m = json["message"];
            const User user(
                m["from"]["id"].get<int64_t>(),
                m["from"].value("username", ""),
                m["from"].value("first_name", ""),
                m["from"].value("last_name", "")
            );
            const Message msg(
                m["message_id"].get<int64_t>(),
                user,
                m["chat"]["id"].get<int64_t>(),
                m.value("text", "")
            );
            message = msg;
        }
        Update::Update() = default;
    };

}
