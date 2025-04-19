#include "Commands/CommandRegistry.h"

#include <Core/Config.h>
#include <Telegram/TelegramApi.h>

using namespace Telegram;

namespace Commands {

    CommandRegistry& CommandRegistry::instance() {
        static CommandRegistry inst;
        return inst;
    }

    void CommandRegistry::register_command(std::unique_ptr<ICommand> cmd) {
        cmds_.emplace(cmd->name(), std::move(cmd));
    }

    std::vector<std::string> CommandRegistry::list_commands() const {
        std::vector<std::string> names;
        for (auto& [k, _]: cmds_) names.push_back(k);
        return names;
    }

    void CommandRegistry::handle(const Models::Message& msg) {
        auto text = msg.text;
        const auto pos = text.find(' ');
        const auto cmd = pos == std::string::npos ? text : text.substr(0, pos);
        const auto it = cmds_.find(cmd);
        if (it != cmds_.end()) {
            it->second->execute(msg.chat_id, msg);
        } else {
            const TelegramApi api(Core::Config::get<std::string>("telegram_api_token"));
            api.sendMessage(msg.chat_id, "Unknown command: " + cmd);
        }
    }

    static bool _initialized = []() {
        auto& reg = CommandRegistry::instance();
        static TelegramApi api(Core::Config::get<std::string>("telegram_api_token"));
        reg.register_command(std::make_unique<StartCommand>(api));
        reg.register_command(std::make_unique<HelpCommand>(api));
        return true;
    }();

}
