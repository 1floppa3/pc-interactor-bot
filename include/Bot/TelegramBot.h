#pragma once

#include <Commands/CommandRegistry.h>

#include "../Telegram/TelegramApi.h"

namespace Bot {

    class TelegramBot {
        Telegram::TelegramApi api_;
        int64_t last_update_id_{0};
    public:
        explicit TelegramBot(const std::string& token);
        void run();
    };

}
