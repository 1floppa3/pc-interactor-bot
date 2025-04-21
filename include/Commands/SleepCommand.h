#pragma once

#include <Telegram/TelegramApi.h>

#include "ICommand.h"

namespace Commands {

    class SleepCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit SleepCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/sleep"; }
        [[nodiscard]] std::string description() const override { return "😴 Put the PC into sleep mode"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}
