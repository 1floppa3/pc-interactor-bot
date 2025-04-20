#include <Commands/ShutdownPCCommand.h>
#include <Core/Config.h>

namespace Commands {

    void ShutdownPCCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        auto admins = Core::Config::get<std::vector<int64_t>>("admin_ids");
        const bool is_admin = std::ranges::find(admins, msg.from.id) != admins.end();
        if (!is_admin) {
            api_.sendMessage(chat_id, "❌ Forbidden. This command is for admins only.");
            return;
        }

        api_.sendMessage(chat_id, "⚙️ Shutting down the PC...");

        std::system("shutdown /s /t 0");
    }

}
