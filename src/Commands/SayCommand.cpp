#include <Commands/SayCommand.h>
#include <Core/SystemController.h>
#include <Utils/String.h>

namespace Commands {

    void SayCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        const std::string& text = msg.text;
        const size_t space_pos = text.find(' ');
        if (space_pos == std::string::npos || space_pos + 1 >= text.size()) {
            api_.sendMessage(chat_id, "<b>⚠️ Usage</b>: <code>/say &lt;text&gt;</code>");
            return;
        }

        const auto to_speech = std::string(text.begin() + static_cast<long long>(space_pos) + 1, text.end());
        if (Core::SystemController::say_text(to_speech))
            api_.sendMessage(chat_id, "<b>🗣️ Speaking:</b> <code>" + Utils::html_escape(to_speech) + "</code>");
        else
            api_.sendMessage(chat_id,  "<b>⚠️ Speaking error occurred</b>\n<code>" +
                                        Utils::html_escape(to_speech) + "</code>");

    }


}
