#include <Commands/RunCommand.h>
#include <fstream>

namespace Commands {

    void RunCommand::execute(int64_t chat_id, const Telegram::Models::Message &msg) {
        const std::string& text = msg.text;
        const auto space_pos = text.find(' ');
        if (space_pos == std::string::npos || space_pos + 1 >= text.size()) {
            api_.sendMessage(chat_id, "⚠️ Usage: /run <command>");
            return;
        }

        const std::string cmd = text.substr(space_pos + 1);
        const std::string temp_file = "cmd_output.txt";
        const std::string full_cmd = "chcp 65001 > nul & " + cmd + " > " + temp_file + " 2>&1";

        int result = std::system(full_cmd.c_str());
        if (result != 0) {
            api_.sendMessage(chat_id, "⚠️ Command exited with code " + std::to_string(result));
        }

        std::ifstream in(temp_file);
        if (!in.is_open()) {
            api_.sendMessage(chat_id, "❌ Failed to read command output");
            return;
        }

        std::ostringstream contents;
        contents << in.rdbuf();
        std::string output = contents.str();
        in.close();
        std::remove(temp_file.c_str());

        if (output.empty()) output = "(no output)";

        if (output.size() < 4000) {
            api_.sendMessage(chat_id, "💻 Output:\n" + output);
        } else {
            std::ofstream file("output.txt");
            file << output;
            file.close();
            api_.sendDocument(chat_id, "output.txt", "💻 Command output");
            std::remove("output.txt");
        }
    }


}