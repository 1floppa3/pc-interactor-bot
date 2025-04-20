#include "Core/AppManager.h"

#include <iostream>
#include <Core/Logger.h>

#include "windows.h"

namespace Core {

    std::wstring AppManager::get_program_path() {
        wchar_t lp_filename[MAX_PATH];
        if (GetModuleFileNameW(nullptr, lp_filename, MAX_PATH) == 0)
            throw std::runtime_error("Failed to get program path.");
        return lp_filename;
    }

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

    long AppManager::write_to_registry(const std::wstring &key, const std::wstring &path) {
        HKEY hkey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hkey)!= ERROR_SUCCESS)
            throw std::runtime_error("Unable to open registry for write.");

        const long result = RegSetValueExW(hkey, key.c_str(), 0, REG_SZ,
                                           reinterpret_cast<const BYTE*>(path.c_str()),
                                           (path.length() + 1) * sizeof(wchar_t));
        RegCloseKey(hkey);
        return result;
    }
    bool AppManager::is_path_in_registry(const std::wstring &program_name, std::wstring &current_path) {
        HKEY hkey;
        long result = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hkey);
        if (result != ERROR_SUCCESS)
            throw std::runtime_error("Error opening registry key for auto-start.");

        DWORD buffer_size = MAX_PATH;
        wchar_t reg_path[MAX_PATH] = {};
        result = RegQueryValueExW(hkey, program_name.c_str(), nullptr, nullptr, reinterpret_cast<BYTE*>(reg_path), &buffer_size);
        RegCloseKey(hkey);

        if (result != ERROR_SUCCESS)
            return false;

        current_path = std::wstring(reg_path);
        return true;
    }
    void AppManager::update_startup() {
        const std::wstring current_program_path = get_program_path();
        std::wstring stored_path;
        if (is_path_in_registry(app_name_, stored_path)) {
            if (stored_path != current_program_path) {
                if (write_to_registry(app_name_, current_program_path) != ERROR_SUCCESS)
                    throw std::runtime_error("Error writing to registry for auto-start.");
                LOG_INFO("Updated registry auto-start path for '", Utils::ws2s(app_name_),
                    "' from: '", Utils::ws2s(stored_path), "' to: '", Utils::ws2s(current_program_path), "'");
            } else {
                LOG_INFO("Registry auto-start path is already set.");
            }
        } else {
            if (write_to_registry(app_name_, current_program_path) != ERROR_SUCCESS)
                throw std::runtime_error("Error writing to registry for auto-start.");
            LOG_INFO("Added program path to auto-start registry: ", Utils::ws2s(current_program_path));
        }
    }

    void AppManager::hold_exit() {
        LOG_RAW("Press Enter to exit...");
        std::cin.get();
    }

}
