#include "Commands/CommandRegistry.h"

#include <Commands/CameraCommand.h>
#include <Commands/HelpCommand.h>
#include <Commands/LockCommand.h>
#include <Commands/MediaNextCommand.h>
#include <Commands/MediaPrevCommand.h>
#include <Commands/MediaToggleCommand.h>
#include <Commands/MonitorCommand.h>
#include <Commands/RunCommand.h>
#include <Commands/SayCommand.h>

#include "Commands/SystemInfoCommand.h"
#include <Commands/ScreenshotCommand.h>
#include <Commands/ShutdownPCCommand.h>
#include <Commands/StartCommand.h>
#include <Commands/VolumeCommand.h>
#include <Core/Config.h>
#include <Core/Logger.h>
#include <Telegram/TelegramApi.h>

using namespace Telegram;

namespace Commands {

    CommandRegistry& CommandRegistry::instance() {
        static CommandRegistry inst;
        return inst;
    }

    void CommandRegistry::register_command(std::unique_ptr<ICommand> cmd) {
        cmds_.emplace(cmd->name(), std::move(cmd));
    }

    std::vector<ICommand*> CommandRegistry::list_commands() const {
        std::vector<ICommand*> ptrs;
        ptrs.reserve(cmds_.size());
        for (auto& [_, cmd]: cmds_)
            ptrs.push_back(cmd.get());
        return ptrs;
    }

    void CommandRegistry::handle(const Models::Message& msg) {
        auto text = msg.text;
        const auto pos = text.find(' ');
        const auto cmd = pos == std::string::npos ? text : text.substr(0, pos);
        const auto it = cmds_.find(cmd);
        if (it != cmds_.end()) {
            const auto &command = it->second;
            if (command->admin_only()) {
                auto admins = Core::Config::get<std::vector<int64_t>>("admin_ids");
                if (!std::ranges::contains(admins, msg.from.id)) {
                    const TelegramApi api(Core::Config::get<std::string>("telegram_api_token"));
                    api.sendMessage(msg.chat_id, "❌ Forbidden. This command is for admins only.");
                    return;
                }
            }
            command->execute(msg.chat_id, msg);
            LOG_USER("User (", msg.from.first_name, msg.from.last_name.empty() ? "" : " ", msg.from.last_name,
                ", @", msg.from.username, ") command: ", command->name());
        } else {
            const TelegramApi api(Core::Config::get<std::string>("telegram_api_token"));
            api.sendMessage(msg.chat_id, "Unknown command: " + cmd);
        }
    }

    void CommandRegistry::register_all_commands(TelegramApi& api) {
        register_command(std::make_unique<StartCommand>(api));
        register_command(std::make_unique<HelpCommand>(api));
        register_command(std::make_unique<ShutdownPCCommand>(api));
        register_command(std::make_unique<SystemInfoCommand>(api));
        register_command(std::make_unique<MonitorCommand>(api));
        register_command(std::make_unique<ScreenshotCommand>(api));
        register_command(std::make_unique<CameraCommand>(api));
        register_command(std::make_unique<LockCommand>(api));
        register_command(std::make_unique<MediaToggleCommand>(api));
        register_command(std::make_unique<MediaNextCommand>(api));
        register_command(std::make_unique<MediaPrevCommand>(api));
        register_command(std::make_unique<VolumeCommand>(api));
        register_command(std::make_unique<RunCommand>(api));
        register_command(std::make_unique<SayCommand>(api));
    }

}
