#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class LockCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit LockCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/lock"; }
        [[nodiscard]] std::string description() const override { return "🔒 Locks the workstation"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}