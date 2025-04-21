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
            throw std::runtime_error("getUpdates: " + response.text);

        auto response_text = nlohmann::json::parse(response.text);
        if (!response_text["ok"].get<bool>())
            throw std::runtime_error("getUpdates: API error");

        std::vector<Update> out;
        for (auto& item: response_text["result"])
                out.emplace_back(item);
        return out;
    }
    void TelegramApi::setMyCommands(const std::vector<Commands::ICommand*>& commands, const bool admin_commands,
                                    std::optional<int64_t> scope_chat_id) const {
        nlohmann::json payload;
        payload["commands"] = nlohmann::json::array();

        for (const auto* cmd : commands) {
            if (admin_commands || !cmd->admin_only()) {
                payload["commands"].push_back({
                    {"command", cmd->name().substr(1)},  // remove '/'
                    {"description", cmd->description()}
                });
            }
        }

        if (scope_chat_id.has_value()) {
            payload["scope"] = {
                {"type", "chat"},
                {"chat_id", scope_chat_id.value()}
            };
        }

        const auto response = Post(
            cpr::Url{api_url_ + "setMyCommands"},
            cpr::Body{payload.dump()},
            cpr::Header{{"Content-Type", "application/json"}}
        );

        if (response.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("setMyCommands: " + response.text);
    }
    void TelegramApi::setChatMenuButton() const {
        const nlohmann::json payload = {
            {"menu_button", {{"type", "commands"}}}
        };

        const auto response = Post(
            cpr::Url{api_url_ + "setChatMenuButton"},
            cpr::Body{payload.dump()},
            cpr::Header{{"Content-Type", "application/json"}}
        );

        if (response.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("setChatMenuButton: " + response.text);
    }
    void TelegramApi::setBotInfo(const std::string& name, const std::string& short_desc,
                                 const std::string& full_desc) const {
        const auto set_name = Post(
            cpr::Url{api_url_ + "setMyName"},
            cpr::Payload{{"name", name}},
            cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"}}
        );
        if (set_name.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("setMyName failed: " + set_name.text);

        const auto set_short_desc = Post(
            cpr::Url{api_url_ + "setMyShortDescription"},
            cpr::Payload{{"short_description", short_desc}},
            cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"}}
        );
        if (set_short_desc.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("setMyShortDescription failed: " + set_short_desc.text);

        const auto set_full_desc = Post(
            cpr::Url{api_url_ + "setMyDescription"},
            cpr::Payload{{"description", full_desc}},
            cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"}}
        );
        if (set_full_desc.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("setMyDescription failed: " + set_full_desc.text);
    }

    void TelegramApi::sendMessage(int64_t chat_id, const std::string& text, const nlohmann::json& reply_markup,
                                  const std::string &parse_mode) const {
        nlohmann::json payload = {
            {"chat_id", chat_id},
            {"text", text},
            {"parse_mode", "HTML"}
        };
        if (!reply_markup.is_null() && !reply_markup.empty())
            payload["reply_markup"] = reply_markup;
        if (!parse_mode.empty())
            payload["parse_mode"] = parse_mode;

        const cpr::Response response = Post(
            cpr::Url{api_url_ + "sendMessage"},
            cpr::Body{payload.dump()},
            cpr::Header{{"Content-Type", "application/json"}}
        );
        if (response.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("sendMessage: " + response.text);
    }
    void TelegramApi::editMessageText(int64_t chat_id, int64_t message_id, const std::string& text,
                                      const nlohmann::json& reply_markup) const {
        nlohmann::json payload = {
            {"chat_id", chat_id},
            {"message_id", message_id},
            {"text", text},
            {"parse_mode", "HTML"}
        };
        if (!reply_markup.is_null() && !reply_markup.empty())
            payload["reply_markup"] = reply_markup;

        const cpr::Response response = Post(
            cpr::Url{api_url_ + "editMessageText"},
            cpr::Body{payload.dump()},
            cpr::Header{{"Content-Type", "application/json"}}
        );

        if (response.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("editMessageText failed: " + response.text);
    }
    void TelegramApi::sendPhoto(const int64_t chat_id, const std::string& file_path, const std::string& caption) const {
        cpr::Multipart form = {
            {"chat_id", std::to_string(chat_id)},
            {"caption", caption},
            {"parse_mode", "HTML"},
            {"photo", cpr::File{file_path}},
        };

        const cpr::Response response = Post(
            cpr::Url{api_url_ + "sendPhoto"},
            form
        );

        if (response.status_code != cpr::status::HTTP_OK) {
            throw std::runtime_error("sendPhoto: " + response.text);
        }
    }
    void TelegramApi::sendDocument(const int64_t chat_id, const std::string& file_path,
                                   const std::string& caption) const {
        cpr::Multipart multipart{
            {"chat_id", std::to_string(chat_id)},
            {"caption", caption},
            {"document", cpr::File{file_path}},
            {"parse_mode", "HTML"}
        };

        const cpr::Response response = Post(
            cpr::Url{api_url_ + "sendDocument"},
            multipart
        );

        if (response.status_code != cpr::status::HTTP_OK)
            throw std::runtime_error("sendDocument: " + response.text);
    }

}