#include <Commands/ScreenshotCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void ScreenshotCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        Core::SystemController::take_screenshot("screenshot.bmp");
        api_.sendPhoto(chat_id, "screenshot.bmp", "<b>📸 Screenshot captured</b>");
        std::remove("screenshot.bmp");
    }

}
