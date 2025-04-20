#include <Commands/SystemInfoCommand.h>
#include <Core/Config.h>
#include <Core/Logger.h>

#include <Windows.h>
#include <tchar.h>
#include <Lmcons.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <psapi.h>

#include <sstream>
#include <format>
#include <chrono>
#include <map>

#include "Utils/Convert.h"

namespace Commands {

    std::string get_username() {
        TCHAR name[UNLEN + 1];
        DWORD size = UNLEN + 1;
        GetUserName(name, &size);
        return Utils::tchar2s(name);
    }
    std::string get_computer_name() {
        TCHAR name[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerName(name, &size);
        return Utils::tchar2s(name);
    }
    std::string format_capacity_gb(const std::vector<uint64_t>& capacities) {
        uint64_t total_bytes = 0;
        for (const auto cap : capacities)
            total_bytes += cap;

        std::ostringstream oss;
        oss << total_bytes / (1024 * 1024 * 1024) << " GB (" << capacities.size() << "x"
            << capacities.front() / (1024 * 1024 * 1024) << " GB)";
        return oss.str();
    }
    std::vector<std::map<std::string, std::string>> query_wmi_all(const std::wstring& wql, const std::vector<std::wstring>& fields) {
        std::vector<std::map<std::string, std::string>> records;

        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr)) return records;

        CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
                             RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);

        IWbemLocator* locator = nullptr;
        hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                              IID_IWbemLocator, reinterpret_cast<LPVOID*>(&locator));
        if (FAILED(hr)) {
            CoUninitialize();
            return records;
        }

        IWbemServices* services = nullptr;
        locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services);
        CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
                          RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
                          nullptr, EOAC_NONE);

        IEnumWbemClassObject* enumerator = nullptr;
        services->ExecQuery(_bstr_t(L"WQL"), _bstr_t(wql.c_str()),
                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                            nullptr, &enumerator);

        IWbemClassObject* obj = nullptr;
        ULONG ret = 0;
        while (enumerator->Next(WBEM_INFINITE, 1, &obj, &ret) == S_OK) {
            std::map<std::string, std::string> row;
            for (const auto& field : fields) {
                VARIANT val;
                if (SUCCEEDED(obj->Get(field.c_str(), 0, &val, nullptr, nullptr))) {
                    if (val.vt == VT_BSTR)
                        row[Utils::ws2s(field)] = Utils::ws2s(std::wstring(_bstr_t(val.bstrVal)));
                    else if (val.vt == VT_UI4 || val.vt == VT_I4)
                        row[Utils::ws2s(field)] = std::to_string(val.uintVal);
                    VariantClear(&val);
                }
            }
            records.push_back(std::move(row));
            obj->Release();
        }

        enumerator->Release();
        services->Release();
        locator->Release();
        CoUninitialize();

        return records;
    }

    void SystemInfoCommand::execute(const int64_t chat_id, const Telegram::Models::Message& msg) {
        auto admins = Core::Config::get<std::vector<int64_t>>("admin_ids");
        const bool is_admin = std::ranges::find(admins, msg.from.id) != admins.end();
        if (!is_admin) {
            api_.sendMessage(chat_id, "❌ Forbidden. This command is for admins only.");
            return;
        }

        std::ostringstream out;
        out << "🖥️ Hostname: " << get_computer_name() << "\n";
        out << "👤 User: " << get_username() << "\n";

        auto cpu = query_wmi_all(L"SELECT Name, NumberOfCores, NumberOfLogicalProcessors FROM Win32_Processor",
                                 {L"Name", L"NumberOfCores", L"NumberOfLogicalProcessors"});
        if (!cpu.empty()) {
            out << "🧠 CPU: " << cpu[0]["Name"] << " | " << cpu[0]["NumberOfCores"] << " cores / " << cpu[0]["NumberOfLogicalProcessors"] << " threads\n";
        }

        auto gpus = query_wmi_all(L"SELECT Name FROM Win32_VideoController", {L"Name"});
        out << "🎮 GPU(s):\n";
        for (const auto& gpu : gpus) {
            out << "• " << gpu.at("Name") << "\n";
        }

        auto ram_modules = query_wmi_all(L"SELECT Capacity FROM Win32_PhysicalMemory", {L"Capacity"});
        std::vector<uint64_t> capacities;
        for (const auto& ram : ram_modules)
            capacities.push_back(std::stoull(ram.at("Capacity")));
        if (!capacities.empty())
            out << "💾 RAM: " << format_capacity_gb(capacities) << "\n";

        auto board = query_wmi_all(L"SELECT Manufacturer, Product FROM Win32_BaseBoard",
                                   {L"Manufacturer", L"Product"});
        if (!board.empty())
            out << "🧩 Motherboard: " << board[0]["Manufacturer"] << " (" << board[0]["Product"] << ")\n";

        auto bios = query_wmi_all(L"SELECT SMBIOSBIOSVersion, ReleaseDate FROM Win32_BIOS",
                                  {L"SMBIOSBIOSVersion", L"ReleaseDate"});
        if (!bios.empty()) {
            std::string date_raw = bios[0]["ReleaseDate"];
            std::string date = date_raw.substr(6, 2) + "." + date_raw.substr(4, 2) + "." + date_raw.substr(0, 4);
            out << "📦 BIOS: " << bios[0]["SMBIOSBIOSVersion"] << " (" << date << ")\n";
        }

        auto drives = query_wmi_all(L"SELECT DeviceID, FileSystem, Size FROM Win32_LogicalDisk WHERE DriveType = 3",
                                    {L"DeviceID", L"FileSystem", L"Size"});
        out << "💽 Drives:\n";
        for (const auto& d : drives) {
            out << "• " << d.at("DeviceID") << " [" << d.at("FileSystem") << "] — "
                << std::stoull(d.at("Size")) / (1024 * 1024 * 1024) << " GB\n";
        }

        std::string result = out.str();
        if (result.size() > 4000)
            result = result.substr(0, 4000) + "\n\n[...output truncated]";

        api_.sendMessage(chat_id, result);
    }

}