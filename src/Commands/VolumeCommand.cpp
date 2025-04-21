#include <Commands/VolumeCommand.h>

#include <Windows.h>
#include <endpointvolume.h>
#include <format>
#include <mmdeviceapi.h>

namespace Commands {

    void VolumeCommand::execute(const int64_t chat_id, const Telegram::Models::Message &msg) {
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ COM initialization failed");
            return;
        }

        IMMDeviceEnumerator* deviceEnumerator = nullptr;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator), reinterpret_cast<void **>(&deviceEnumerator));
        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ Failed to get device enumerator");
            CoUninitialize();
            return;
        }

        IMMDevice* defaultDevice = nullptr;
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ Failed to get default audio endpoint");
            deviceEnumerator->Release();
            CoUninitialize();
            return;
        }

        IAudioEndpointVolume* endpointVolume = nullptr;
        hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, reinterpret_cast<void **>(&endpointVolume));
        if (FAILED(hr)) {
            api_.sendMessage(chat_id, "❌ Failed to access volume control");
            defaultDevice->Release();
            deviceEnumerator->Release();
            CoUninitialize();
            return;
        }

        float volume = 0.0f;
        endpointVolume->GetMasterVolumeLevelScalar(&volume);

        const std::string& text = msg.text;
        size_t space = text.find(' ');
        if (space != std::string::npos) {
            try {
                float target = std::stof(text.substr(space + 1)) / 100.0f;
                target = std::clamp(target, 0.0f, 1.0f);
                endpointVolume->SetMasterVolumeLevelScalar(target, nullptr);
                volume = target;
            } catch (...) {
                api_.sendMessage(chat_id, "⚠️ Invalid volume value. Use: /volume 0-100");
            }
        }

        endpointVolume->Release();
        defaultDevice->Release();
        deviceEnumerator->Release();
        CoUninitialize();

        api_.sendMessage(chat_id, std::format("🔊 Volume: {:.0f}%", volume * 100));
    }

}