#include <Commands/CommandRegistry.h>
#include <Commands/HelpCommand.h>
#include <Core/Config.h>

namespace Commands {

    void HelpCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        auto cmds = CommandRegistry::instance().list_commands();
        std::ranges::sort(cmds, {}, [](const ICommand* c) { return c->name(); });
        auto admins = Core::Config::get<std::vector<int64_t>>("admin_ids");
        const bool is_admin = std::ranges::contains(admins, msg.from.id);

        std::string text = "Available commands:\n";
        for (const auto& cmd: cmds) {
            if (cmd->admin_only() && !is_admin)
                text += "[✖️] ";
            text += cmd->name() + " — ";
            text += cmd->description() + "\n";
        }
        api_.sendMessage(chat_id, text);
    }

}
