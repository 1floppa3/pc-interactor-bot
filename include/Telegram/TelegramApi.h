#pragma once

#include <string>
#include <vector>
#include <Commands/ICommand.h>

#include "Models/Update.h"

namespace Telegram {

    class TelegramApi {
        std::string api_url_;
    public:
        explicit TelegramApi(const std::string& token);

        [[nodiscard]] std::vector<Models::Update> getUpdates(int64_t offset, int timeout) const;
        void setMyCommands(const std::vector<Commands::ICommand*>& commands, bool admin_commands = false,
                std::optional<int64_t> scope_chat_id = std::nullopt) const;
        void setChatMenuButton() const;
        void setBotInfo(const std::string& name, const std::string& short_desc,
                const std::string& full_desc) const;

        void sendMessage(int64_t chat_id, const std::string &text, const nlohmann::json &reply_markup = {},
                         const std::string &parse_mode = "") const;
        void editMessageText(int64_t chat_id, int64_t message_id, const std::string &text,
                             const nlohmann::json &reply_markup = {}) const;
        void sendPhoto(int64_t chat_id, const std::string &file_path, const std::string &caption = "") const;
        void sendDocument(int64_t chat_id, const std::string &file_path, const std::string &caption = "") const;
    };

}
