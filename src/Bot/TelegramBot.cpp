#include "Bot/TelegramBot.h"

#include <thread>
#include <Commands/CommandRegistry.h>
#include <Core/Config.h>
#include "Core/Logger.h"

namespace Bot {

    TelegramBot::TelegramBot(const std::string& token): api_(token) {}

    void TelegramBot::run() {
        LOG_INFO("Bot started");
        const int timeout = Core::Config::get<int>("long_polling_timeout");
        Commands::CommandRegistry::instance().register_all_commands(api_);
        while (true) {
            try {
                auto updates = api_.getUpdates(last_update_id_, timeout);
                for (auto& u: updates) {
                    last_update_id_ = u.update_id + 1;
                    Commands::CommandRegistry::instance().handle(u.message);
                }
            } catch (std::exception& ex) {
                LOG_ERROR("run() error: ", ex.what());
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }

}
