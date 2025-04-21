#include <Commands/StartCommand.h>
#include <Core/Config.h>
#include <Core/SystemController.h>

namespace Commands {

    void StartCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        const std::string pc_info = Core::SystemController::get_computer_name() + "\\" + Core::SystemController::get_username();
        const auto admins = Core::Config::get<std::vector<int64_t>>("admin_ids");
        const bool is_admin = std::ranges::contains(admins, msg.from.id);

        std::ostringstream out;

        out << "👋 <b>Welcome, " << msg.from.first_name << "!</b>\n\n";
        out << "💻 Connected PC: <code>" << pc_info << "</code>\n";
        out << "🔐 Access: " << (is_admin ? "Administrator ✅" : "User ❌") << "\n\n";
        out << "📖 Use <b>/help</b> to view available commands.\n";

        if (!is_admin) {
            out << "\n⚠️ Some features are only available to administrators.";
        }

        api_.sendMessage(chat_id, out.str());
    }

}
