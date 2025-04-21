#pragma once

#include <Telegram/Models/CallbackQuery.h>

namespace Commands {

    class ICallbackCommand {
    public:
        virtual ~ICallbackCommand() = default;
        [[nodiscard]] virtual bool can_handle_callback(const std::string& callback_data) const = 0;
        virtual void handle_callback(const Telegram::Models::CallbackQuery& cq) = 0;
    };

}