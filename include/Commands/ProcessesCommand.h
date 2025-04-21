#pragma once

#include <Telegram/TelegramApi.h>

#include "ICommand.h"
#include "ICallbackCommand.h"
#include <Telegram/Models/InlineKeyboardMarkup.h>

namespace Commands {


    class ProcessesCommand final : public ICommand, public ICallbackCommand {
        struct ProcessInfo {
            std::string name;
            uint32_t pid;
            double mem_mb;
            double mem_pct;
        };
        static constexpr int ITEMS_PER_PAGE = 10;
        static constexpr auto PAGE_PREFIX = "/processes|";
        static constexpr auto INFO_PREFIX = "/procinfo|";
        static constexpr auto KILL_PREFIX = "/killproc|";
        static constexpr auto BACK_PREFIX = "/procback|";

        Telegram::TelegramApi& api_;
        std::map<std::string,std::vector<ProcessInfo>> snapshots_;
        int snapshot_token_;

        void send_initial(int64_t chat_id, const std::string& token) const;
        void edit_page(int64_t chat_id, int64_t message_id, const std::string& token, int page) const;

        [[nodiscard]] std::string build_page(const std::string& token, int page) const;
        [[nodiscard]] Telegram::Models::InlineKeyboardMarkup build_keyboard(const std::string& token, int page) const;

        void show_info(int64_t chat_id, int64_t message_id, const std::string& token, int index, int page) const;
        void kill_process(int64_t chat_id, int64_t message_id, const std::string& token, int index, int page) const;

    public:
        explicit ProcessesCommand(Telegram::TelegramApi& api) : api_(api), snapshot_token_(0) {}

        [[nodiscard]] std::string name() const override { return "/processes"; }
        [[nodiscard]] std::string description() const override { return "📋 List processes with pagination"; }
        [[nodiscard]] bool admin_only() const override { return true; }
        [[nodiscard]] bool can_handle_callback(const std::string &callback_data) const override {
            return callback_data.rfind(PAGE_PREFIX,0)==0 || callback_data.rfind(INFO_PREFIX,0)==0
            || callback_data.rfind(KILL_PREFIX,0)==0 || callback_data.rfind(BACK_PREFIX,0)==0;
        }

        void execute(int64_t chat_id, const Telegram::Models::Message &msg) override;
        void handle_callback(const Telegram::Models::CallbackQuery &cq) override;
    };

}