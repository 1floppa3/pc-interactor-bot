#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {
    class RunCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit RunCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/run"; }
        [[nodiscard]] std::string description() const override { return "💻 Run a shell command"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };
}
