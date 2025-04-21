#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <Telegram/TelegramApi.h>

#include "ICommand.h"

namespace Commands {

    class CommandDispatcher {
        std::unordered_map<std::string, std::unique_ptr<ICommand>> cmds_;
    public:
        static CommandDispatcher& instance();

        std::vector<ICommand*> list_commands() const;
        void register_command(std::unique_ptr<ICommand> cmd);
        void register_all_commands(Telegram::TelegramApi& api);

        void handle(const Telegram::TelegramApi& api, const Telegram::Models::Message& msg);
        void handle_callback(const Telegram::Models::CallbackQuery &cq);
    };

}
