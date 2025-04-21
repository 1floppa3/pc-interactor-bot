#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace Core {

    class SystemController {
    public:
        struct ProcPerf {
            uint32_t working_set;
            double mem_percent;
        };
        static std::vector<std::map<std::string, std::string>> query_wmi_all(const std::wstring& wql, const std::vector<std::wstring>& fields);
        static bool capture_camera(const std::string& path);
        static bool lock();
        static void media_next();
        static void media_prev();
        static void media_toggle();
        static std::string format_usage();
        static std::string execute_shell(const std::string& cmd);
        static bool say_text(const std::string& text);
        static void take_screenshot(const std::string& path);
        static void shutdown();
        static std::string format_info();
        static std::string get_username();
        static std::string get_computer_name();
        static std::string format_capacity_gb(const std::vector<uint64_t>& capacities);
        static bool volume(float &volume);
        static void hibernate();
        static bool sleep();
        static bool kill_process(uint32_t pid);
        static uint64_t get_total_physical_memory();
        static ProcPerf query_process_perf(uint32_t pid);
    };

}