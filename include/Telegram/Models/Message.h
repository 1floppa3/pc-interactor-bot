#pragma once

#include "User.h"

namespace Telegram::Models {

    struct Message {
        int64_t message_id{};
        User from{};
        int64_t chat_id{};
        std::string text{};

        Message(const int64_t message_id, User from, const int64_t chat_id, std::string text):
                message_id(message_id), from(std::move(from)), chat_id(chat_id), text(std::move(text)) {}
        Message(const int64_t chat_id, std::string text):
                chat_id(chat_id), text(std::move(text)) {}
        Message() = default;
    };

}
