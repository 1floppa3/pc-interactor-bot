#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class MediaToggleCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit MediaToggleCommand(Telegram::TelegramApi& api) : api_(api) {}

        [[nodiscard]] std::string name() const override { return "/media_toggle"; }
        [[nodiscard]] std::string description() const override { return "⏯ Play/Pause media"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}