#include <Commands/HibernateCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void HibernateCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        api_.sendMessage(chat_id, "<b>🌙 Going into hibernation...</b>");
        Core::SystemController::hibernate();
    }

}
