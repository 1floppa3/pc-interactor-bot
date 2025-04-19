#include <Commands/CommandRegistry.h>
#include <Commands/HelpCommand.h>

namespace Commands {

    void HelpCommand::execute(int64_t chat_id, const Telegram::Models::Message&) {
        auto names = CommandRegistry::instance().list_commands();
        std::string text = "Доступные команды:\n";
        for (auto& n : names) text += n + "\n";
        api_.sendMessage(chat_id, text);
    }

}