#include "Commands/CommandDispatcher.h"

#include <Commands/CommandList.h>
#include <Core/Config.h>
#include <Core/Logger.h>

using namespace Telegram;

namespace Commands {

    CommandDispatcher& CommandDispatcher::instance() {
        static CommandDispatcher inst;
        return inst;
    }

    std::vector<ICommand*> CommandDispatcher::list_commands() const {
        std::vector<ICommand*> ptrs;
        ptrs.reserve(cmds_.size());
        for (auto& [_, cmd]: cmds_)
            ptrs.push_back(cmd.get());
        return ptrs;
    }
    void CommandDispatcher::register_command(std::unique_ptr<ICommand> cmd) {
        cmds_.emplace(cmd->name(), std::move(cmd));
    }
    void CommandDispatcher::register_all_commands(TelegramApi& api) {
#define REGISTER(C) register_command(std::make_unique<C>(api));
        COMMAND_LIST(REGISTER)
#undef REGISTER
    }

    void CommandDispatcher::handle(const TelegramApi& api, const Models::Message& msg) {
        auto text = msg.text;
        const auto pos = text.find(' ');
        const auto cmd = pos == std::string::npos ? text : text.substr(0, pos);
        const auto it = cmds_.find(cmd);
        if (it != cmds_.end()) {
            const auto &command = it->second;
            if (command->admin_only()) {
                auto admins = Core::Config::get<std::vector<int64_t>>("admin_ids");
                if (!std::ranges::contains(admins, msg.from.id)) {
                    api.sendMessage(msg.chat_id, "❌ Forbidden. This command is for admins only.");
                    return;
                }
            }
            command->execute(msg.chat_id, msg);
            LOG_USER("User (", msg.from.first_name, msg.from.last_name.empty() ? "" : " ", msg.from.last_name,
                ", @", msg.from.username, ") command: ", command->name());
        } else {
            api.sendMessage(msg.chat_id, "Unknown command: " + cmd);
        }
    }
    void CommandDispatcher::handle_callback(const Models::CallbackQuery &cq) {
        for (auto& [name, cmd_ptr] : cmds_) {
            if (const auto cb = dynamic_cast<ICallbackCommand*>(cmd_ptr.get())) {
                if (cb->can_handle_callback(cq.data)) {
                    cb->handle_callback(cq);
                    LOG_USER("User (", cq.from.first_name, cq.from.last_name.empty() ? "" : " ", cq.from.last_name,
                        ", @", cq.from.username, ") callback for ", name, ": data=", cq.data);
                    return;
                }
            }
        }
        LOG_WARN("Unhandled callback data: ", cq.data);
    }


}
