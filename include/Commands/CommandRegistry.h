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
        void register_command(std::unique_ptr<ICommand> cmd);
        void handle(const Telegram::Models::Message& msg);
        std::vector<std::string> list_commands() const;
        static CommandRegistry& instance();
    };

}
