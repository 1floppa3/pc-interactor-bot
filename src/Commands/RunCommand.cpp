#include <Commands/RunCommand.h>
#include <Core/SystemController.h>
#include <fstream>
#include <Utils/String.h>

namespace Commands {

    void RunCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        const std::string& text = msg.text;
        const auto space_pos = text.find(' ');
        if (space_pos == std::string::npos || space_pos + 1 >= text.size()) {
            api_.sendMessage(chat_id, "<b>⚠️ Usage:</b> <code>/run &lt;command&gt;</code>");
            return;
        }
        const std::string cmd = text.substr(space_pos + 1);
        std::string output = Core::SystemController::execute_shell(cmd);

        if (output.empty()) output = "(no output)";

        if (output.size() < 4000) {
            api_.sendMessage(chat_id, "<b>💻 Output:</b>\n<pre>" + Utils::html_escape(output) + "</pre>", {});
        } else {
            std::ofstream file("output.txt");
            file << output;
            file.close();
            api_.sendDocument(chat_id, "output.txt", "💻 Command output");
            std::remove("output.txt");
        }
    }

}
