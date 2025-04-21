#pragma once

#include "Message.h"
#include "CallbackQuery.h"
#include <nlohmann/json.hpp>

namespace Telegram::Models {

    struct Update {
        int64_t update_id{};
        Message message;
        std::optional<CallbackQuery> callback_query;

        Update(const int64_t update_id, Message message):
            update_id(update_id), message(std::move(message)) {}
        explicit Update(const nlohmann::json &json) {
            update_id = json["update_id"].get<int64_t>();
            if (json.contains("message"))
                message = Message(json["message"]);
            if (json.contains("callback_query"))
                callback_query = CallbackQuery(json["callback_query"]);
        }
        Update() = default;
    };

}
