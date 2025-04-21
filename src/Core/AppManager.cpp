#include "Core/AppManager.h"

#include <iostream>
#include <Core/Logger.h>

#include "windows.h"

namespace Core {

    void AppManager::init_console() {
        const auto h = GetStdHandle(STD_ERROR_HANDLE);
        DWORD mode;
        GetConsoleMode(h, &mode);
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    bool AppManager::check_admin() {
        BOOL is_admin = FALSE;
        PSID admin_group = nullptr;
        SID_IDENTIFIER_AUTHORITY nt_auth{SECURITY_NT_AUTHORITY};
        if (AllocateAndInitializeSid(&nt_auth, 2,
                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0, &admin_group)) {
            CheckTokenMembership(nullptr, admin_group, &is_admin);
            FreeSid(admin_group);
            }
        return is_admin == TRUE;
    }
    bool AppManager::relaunch_as_admin() {
        const std::wstring lp_filename = get_program_path();
        HINSTANCE result = ShellExecuteW(nullptr, L"runas", lp_filename.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return reinterpret_cast<intptr_t>(result) > 32;
    }
    void AppManager::ensure_admin() {
        if (!check_admin()) {
            LOG_INFO("No admin rights - requesting an elevation...");
            if (!relaunch_as_admin())
                throw std::runtime_error("Failed to get administrator rights.");
            exit(0);
        }
    }

    void AppManager::update_startup() {
        const std::wstring task_name = L"PCInteractorBot";
        const std::wstring exe_path = get_program_path();

        HKEY hkey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS) {
            RegDeleteValueW(hkey, task_name.c_str());
            RegCloseKey(hkey);
        }

        const std::wstring check_cmd = L"schtasks /Query /TN \"" + task_name + L"\" > nul 2>&1";
        if (_wsystem(check_cmd.c_str()) == 0) {
            return;
        }

        std::wstring create_cmd = L"schtasks /Create /F "
            L"/RL HIGHEST "                         // HIGHEST = admin
            L"/SC ONLOGON "                         // on system logon
            L"/TN \"" + task_name + L"\" "          // task name
            L"/TR \"" + exe_path + L"\"";           // path

        const int result = _wsystem(create_cmd.c_str());
        if (result != 0)
            throw std::runtime_error("Failed to create scheduled task for autostart");

        LOG_INFO("Autostart task registered via Task Scheduler.");
    }

    std::wstring AppManager::get_program_path() {
        wchar_t lp_filename[MAX_PATH];
        if (GetModuleFileNameW(nullptr, lp_filename, MAX_PATH) == 0)
            throw std::runtime_error("Failed to get program path.");
        return lp_filename;
    }
    std::wstring AppManager::get_program_dir() {
        const std::wstring full_path = get_program_path();
        return full_path.substr(0, full_path.find_last_of(L"\\/"));
    }

    void AppManager::hold_exit() {
        LOG_RAW("Press Enter to exit...");
        std::cin.get();
    }

}
