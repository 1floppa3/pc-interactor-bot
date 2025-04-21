#include <Commands/SystemInfoCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void SystemInfoCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        const std::string result = Core::SystemController::format_info();
        api_.sendMessage(chat_id, result);
    }

}