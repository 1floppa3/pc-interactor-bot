#pragma once

#include <Telegram/TelegramApi.h>

namespace Bot {

    class TelegramBot {
        Telegram::TelegramApi api_;
        int64_t last_update_id_{0};

        void notify_started() const;
        void register_commands();
        void configure_bot() const;
        void skip_old_updates();
        void polling_loop();
    public:
        explicit TelegramBot(const std::string& token);
        void run();
    };

}
