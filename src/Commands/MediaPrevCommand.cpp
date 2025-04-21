#include <Commands/MediaPrevCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void MediaPrevCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        Core::SystemController::media_prev();
        api_.sendMessage(chat_id, "<b>⏮ Returned to previous track</b>");
    }

}
