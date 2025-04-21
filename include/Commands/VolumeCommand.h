#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class VolumeCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit VolumeCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/volume"; }
        [[nodiscard]] std::string description() const override { return "	🔊 Show or set the system volume (0–100%)"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}