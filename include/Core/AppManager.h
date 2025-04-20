#pragma once

#include <windows.h>
#include <string>

#include "Logger.h"
#include "Utils/Convert.h"

namespace Core {

#define GUARD(expr, message)                                 \
    do {                                                     \
        try {                                                \
            expr;                                            \
        } catch (const std::exception& ex) {                 \
            LOG_ERROR(message, ": ", ex.what());             \
            Core::AppManager::hold_exit();                   \
            return 1;                                        \
        }                                                    \
    } while (false)


    class AppManager {
        static constexpr auto app_name_ = L"PCInteractorBot";


        static std::wstring get_program_path();

        static bool check_admin();
        static bool relaunch_as_admin();

        static long write_to_registry(const std::wstring& key, const std::wstring& path);
        static bool is_path_in_registry(const std::wstring& program_name, std::wstring& current_path);

    public:
        static void init_console();
        static void ensure_admin();
        static void update_startup();

        static void hold_exit();
    };

}
