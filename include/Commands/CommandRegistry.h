#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "ICommand.h"

namespace Commands {

    class CommandRegistry {
        std::unordered_map<std::string, std::unique_ptr<ICommand>> cmds_;
    public:
        static CommandRegistry& instance();
        void register_command(std::unique_ptr<ICommand> cmd);
        void handle(const Telegram::Models::Message& msg);
        std::vector<ICommand*> list_commands() const;
    };

}
