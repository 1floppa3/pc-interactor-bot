#include <Commands/ShutdownCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void ShutdownCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        api_.sendMessage(chat_id, "<b>⚙️ Shutting down the PC...</b>");
        Core::SystemController::shutdown();
    }

}
