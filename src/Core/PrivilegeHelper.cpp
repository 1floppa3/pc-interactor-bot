#include "Core/PrivilegeHelper.h"

#include <Core/Logger.h>

#include "windows.h"

namespace Core {

    bool PrivilegeHelper::check_admin() {
        BOOL is_admin = FALSE;
        PSID admin_group = nullptr;
        SID_IDENTIFIER_AUTHORITY nt_auth{SECURITY_NT_AUTHORITY};
        if (AllocateAndInitializeSid(&nt_auth, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0,0,0,0,0,0, &admin_group)) {
            CheckTokenMembership(nullptr, admin_group, &is_admin);
            FreeSid(admin_group);
            }
        return is_admin == TRUE;
    }

    bool PrivilegeHelper::relaunch_as_admin() {
        wchar_t lp_filename[MAX_PATH];
        if (!GetModuleFileNameW(nullptr, lp_filename, MAX_PATH))
            return false;
        HINSTANCE result = ShellExecuteW(
            nullptr, L"runas", lp_filename, nullptr, nullptr, SW_SHOWNORMAL
        );
        return reinterpret_cast<intptr_t>(result) > 32;
    }

    void PrivilegeHelper::ensure_admin() {
        if (!check_admin()) {
            LOG_INFO("No admin rights - requesting an elevation...");
            if (!relaunch_as_admin())
                throw std::runtime_error("Failed to get administrator rights.");
            exit(0);
        }
    }

}
