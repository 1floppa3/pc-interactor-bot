#pragma once

#include <Telegram/TelegramApi.h>

#include "ICommand.h"

namespace Commands {

    class ShutdownPCCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit ShutdownPCCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/shutdown_pc"; }
        [[nodiscard]] std::string description() const override { return "⚠️ Shut down the PC"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}
