#pragma once

#include <Telegram/TelegramApi.h>
#include "ICommand.h"

namespace Commands {

    class CameraCommand final : public ICommand {
        Telegram::TelegramApi& api_;
    public:
        explicit CameraCommand(Telegram::TelegramApi& api): api_(api) {}

        [[nodiscard]] std::string name() const override { return "/camera"; }
        [[nodiscard]] std::string description() const override { return "📷 Takes a snapshot from the webcam"; }
        [[nodiscard]] bool admin_only() const override { return true; }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
    };

}
