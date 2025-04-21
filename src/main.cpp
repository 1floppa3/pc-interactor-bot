#include <Bot/TelegramBot.h>
#include <Core/AppManager.h>
#include <Core/Config.h>
#include <Core/Logger.h>

using App = Core::AppManager;

int main() {
    GUARD(App::init_console(), "Console initialization failed");
    GUARD(App::ensure_admin(), "Privilege elevation failed");
    GUARD(App::update_startup(), "Update start-up registry failed");

    const std::wstring config_path = Core::AppManager::get_program_dir() + L"\\config.json";
    GUARD(Core::Config::load(Utils::ws2s(config_path)), "Error loading config");

    GUARD({
        Bot::TelegramBot bot(Core::Config::get<std::string>("telegram_api_token"));
        bot.run();
    }, "Telegram bot execution failed");

    return 0;
}
