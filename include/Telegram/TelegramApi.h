#pragma once

#include <string>
#include <vector>

#include "Models/Update.h"

namespace Telegram {

    class TelegramApi {
        std::string api_url_;
    public:
        explicit TelegramApi(const std::string& token);

        [[nodiscard]] std::vector<Models::Update> getUpdates(int64_t offset, int timeout) const;

        void sendMessage(int64_t chat_id, const std::string& text) const;
    };

}
