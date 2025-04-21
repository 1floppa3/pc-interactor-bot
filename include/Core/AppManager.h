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
        static bool check_admin();
        static bool relaunch_as_admin();

    public:
        static void init_console();
        static void ensure_admin();
        static void update_startup();

        static std::wstring get_program_path();
        static std::wstring get_program_dir();

        static void hold_exit();
    };

}
