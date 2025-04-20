#pragma once

#include <Telegram/TelegramApi.h>

#include "ICommand.h"

namespace Commands {

    class StartCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit StartCommand(Telegram::TelegramApi& api): api_(api) {}
        [[nodiscard]] std::string name() const override {
            return "/start";
        }
        [[nodiscard]] std::string description() const override {
            return "(re)Starts the bot";
        }
        [[nodiscard]] bool admin_only() const override {
            return false;
        }
        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}
