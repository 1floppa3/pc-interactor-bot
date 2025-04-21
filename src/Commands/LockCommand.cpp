#include <Commands/LockCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void LockCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        if (Core::SystemController::lock())
            api_.sendMessage(chat_id, "<b>🔒 PC locked</b>");
        else
            api_.sendMessage(chat_id, "<b>❌ Failed to lock PC</b>");
    }


}
