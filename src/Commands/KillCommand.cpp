#include <Commands/KillCommand.h>
#include <Core/SystemController.h>
#include <Utils/String.h>

namespace Commands {

    void KillCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        const auto parts = Utils::split(msg.text, ' ', false);
        if (parts.size() != 2) {
            api_.sendMessage(chat_id, "⚠️ Usage: <code>/kill &lt;PID&gt;</code>");
            return;
        }

        uint32_t pid = 0;
        try {
            pid = static_cast<uint32_t>(std::stoul(parts[1]));
        } catch (...) {
            api_.sendMessage(chat_id, "❌ Invalid PID: <code>" + Utils::html_escape(parts[1]) + "</code>");
            return;
        }

        if (Core::SystemController::kill_process(pid)) {
            api_.sendMessage(chat_id, "✅ Process <code>" + std::to_string(pid) + "</code> has been terminated.");
        } else {
            api_.sendMessage(chat_id, "❌ Cannot kill process <code>" + std::to_string(pid) + "</code>.");
        }
    }

}
