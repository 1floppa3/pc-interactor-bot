#pragma once

#include <Telegram/TelegramApi.h>

#include "ICommand.h"

namespace Commands {

    class HibernateCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit HibernateCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/hibernate"; }
        [[nodiscard]] std::string description() const override { return "🌙 Hibernate the PC"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}
