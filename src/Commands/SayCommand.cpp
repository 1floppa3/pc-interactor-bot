#include <Commands/SayCommand.h>

#include <Windows.h>
#include <sapi.h>
#include <Utils/Convert.h>

namespace Commands {

    void SayCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        const std::string& text = msg.text;
        const size_t space_pos = text.find(' ');
        if (space_pos == std::string::npos || space_pos + 1 >= text.size()) {
            api_.sendMessage(chat_id, "⚠️ Usage: /say <text>");
            return;
        }

        const auto text_to_speech = std::string(text.begin() + static_cast<long long>(space_pos) + 1, text.end());

        ISpVoice* voice = nullptr;
        if (FAILED(::CoInitialize(nullptr))) {
            api_.sendMessage(chat_id, "❌ CoInitialize failed");
            return;
        }
        if (FAILED(::CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, reinterpret_cast<void **>(&voice)))) {
            api_.sendMessage(chat_id, "❌ Failed to create voice instance");
            CoUninitialize();
            return;
        }

        api_.sendMessage(chat_id, "🗣️ Speaking: " + text_to_speech);
        voice->Speak(Utils::s2ws(text_to_speech).c_str(), SPF_DEFAULT, nullptr);
        voice->Release();
        CoUninitialize();
    }


}
