#include <Commands/SleepCommand.h>
#include <Core/SystemController.h>

namespace Commands {

    void SleepCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        api_.sendMessage(chat_id, "😴 Going to sleep....");
        Core::SystemController::sleep();
    }

}
