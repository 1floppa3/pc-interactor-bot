#include "Commands/CommandRegistry.h"

#include <Commands/HelpCommand.h>
#include <Core/Config.h>
#include <Core/Logger.h>
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

    std::vector<ICommand*> CommandRegistry::list_commands() const {
        std::vector<ICommand*> ptrs;
        ptrs.reserve(cmds_.size());
        for (auto& [_, cmd]: cmds_)
            ptrs.push_back(cmd.get());
        return ptrs;
    }

    void CommandRegistry::handle(const Models::Message& msg) {
        auto text = msg.text;
        const auto pos = text.find(' ');
        const auto cmd = pos == std::string::npos ? text : text.substr(0, pos);
        const auto it = cmds_.find(cmd);
        if (it != cmds_.end()) {
            it->second->execute(msg.chat_id, msg);
            LOG_USER(                "User (", msg.from.first_name, msg.from.last_name.empty() ? "" : " ", msg.from.last_name,
                ", @", msg.from.username, ") command: ", it->second->name());
        } else {
            const TelegramApi api(Core::Config::get<std::string>("telegram_api_token"));
            api.sendMessage(msg.chat_id, "Unknown command: " + cmd);
        }
    }

}
