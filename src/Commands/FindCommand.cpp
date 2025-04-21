#include <Commands/FindCommand.h>
#include <Core/SystemController.h>
#include <Telegram/Models/InlineKeyboardMarkup.h>
#include <Utils/String.h>

using Telegram::Models::InlineKeyboardMarkup;
using Telegram::Models::InlineKeyboardButton;

namespace Commands {

    void FindCommand::send_list(const int64_t chat_id, const std::string &token) const {
        const auto& vec = snapshots_.at(token);
        std::ostringstream out;
        out << "🔍 Found " << vec.size() << " process(es):";

        InlineKeyboardMarkup kb;
        for (size_t i = 0; i < vec.size(); ++i) {
            const auto&[name, pid, mem_mb, mem_pct] = vec[i];
            kb.add_row({{
                InlineKeyboardButton{
                  Utils::html_escape(name) + " (PID:" + std::to_string(pid) + ")",
                  std::string(INFO_PREFIX) + token + "|" + std::to_string(i)
                }
            }});
        }

        api_.sendMessage(chat_id, out.str(), kb.serialize());
    }
    void FindCommand::edit_list(const int64_t chat_id, const int64_t message_id, const std::string &token) const {
        const auto& vec = snapshots_.at(token);
        std::ostringstream out;
        out << "🔍 Found " << vec.size() << " process(es):";

        InlineKeyboardMarkup kb;
        for (size_t i = 0; i < vec.size(); ++i) {
            const auto&[name, pid, mem_mb, mem_pct] = vec[i];
            kb.add_row({{
                InlineKeyboardButton{
                  Utils::html_escape(name) + " (PID:" + std::to_string(pid) + ")",
                  std::string(INFO_PREFIX) + token + "|" + std::to_string(i)
                }
            }});
        }

        api_.editMessageText(chat_id, message_id, out.str(), kb.serialize());
    }
    void FindCommand::show_details(const int64_t chat_id, const int64_t message_id, const std::string &token,
                                   const int index) const {
        const auto&[name, pid, mem_mb, mem_pct] = snapshots_.at(token)[index];
        std::ostringstream o;
        o << "<b>Process Details</b>\n"
          << "<b>Name:</b> " << Utils::html_escape(name) << "\n"
          << "<b>PID:</b> "  << pid   << "\n"
          << "<b>Mem:</b> "  << static_cast<int>(mem_mb) << " MB ("
                       << std::fixed << std::setprecision(1)
                       << mem_pct << "%)\n";

        InlineKeyboardMarkup kb;
        kb.add_button({
          "🗑 Kill",
          std::string(KILL_PREFIX) + token + "|" + std::to_string(index)
        });
        kb.add_button({
          "◀️ Back",
          std::string(BACK_PREFIX) + token
        });

        api_.editMessageText(chat_id, message_id, o.str(), kb.serialize());
    }

    void FindCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        const auto text = msg.text;
        const auto pos = text.find(' ');
        if (pos == std::string::npos || pos + 1 >= text.size()) {
            api_.sendMessage(chat_id, "⚠️ Usage: <code>/findproc <substring></code>");
            return;
        }
        const std::string query = Utils::to_lower(text.substr(pos + 1));

        auto raw = Core::SystemController::query_wmi_all(
            L"SELECT Name, ProcessId FROM Win32_Process",
            {L"Name", L"ProcessId"}
        );

        std::vector<FindProcessInfo> found;
        for (auto const& row : raw) {
            const std::string nm = row.at("Name");
            std::string nm_l = Utils::to_lower(nm);
            if (nm_l.find(query) != std::string::npos) {
                const uint32_t pid = static_cast<uint32_t>(std::stoul(row.at("ProcessId")));
                auto perf = Core::SystemController::query_process_perf(pid);
                found.push_back({ nm, pid,
                                  perf.working_set / 1024.0 / 1024.0,
                                  perf.mem_percent });
            }
        }

        if (found.empty()) {
            api_.sendMessage(chat_id, "❌ No processes matching <code>" + Utils::html_escape(query) + "</code>");
            return;
        }

        const std::string token = std::to_string(snapshot_token_++);
        snapshots_[token] = std::move(found);
        send_list(chat_id, token);
    }
    void FindCommand::handle_callback(const Telegram::Models::CallbackQuery &cq) {
        const auto& d = cq.data;

        if (d.rfind(INFO_PREFIX, 0) == 0) {
            // d == "fpinfo|<token>|<idx>"
            const auto parts = Utils::split(d.substr(strlen(INFO_PREFIX)), '|', false);
            if (parts.size() == 2) {
                const std::string& token = parts[0];
                const int idx = std::stoi(parts[1]);
                show_details(cq.message.chat_id, cq.message.message_id, token, idx);
            }
        }
        else if (d.rfind(BACK_PREFIX, 0) == 0) {
            // d == "fprocback|<token>"
            const std::string token = d.substr(strlen(BACK_PREFIX));
            edit_list(cq.message.chat_id, cq.message.message_id, token);
        }
        else if (d.rfind(KILL_PREFIX, 0) == 0) {
            // d == "fpkill|<token>|<idx>"
            const auto parts = Utils::split(d.substr(strlen(KILL_PREFIX)), '|', false);
            if (parts.size() == 2) {
                const std::string& token = parts[0];
                const int idx = std::stoi(parts[1]);

                auto &vec = snapshots_[token];
                vec.erase(vec.begin() + idx);

                const uint32_t pid = snapshots_[token][idx].pid;
                std::ostringstream out;
                out << (Core::SystemController::kill_process(pid) ? "✅ Killed PID " : "❌ Cannot kill PID ")
                    << pid << "\n";
                api_.sendMessage(cq.message.chat_id, out.str());
                edit_list(cq.message.chat_id, cq.message.message_id, token);
            }
        }
    }





}
