#pragma once

#include <Telegram/TelegramApi.h>

#include "ICommand.h"

namespace Commands {

    class KillCommand final : public ICommand {
    public:
        explicit KillCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/kill"; }
        [[nodiscard]] std::string description() const override{ return "⚠️ Kill process by PID"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;

    private:
        Telegram::TelegramApi& api_;
    };

}