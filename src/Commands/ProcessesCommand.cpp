#include <Commands/ProcessesCommand.h>
#include <Core/SystemController.h>

#include "Utils/String.h"

using Telegram::Models::InlineKeyboardMarkup;
using Telegram::Models::InlineKeyboardButton;

namespace Commands {

    void ProcessesCommand::send_initial(const int64_t chat_id, const std::string& token) const {
        api_.sendMessage(chat_id, build_page(token, 0), build_keyboard(token, 0).serialize());

    }
    void ProcessesCommand::edit_page(const int64_t chat_id, const int64_t message_id,
                                    const std::string &token, const int page) const {
        api_.editMessageText(chat_id, message_id, build_page(token, page), build_keyboard(token, page).serialize());
    }

    std::string ProcessesCommand::build_page(const std::string& token, const int page) const {
        const auto& v = snapshots_.at(token);
        const int total = static_cast<int>(v.size());
        const int pages = (total + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
        const int from = page * ITEMS_PER_PAGE;
        const int to = std::min(from + ITEMS_PER_PAGE, total);

        int name_width = static_cast<int>(std::string("Name").length());
        for (int i = from; i < to; ++i)
            name_width = std::max(name_width, static_cast<int>(v[i].name.length()));
        name_width += 1;

        std::ostringstream oss;
        oss << "<b>📋 Processes (" << (page+1) << "/" << pages << ")</b>\n"
          << "<pre>"
          << std::left << std::setw(name_width) << "Name"
          << std::setw(6)  << "PID"
          << std::setw(8)  << "MemMB"
          << std::setw(6)  << "%Mem" << "\n"
          << std::string(name_width + 6 + 8 + 6, '-') << "\n";

        for (int i = from; i < to; ++i) {
            const auto&[name, pid, mem_mb, mem_pct] = v[i];
            std::ostringstream mem_pct_str;
            mem_pct_str << std::fixed << std::setprecision(2) << mem_pct;
            oss << std::left << std::setw(name_width) << name
              << std::setw(6)  << pid
              << std::setw(8)  << static_cast<int>(mem_mb)
              << std::setw(6)  << mem_pct_str.str()
              << "\n";
        }

        oss << "</pre>";
        return oss.str();
    }
    InlineKeyboardMarkup ProcessesCommand::build_keyboard(const std::string& token, const int page) const {
        InlineKeyboardMarkup kb;
        const auto& v = snapshots_.at(token);
        const int total = static_cast<int>(v.size());
        const int pages = (total + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
        const int from = page * ITEMS_PER_PAGE;
        const int to = std::min(from + ITEMS_PER_PAGE, total);

        std::vector<InlineKeyboardButton> row;
        for (int i = from; i < to; ++i) {
            const auto&[name, pid, mem_mb, mem_pct] = v[i];

            row.emplace_back(name + " (PID:" + std::to_string(pid) + ")",
                std::string(INFO_PREFIX) + token + "|" + std::to_string(i) + "|" + std::to_string(page)
            );
            if (row.size() % 2 == 0) {
                kb.add_row(row);
                row.clear();
            }
        }
        if (!row.empty()) {
            kb.add_row(row);
            row.clear();
        }

        std::vector<InlineKeyboardButton> nav;
        if (page > 0)
            nav.emplace_back("◀️ Prev", std::string(PAGE_PREFIX) + token + "|" + std::to_string(page-1));
        if (page + 1 < pages)
            nav.emplace_back("Next ▶️", std::string(PAGE_PREFIX) + token + "|" + std::to_string(page+1));
        if (!nav.empty())
            kb.add_row(nav);

        return kb;
    }

    void ProcessesCommand::show_info(const int64_t chat_id, const int64_t message_id, const std::string &token,
                                    const int index, const int page) const {
        const auto&[name, pid, mem_mb, mem_pct] = snapshots_.at(token)[index];
        std::ostringstream mem_pct_str;
        mem_pct_str << std::fixed << std::setprecision(2) << mem_pct;
        std::ostringstream o;
        o << "<b>Details:</b>\n"
          << "<b>Name:</b> " << name << "\n"
          << "<b>PID:</b> "  << pid   << "\n"
          << "<b>Mem:</b> "  << static_cast<int>(mem_mb)  << " MB ("<<mem_pct_str.str()<<"%)\n";

        InlineKeyboardMarkup kb;
        kb.add_button(
          InlineKeyboardButton{"🗑 Kill",
              std::string(KILL_PREFIX) + token + "|" + std::to_string(index) + "|" + std::to_string(page)}
        );
        kb.add_button(
          InlineKeyboardButton{"🔙 Back",
              std::string(BACK_PREFIX) + token + "|" + std::to_string(page)}
        );

        api_.editMessageText(chat_id, message_id, o.str(), kb.serialize());
    }
    void ProcessesCommand::kill_process(const int64_t chat_id, const int64_t message_id, const std::string &token,
                                        const int index, const int page) const {
        const auto&[name, pid, mem_mb, mem_pct] = snapshots_.at(token)[index];
        std::ostringstream o;
        o << "<b>Details:</b>\n"
          << "<b>Name:</b> " << name << "\n"
          << "<b>PID:</b> "  << pid   << "\n"
          << "<b>Mem:</b> "  << static_cast<int>(mem_pct)  << " MB ("<<static_cast<int>(mem_pct)<<"%)\n";

        InlineKeyboardMarkup kb;
        kb.add_button(
          InlineKeyboardButton{"🗑 Kill",
              std::string(KILL_PREFIX) + token + "|" + std::to_string(index) + "|" + std::to_string(page)}
        );
        kb.add_button(
          InlineKeyboardButton{"🔙 Back",
              std::string(BACK_PREFIX) + token + "|" + std::to_string(page)}
        );

        api_.editMessageText(chat_id, message_id, o.str(), kb.serialize());
    }

    void ProcessesCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        auto raw = Core::SystemController::query_wmi_all(
            L"SELECT Name, ProcessId FROM Win32_Process",
            {L"Name", L"ProcessId"}
        );

        std::vector<ProcessInfo> list;
        list.reserve(raw.size());
        for (auto const& row : raw) {
            const uint32_t pid = static_cast<uint32_t>(std::stoul(row.at("ProcessId")));
            auto [working_set, mem_percent] = Core::SystemController::query_process_perf(pid);
            if (working_set == 0) continue;

            list.push_back({
                row.at("Name"),
                pid,
                working_set / 1024.0 / 1024.0,
                mem_percent
            });
        }

        const std::string token = std::to_string(snapshot_token_++);
        snapshots_[token] = std::move(list);
        send_initial(chat_id, token);
    }
    void ProcessesCommand::handle_callback(const Telegram::Models::CallbackQuery &cq) {
        const auto& d = cq.data;
        if (d.rfind(PAGE_PREFIX,0)==0) {
            const auto p = Utils::split(d.substr(strlen(PAGE_PREFIX)), '|');
            edit_page(cq.message.chat_id, cq.message.message_id, p[0], std::stoi(p[1]));
        }
        else if (d.rfind(INFO_PREFIX,0)==0) {
            const auto p = Utils::split(d.substr(strlen(INFO_PREFIX)), '|');
            show_info(cq.message.chat_id, cq.message.message_id,
                     p[0], std::stoi(p[1]), std::stoi(p[2]));
        }
        else if (d.rfind(KILL_PREFIX,0)==0) {
           const  auto p = Utils::split(d.substr(strlen(KILL_PREFIX)), '|');
            kill_process(cq.message.chat_id, cq.message.message_id,
                        p[0], std::stoi(p[1]), std::stoi(p[2]));
        }
        else if (d.rfind(BACK_PREFIX,0)==0) {
            const auto p = Utils::split(d.substr(strlen(BACK_PREFIX)), '|');
            edit_page(cq.message.chat_id, cq.message.message_id, p[0], std::stoi(p[1]));
        }
    }

}
