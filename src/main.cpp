#include <iostream>
#include <thread>
#include <Bot/TelegramBot.h>
#include <Core/AppManager.h>
#include <Core/Config.h>
#include <Core/Logger.h>

using App = Core::AppManager;

int main() {
    App::init_console();

    GUARD(App::ensure_admin(), "Privilege elevation failed");
    GUARD(App::update_startup(), "Update start-up registry failed");
    GUARD(Core::Config::load("config.json"), "Error loading config");

    GUARD({
        Bot::TelegramBot bot(Core::Config::get<std::string>("telegram_api_token"));
        bot.run();
    }, "Telegram bot execution failed");

    return 0;
}
