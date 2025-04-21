#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class MonitorCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit MonitorCommand(Telegram::TelegramApi& api) : api_(api) {}

        [[nodiscard]] std::string name() const override { return "/monitor"; }
        [[nodiscard]] std::string description() const override { return "📈 Shows current CPU, RAM, disk and uptime"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}