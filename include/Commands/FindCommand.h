#pragma once

#include <Commands/ICommand.h>
#include <Commands/ICallbackCommand.h>
#include <Telegram/TelegramApi.h>

#include <map>
#include <vector>
#include <string>

namespace Commands {

    class FindCommand final : public ICommand, public ICallbackCommand {
        struct FindProcessInfo {
            std::string name;
            uint32_t pid;
            double mem_mb;
            double mem_pct;
        };
        static constexpr auto INFO_PREFIX = "fpinfo|";
        static constexpr auto BACK_PREFIX = "fprocback|";
        static constexpr auto KILL_PREFIX = "fpkill|";

        Telegram::TelegramApi& api_;
        std::map<std::string,std::vector<FindProcessInfo>> snapshots_;
        int snapshot_token_;

        void send_list(int64_t chat_id, const std::string& token) const;
        void edit_list(int64_t chat_id, int64_t message_id, const std::string& token) const;
        void show_details(int64_t chat_id, int64_t message_id,
                          const std::string& token, int index) const;

    public:
        explicit FindCommand(Telegram::TelegramApi& api): api_(api), snapshot_token_(0) {}

        [[nodiscard]] std::string name() const override { return "/find"; }
        [[nodiscard]] std::string description() const override { return "🔍 Find processes by name"; }
        [[nodiscard]] bool admin_only() const override { return true; }
        [[nodiscard]] bool can_handle_callback(const std::string& d) const override {
            return d.rfind(INFO_PREFIX,0)==0 || d.rfind(BACK_PREFIX,0)==0 || d.rfind(KILL_PREFIX,0)==0;
        }

        void execute(int64_t chat_id, const Telegram::Models::Message& msg) override;
        void handle_callback(const Telegram::Models::CallbackQuery& cq) override;
    };

}
