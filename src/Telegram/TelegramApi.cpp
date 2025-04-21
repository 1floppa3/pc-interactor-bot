#include "Telegram/TelegramApi.h"

#include "cpr/cpr.h"

using namespace Telegram::Models;

namespace Telegram {

    TelegramApi::TelegramApi(const std::string &token): api_url_("https://api.telegram.org/bot" + token + "/") {}

    std::vector<Update> TelegramApi::getUpdates(const int64_t offset, const int timeout) const {
        cpr::Response response = Get(
            cpr::Url{api_url_ + "getUpdates"},
            cpr::Parameters{{"offset", std::to_string(offset)}, {"timeout", std::to_string(timeout)}}
        );
        if (response.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("getUpdates: HTTP " + std::to_string(response.status_code));

        auto response_text = nlohmann::json::parse(response.text);
        if (!response_text["ok"].get<bool>())
            throw std::runtime_error("getUpdates: API error");

        std::vector<Update> out;
        for (auto& item: response_text["result"])
            out.emplace_back(item);
        return out;
    }

    void TelegramApi::sendMessage(int64_t chat_id, const std::string& text) const {
        const nlohmann::json payload = {{"chat_id", chat_id}, {"text", text}};
        const cpr::Response response = Post(
            cpr::Url{api_url_ + "sendMessage"},
            cpr::Body{payload.dump()},
            cpr::Header{{"Content-Type", "application/json"}}
        );
        if (response.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("sendMessage: HTTP " + std::to_string(response.status_code));
    }

    void TelegramApi::sendPhoto(const int64_t chat_id, const std::string& file_path, const std::string& caption) const {
        cpr::Multipart form = {
            {"chat_id", std::to_string(chat_id)},
            {"caption", caption},
            {"photo", cpr::File{file_path}}
        };

        const cpr::Response response = Post(
            cpr::Url{api_url_ + "sendPhoto"},
            form
        );

        if (response.status_code != cpr::status::HTTP_OK) {
            throw std::runtime_error("sendPhoto: HTTP " + std::to_string(response.status_code));
        }
    }

    void TelegramApi::sendDocument(const int64_t chat_id, const std::string& file_path, const std::string& caption) const {
        cpr::Multipart multipart{
            {"chat_id", std::to_string(chat_id)},
            {"caption", caption},
            {"document", cpr::File{file_path}}
        };

        const cpr::Response response = Post(
            cpr::Url{api_url_ + "sendDocument"},
            multipart
        );

        if (response.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("sendDocument: HTTP " + std::to_string(response.status_code));
    }

}