#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class SayCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit SayCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/say"; }
        [[nodiscard]] std::string description() const override { return "🗣️ Say given text via PC speaker"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

    }
