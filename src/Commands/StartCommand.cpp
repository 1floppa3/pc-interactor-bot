#include <Commands/StartCommand.h>
#include <Core/Config.h>

namespace Commands {

    void StartCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        std::string text = "Hello, " + msg.from.first_name + ".\n\n";
        auto admins = Core::Config::get<std::vector<int64_t>>("admin_ids");
        const bool is_admin = std::ranges::find(admins, msg.from.id) != admins.end();
        text += is_admin
             ? "You are an administrator. You can use the features of this bot ✅"
             : "You are not an administrator. You can't use all the features of this bot ✖️";
        api_.sendMessage(chat_id, text);
    }

}
