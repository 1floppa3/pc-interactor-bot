#include <Commands/MediaToggleCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void MediaToggleCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        Core::SystemController::media_toggle();
        api_.sendMessage(chat_id, "<b>⏯ Media toggled</b>");
    }

}
