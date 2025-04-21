#include <Commands/MediaNextCommand.h>
#include <Core/Config.h>

#include <Windows.h>

namespace Commands {

    void MediaNextCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
        api_.sendMessage(chat_id, "⏭ Skipped to next track");
    }


}
