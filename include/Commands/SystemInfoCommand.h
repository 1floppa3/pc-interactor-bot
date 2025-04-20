#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class SystemInfoCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit SystemInfoCommand(Telegram::TelegramApi& api) : api_(api) {}

        [[nodiscard]] std::string name() const override { return "/systeminfo"; }
        [[nodiscard]] std::string description() const override { return "Shows general system information"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}
