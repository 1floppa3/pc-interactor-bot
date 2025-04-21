#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class MediaNextCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit MediaNextCommand(Telegram::TelegramApi& api) : api_(api) {}

        [[nodiscard]] std::string name() const override { return "/media_next"; }
        [[nodiscard]] std::string description() const override { return "⏭ Next media track"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}