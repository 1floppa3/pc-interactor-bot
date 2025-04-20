#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class ScreenshotCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit ScreenshotCommand(Telegram::TelegramApi& api) : api_(api) {}

        [[nodiscard]] std::string name() const override { return "/screenshot"; }
        [[nodiscard]] std::string description() const override { return "Takes a screenshot and sends it"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}