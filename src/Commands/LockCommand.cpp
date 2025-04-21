#include <Commands/LockCommand.h>
#include <Core/Config.h>

#include <Windows.h>

namespace Commands {

    void LockCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        if (LockWorkStation())
            api_.sendMessage(chat_id, "🔒 Workstation locked");
        else
            api_.sendMessage(chat_id, "❌ Failed to lock workstation");
    }


}
