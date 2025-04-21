#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace Telegram::Models {

    struct InlineKeyboardButton {
        std::string text;
        std::string callback_data;

        InlineKeyboardButton() = default;
        InlineKeyboardButton(std::string t, std::string cb): text(std::move(t)), callback_data(std::move(cb)) {}

        nlohmann::json serialize() const {
            return {
                {"text", text},
                {"callback_data", callback_data}
            };
        }
    };

}