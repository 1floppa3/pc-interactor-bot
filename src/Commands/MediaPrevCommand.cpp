#include <Commands/MediaPrevCommand.h>

#include <Core/Config.h>
#include <Windows.h>

namespace Commands {

    void MediaPrevCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
        api_.sendMessage(chat_id, "⏮ Returned to previous track");
    }


}