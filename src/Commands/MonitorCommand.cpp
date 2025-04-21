#include <Commands/MonitorCommand.h>
#include <Core/Config.h>
#include <Core/Logger.h>

#include <windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <Wbemidl.h>
#include <comdef.h>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace Commands {

    void MonitorCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        std::ostringstream out;

        // 📈 Аптайм
        ULONGLONG uptime_ms = GetTickCount64();
        auto uptime = std::chrono::milliseconds(uptime_ms);
        auto hrs = std::chrono::duration_cast<std::chrono::hours>(uptime).count();
        out << "📈 Uptime: " << hrs / 24 << "d " << hrs % 24 << "h\n";

        // 🧠 CPU Load via WMI
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        double cpu_load = -1.0;
        if (SUCCEEDED(hr)) {
            CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                 RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
                                 nullptr, EOAC_NONE, nullptr);

            IWbemLocator* locator = nullptr;
            hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_IWbemLocator, reinterpret_cast<LPVOID*>(&locator));
            if (SUCCEEDED(hr)) {
                IWbemServices* services = nullptr;
                hr = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services);
                if (SUCCEEDED(hr)) {
                    CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
                                      RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
                                      nullptr, EOAC_NONE);

                    IEnumWbemClassObject* enumerator = nullptr;
                    hr = services->ExecQuery(_bstr_t(L"WQL"),
                        _bstr_t(L"SELECT LoadPercentage FROM Win32_Processor"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        nullptr, &enumerator);

                    if (SUCCEEDED(hr)) {
                        IWbemClassObject* obj = nullptr;
                        ULONG ret = 0;
                        double total_cpu = 0.0;
                        int count = 0;
                        while (enumerator->Next(WBEM_INFINITE, 1, &obj, &ret) == S_OK) {
                            VARIANT val;
                            if (SUCCEEDED(obj->Get(L"LoadPercentage", 0, &val, nullptr, nullptr)) && val.vt == VT_I4) {
                                total_cpu += val.intVal;
                                ++count;
                            }
                            VariantClear(&val);
                            obj->Release();
                        }
                        if (count > 0)
                            cpu_load = total_cpu / count;

                        enumerator->Release();
                    }
                    services->Release();
                }
                locator->Release();
            }
        }

        out << std::fixed << std::setprecision(1);
        if (cpu_load >= 0.0)
            out << "🧠 CPU Load: " << cpu_load << " %\n";
        else
            out << "🧠 CPU Load: [not available]\n";

        // 💾 RAM
        MEMORYSTATUSEX mem{};
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);
        auto total = mem.ullTotalPhys / (1024 * 1024);
        auto used = (mem.ullTotalPhys - mem.ullAvailPhys) / (1024 * 1024);
        out << "💾 RAM: " << used << " / " << total << " MB (" << mem.dwMemoryLoad << "%)\n";

        // 💽 Диски
        out << "💽 Disks:\n";
        for (char letter = 'C'; letter <= 'Z'; ++letter) {
            std::string root = std::string(1, letter) + ":\\";
            UINT type = GetDriveTypeA(root.c_str());
            if (type == DRIVE_FIXED) {
                ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
                if (GetDiskFreeSpaceExA(root.c_str(), &freeBytes, &totalBytes, &totalFreeBytes)) {
                    out << "• " << root << " — "
                        << (totalBytes.QuadPart - freeBytes.QuadPart) / (1024 * 1024 * 1024) << " / "
                        << totalBytes.QuadPart / (1024 * 1024 * 1024) << " GB\n";
                }
            }
        }

        api_.sendMessage(chat_id, out.str());
    }

}