#include <Commands/CameraCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void CameraCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        if (!Core::SystemController::capture_camera("camera.bmp")) {
            api_.sendMessage(chat_id, "<b>❌ Camera capture failed</b>");
            return;
        }

        api_.sendPhoto(chat_id, "camera.bmp", "<b>📷 Camera snapshot</b>");
        std::remove("camera.bmp");
    }

}
