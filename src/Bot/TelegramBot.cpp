#include "Bot/TelegramBot.h"

#include <thread>
#include <Commands/CommandDispatcher.h>
#include <Core/Config.h>
#include <Core/SystemController.h>

#include "Core/Logger.h"

namespace Bot {

    TelegramBot::TelegramBot(const std::string& token): api_(token) {}

    void TelegramBot::configure_bot() const {
        const auto admin_ids = Core::Config::get<std::vector<int64_t>>("admin_ids");
        try {
            const auto& cmds = Commands::CommandDispatcher::instance().list_commands();
            api_.setMyCommands(cmds);
            for (const int64_t id : admin_ids) // admin cmds
                api_.setMyCommands(cmds, true, id);
        } catch (const std::exception& ex) {
            LOG_WARN("Failed to set bot commands: ", ex.what());
        }

        try {
            api_.setChatMenuButton();
        } catch (const std::exception& ex) {
            LOG_WARN("Failed to set chat menu button: ", ex.what());
        }

        try {
            api_.setBotInfo(
                Core::Config::get<std::string>("bot_name"),
                Core::Config::get<std::string>("bot_short_description"),
                Core::Config::get<std::string>("bot_description")
            );
        } catch (const std::exception& ex) {
            LOG_WARN("Failed to set bot info: ", ex.what());
        }
    }
    void TelegramBot::notify_started() const {
        std::string pc_info = Core::SystemController::get_computer_name() + "\\" + Core::SystemController::get_username();

        LOG_INFO("Bot has started (PC: ", pc_info, ")");
        const auto admin_ids = Core::Config::get<std::vector<int64_t>>("admin_ids");
        for (const int64_t id : admin_ids) {
            api_.sendMessage(id, "🤖 Bot has started (PC: " + pc_info + ")");
        }
    }
    void TelegramBot::polling_loop() {
        const int timeout = Core::Config::get<int>("long_polling_timeout");
        while (true) {
            try {
                auto updates = api_.getUpdates(last_update_id_, timeout);
                for (auto& u: updates) {
                    last_update_id_ = u.update_id + 1;
                    if (u.callback_query.has_value()) {
                        Commands::CommandDispatcher::instance().handle_callback(*u.callback_query);
                    } else {
                        Commands::CommandDispatcher::instance().handle(api_, u.message);
                    }
                }
            } catch (std::exception& ex) {
                LOG_ERROR("run() error: ", ex.what());
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }
    void TelegramBot::skip_old_updates() {
        try {
            const auto updates = api_.getUpdates(0, 0);
            if (!updates.empty()) {
                last_update_id_ = updates.back().update_id + 1;
                LOG_INFO("Skipped ", updates.size(), " old updates. Starting from ID ", last_update_id_);
            }
        } catch (const std::exception& ex) {
            LOG_WARN("Failed to skip old updates: ", ex.what());
        }
    }

    void TelegramBot::run() {
        notify_started();

        Commands::CommandDispatcher::instance().register_all_commands(api_);
        configure_bot();

        if (Core::Config::get<bool>("skip_old_updates"))
            skip_old_updates();

        polling_loop();
    }

}
