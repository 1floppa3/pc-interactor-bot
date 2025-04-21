#include <Commands/MediaNextCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void MediaNextCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        Core::SystemController::media_next();
        api_.sendMessage(chat_id, "<b>⏭ Skipped to next track</b>");
    }

}
