#pragma once

#include <cstdint>
#include <string>

#include "../Telegram/Models/Message.h"

namespace Commands {

    class ICommand  {
    public:
        virtual ~ICommand() = default;
        [[nodiscard]] virtual std::string name() const = 0;
        [[nodiscard]] virtual std::string description() const = 0;
        [[nodiscard]] virtual bool admin_only() const = 0;
        virtual void execute(int64_t chat_id, const Telegram::Models::Message& msg) = 0;
    };

}
