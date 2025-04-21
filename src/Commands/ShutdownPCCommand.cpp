#include <Commands/ShutdownPCCommand.h>
#include <Core/Config.h>

namespace Commands {

    void ShutdownPCCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        api_.sendMessage(chat_id, "⚙️ Shutting down the PC...");
        std::system("shutdown /s /t 0");
    }

}
