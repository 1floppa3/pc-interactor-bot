#include <iostream>
#include <Bot/TelegramBot.h>
#include <Core/Config.h>

int main() {
    try {
        Core::Config::load("../config.json");
    } catch (std::exception& ex) {
        std::cerr << "Error loading config: " << ex.what() << "\n"; // change to define log
        return 1;
    }

    Bot::TelegramBot bot(Core::Config::get<std::string>("telegram_api_token"));
    bot.run();
    return 0;
}
