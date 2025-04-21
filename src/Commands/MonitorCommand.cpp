#include <Commands/MonitorCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void MonitorCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        api_.sendMessage(chat_id, Core::SystemController::format_usage());
    }

}
