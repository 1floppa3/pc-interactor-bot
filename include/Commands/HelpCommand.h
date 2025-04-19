#pragma once

#include <Telegram/TelegramApi.h>

#include "ICommand.h"

namespace Commands {

    class HelpCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit HelpCommand(Telegram::TelegramApi& api): api_(api) {}
        [[nodiscard]] std::string name() const override {
          return "/help";
        }
        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}
