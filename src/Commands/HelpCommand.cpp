#include <Commands/CommandDispatcher.h>
#include <Commands/HelpCommand.h>
#include <Core/Config.h>
#include <Core/SystemController.h>

namespace Commands {

    void HelpCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        auto cmds = CommandDispatcher::instance().list_commands();
        std::ranges::sort(cmds, {}, [](const ICommand* c) { return c->name(); });

        auto admins = Core::Config::get<std::vector<int64_t>>("admin_ids");
        const bool is_admin = std::ranges::contains(admins, msg.from.id);

        const std::string pc_info = Core::SystemController::get_computer_name() + "\\" + Core::SystemController::get_username();

        std::ostringstream out;
        out << "💻 <b>Connected PC:</b> <code>" << pc_info << "</code>\n";
        out << "🔐 <b>Access:</b> " << (is_admin ? "Administrator ✅" : "User ❌") << "\n\n";

        out << "📖 <b>Available commands:</b>\n";

        out << "\n<b>👥 Common:</b>\n";
        for (const auto& cmd : cmds) {
            if (!cmd->admin_only()) {
                out << "• <b>" << cmd->name() << "</b> — " << cmd->description() << "\n";
            }
        }

        out << "\n<b>🛠 Admin only:</b>\n";
        for (const auto& cmd : cmds) {
            if (cmd->admin_only()) {
                if (!is_admin)
                    out << "❌ ";
                out << "• <b>" << cmd->name() << "</b> — " << cmd->description() << "\n";
            }
        }

        api_.sendMessage(chat_id, out.str());
    }

}
