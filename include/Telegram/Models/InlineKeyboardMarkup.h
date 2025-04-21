#pragma once

#include <vector>
#include <nlohmann/json.hpp>
#include "InlineKeyboardButton.h"

namespace Telegram::Models {

    struct InlineKeyboardMarkup {
        using Button = InlineKeyboardButton;
        using Row = std::vector<Button>;

        InlineKeyboardMarkup() = default;

        void add_row(const Row& row) {
            inline_keyboard_.push_back(row);
        }

        void add_button(const Button& button) {
            if (inline_keyboard_.empty())
                inline_keyboard_.emplace_back();
            inline_keyboard_.back().push_back(button);
        }

        nlohmann::json serialize() const {
            nlohmann::json keyboard = nlohmann::json::array();
            for (auto const& row : inline_keyboard_) {
                nlohmann::json jr = nlohmann::json::array();
                for (auto const& btn : row)
                    jr.push_back(btn.serialize());
                keyboard.push_back(jr);
            }
            return {{"inline_keyboard", keyboard}};
        }

    private:
        std::vector<Row> inline_keyboard_;
    };

} // namespace Telegram::Models