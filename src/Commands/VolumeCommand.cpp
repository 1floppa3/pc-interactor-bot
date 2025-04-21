#include <Commands/VolumeCommand.h>
#include <format>
#include <Core/SystemController.h>

namespace Commands {

    void VolumeCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        const std::string& text = msg.text;
        const size_t space = text.find(' ');
        float volume = 0.f;
        if (space != std::string::npos) {
            try {
                float target = std::stof(text.substr(space + 1)) / 100.0f;
                target = std::clamp(target, 0.0f, 1.0f);
                volume = target;
                Core::SystemController::volume(volume);
            } catch (...) {
                api_.sendMessage(chat_id, "<b>⚠️ Invalid volume value</b>\nUsage: <code>/volume 0-100</code>");
            }
        } else {
            Core::SystemController::volume(volume);
        }
        api_.sendMessage(chat_id, std::format("<b>🔊 Volume:</b> <code>{:.0f}%</code>", volume * 100));
    }

}
