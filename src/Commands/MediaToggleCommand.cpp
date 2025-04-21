#include <Commands/MediaToggleCommand.h>
#include <Core/Config.h>

#include <Windows.h>

namespace Commands {

    void MediaToggleCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
        api_.sendMessage(chat_id, "⏯ Media toggled");
    }


}
