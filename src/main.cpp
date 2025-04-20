#include <iostream>
#include <Bot/TelegramBot.h>
#include <Commands/HelpCommand.h>
#include <Commands/ShutdownPCCommand.h>
#include <Commands/StartCommand.h>
#include <Core/Config.h>
#include <Core/Logger.h>
#include <Core/PrivilegeHelper.h>

int main() {
    try {
        Core::PrivilegeHelper::ensure_admin();
    } catch (const std::exception& ex) {
        LOG_ERROR("Privilege elevation failed: ", ex.what());
        return 1;
    }

    try {
        Core::Config::load("../config.json");
    } catch (std::exception& ex) {
        LOG_ERROR("Error loading config: ", ex.what(), "\n");
        return 1;
    }

    Bot::TelegramBot bot(Core::Config::get<std::string>("telegram_api_token"));

    bot.register_command<Commands::StartCommand>();
    bot.register_command<Commands::HelpCommand>();
    bot.register_command<Commands::ShutdownPCCommand>();

    bot.run();
    return 0;
}
