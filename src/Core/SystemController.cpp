#include <Core/SystemController.h>

#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <fstream>
#include <psapi.h>
#include <iphlpapi.h>
#include <Wbemidl.h>
#include <comdef.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <sapi.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <powrprof.h>

#include <Utils/Convert.h>

namespace Core {

    std::vector<std::map<std::string, std::string>> SystemController::query_wmi_all(const std::wstring& wql,
        const std::vector<std::wstring>& fields) {
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

    bool SystemController::capture_camera(const std::string& path) {
        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr))
            return false;

        IMFAttributes* attr = nullptr;
        hr = MFCreateAttributes(&attr, 1);
        if (FAILED(hr)) {
            MFShutdown();
            return false;
        }
        attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

        IMFActivate** devices = nullptr;
        UINT32 count = 0;
        hr = MFEnumDeviceSources(attr, &devices, &count);
        if (FAILED(hr) || count == 0) {
            if (devices) CoTaskMemFree(devices);
            attr->Release();
            MFShutdown();
            return false;
        }

        // Activate first device
        IMFMediaSource* media_source = nullptr;
        hr = devices[0]->ActivateObject(
            __uuidof(IMFMediaSource),
            reinterpret_cast<void**>(&media_source)
        );
        for (UINT32 i = 0; i < count; ++i) devices[i]->Release();
        CoTaskMemFree(devices);
        attr->Release();

        if (FAILED(hr)) {
            MFShutdown();
            return false;
        }

        IMFSourceReader* reader = nullptr;
        IMFAttributes* reader_attr = nullptr;
        hr = MFCreateAttributes(&reader_attr, 1);
        if (SUCCEEDED(hr)) {
            reader_attr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
            hr = MFCreateSourceReaderFromMediaSource(media_source, reader_attr, &reader);
            reader_attr->Release();
        }
        media_source->Release();
        if (FAILED(hr)) {
            MFShutdown();
            return false;
        }
        // Select video stream
        reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);
        reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), TRUE);

        // Set output format to RGB32
        IMFMediaType* out_type = nullptr;
        hr = MFCreateMediaType(&out_type);
        if (SUCCEEDED(hr)) {
            out_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            out_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
            hr = reader->SetCurrentMediaType(
                static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
                nullptr, out_type
            );
            out_type->Release();
        }
        if (FAILED(hr)) {
            reader->Release();
            MFShutdown();
            return false;
        }

        // Determine frame size and stride
        IMFMediaType* actual_type = nullptr;
        UINT32 width = 640, height = 480;
        UINT32 stride = 0;
        if (SUCCEEDED(reader->GetCurrentMediaType(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM, &actual_type))) {
            MFGetAttributeSize(actual_type, MF_MT_FRAME_SIZE, &width, &height);
            actual_type->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride);
            actual_type->Release();
        }
        if (stride == 0) stride = width * 4; // fallback

        // Read one sample
        IMFSample* sample = nullptr;
        DWORD stream_index = 0, flags = 0;
        LONGLONG timestamp = 0;
        constexpr int max_attempts = 10;
        int attempts = 0;
        HRESULT read_hr = S_OK;
        while (attempts++ < max_attempts) {
            read_hr = reader->ReadSample(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0, &stream_index, &flags, &timestamp, &sample
            );
            if (FAILED(read_hr) || sample) break;
        }
        if (FAILED(read_hr) || !sample) {
            reader->Release();
            MFShutdown();
            return false;
        }

        // Convert sample to contiguous buffer
        IMFMediaBuffer* buffer = nullptr;
        sample->ConvertToContiguousBuffer(&buffer);
        sample->Release();

        // Lock buffer and get data pointer
        BYTE* data = nullptr;
        DWORD max_len = 0, cur_len = 0;
        buffer->Lock(&data, &max_len, &cur_len);

        DWORD image_size = static_cast<DWORD>(abs(static_cast<LONG>(height))) * stride;
        BITMAPFILEHEADER bmf_header = {};
        BITMAPINFOHEADER bi = {};

        bmf_header.bfType      = 0x4D42; // 'BM'
        bmf_header.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmf_header.bfSize      = bmf_header.bfOffBits + image_size;

        bi.biSize          = sizeof(BITMAPINFOHEADER);
        bi.biWidth         = static_cast<LONG>(width);
        bi.biHeight        = -static_cast<LONG>(height); // top-down bitmap
        bi.biPlanes        = 1;
        bi.biBitCount      = 32;
        bi.biCompression   = BI_RGB;
        bi.biSizeImage     = image_size;

        // Write BMP to file
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<char*>(&bmf_header), sizeof(bmf_header));
        file.write(reinterpret_cast<char*>(&bi),         sizeof(bi));
        file.write(reinterpret_cast<char*>(data),        image_size);
        file.close();

        // Cleanup
        buffer->Unlock();
        buffer->Release();
        reader->Release();
        MFShutdown();
        return true;
    }
    bool SystemController::lock() {
        return LockWorkStation();
    }
    void SystemController::media_next() {
        keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
    }
    void SystemController::media_prev() {
        keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
    }
    void SystemController::media_toggle() {
        keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
    }
    std::string SystemController::format_usage() {
        std::ostringstream out;

        // 📈 Аптайм
        ULONGLONG uptime_ms = GetTickCount64();
        auto uptime = std::chrono::milliseconds(uptime_ms);
        auto hrs = std::chrono::duration_cast<std::chrono::hours>(uptime).count();
        out << "<b>📈 Uptime:</b> <code>"
            << (hrs / 24) << "d " << (hrs % 24) << "h"
            << "</code>\n";

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
        if (cpu_load >= 0.0) {
            out << "<b>🧠 CPU Load:</b> <code>"
                << cpu_load << " %"
                << "</code>\n";
        } else {
            out << "<b>🧠 CPU Load:</b> <i>[not available]</i>\n";
        }

        // 💾 RAM
        MEMORYSTATUSEX mem{};
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);
        auto total = mem.ullTotalPhys / (1024 * 1024);
        auto used = (mem.ullTotalPhys - mem.ullAvailPhys) / (1024 * 1024);
        out << "<b>💾 RAM:</b> <code>"
            << used << " / " << total << " MB"
            << " (" << mem.dwMemoryLoad << "%)"
            << "</code>\n";

        // 💽 Диски
        out << "<b>💽 Disks:</b>\n";
        for (char letter = 'C'; letter <= 'Z'; ++letter) {
            std::string root = std::string(1, letter) + ":\\";
            UINT type = GetDriveTypeA(root.c_str());
            if (type == DRIVE_FIXED) {
                ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
                if (GetDiskFreeSpaceExA(root.c_str(), &freeBytes, &totalBytes, &totalFreeBytes)) {
                    auto usedGB  = (totalBytes.QuadPart - totalFreeBytes.QuadPart) / (1024ull*1024*1024);
                    auto totalGB = totalBytes.QuadPart / (1024ull*1024*1024);
                    out << "• <code>"
                        << root << " — "
                        << usedGB << " / " << totalGB << " GB"
                        << "</code>\n";
                }
            }
        }
        return out.str();
    }
    std::string SystemController::execute_shell(const std::string &cmd) {
        const std::string temp_file = "cmd_output.txt";
        const std::string full_cmd = "chcp 65001 > nul & " + cmd + " > " + temp_file + " 2>&1";

        std::string output;
        int result = std::system(full_cmd.c_str());
        if (result != 0) {
            output = "⚠️ Command exited with code " + std::to_string(result);
            return output;
        }

        std::ifstream in(temp_file);
        if (!in.is_open()) {
            output = "❌ Failed to read command output";
            return output;
        }

        std::ostringstream contents;
        contents << in.rdbuf();
        output = contents.str();
        in.close();
        std::remove(temp_file.c_str());
        return output;
    }
    bool SystemController::say_text(const std::string &text) {
        ISpVoice* voice = nullptr;
        if (FAILED(::CoInitialize(nullptr)))
            return false;
        if (FAILED(::CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, reinterpret_cast<void **>(&voice)))) {
            CoUninitialize();
            return false;
        }
        voice->Speak(Utils::s2ws(text).c_str(), SPF_DEFAULT, nullptr);
        voice->Release();
        CoUninitialize();
        return true;
    }
    void SystemController::take_screenshot(const std::string &path) {
        HDC hScreenDC = GetDC(nullptr);
        HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        auto hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));

        BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
        hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));

        BITMAP bmp;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        BITMAPFILEHEADER bmfHeader;
        BITMAPINFOHEADER bi;

        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmp.bmWidth;
        bi.biHeight = -bmp.bmHeight; // top-down
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        DWORD dwBmpSize = (bmp.bmWidth * bi.biBitCount + 31) / 32 * 4 * bmp.bmHeight;
        std::vector<char> bmpData(dwBmpSize);

        GetDIBits(hMemoryDC, hBitmap, 0, static_cast<UINT>(bmp.bmHeight), bmpData.data(), reinterpret_cast<BITMAPINFO *>(&bi), DIB_RGB_COLORS);

        std::ofstream file(path, std::ios::out | std::ios::binary);

        bmfHeader.bfType = 0x4D42;
        bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmfHeader.bfSize = bmfHeader.bfOffBits + dwBmpSize;
        bmfHeader.bfReserved1 = 0;
        bmfHeader.bfReserved2 = 0;

        file.write(reinterpret_cast<char *>(&bmfHeader), sizeof(BITMAPFILEHEADER));
        file.write(reinterpret_cast<char *>(&bi), sizeof(BITMAPINFOHEADER));
        file.write(bmpData.data(), dwBmpSize);
        file.close();

        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(nullptr, hScreenDC);
    }
    void SystemController::shutdown() {
        std::system("shutdown /s /t 0");
    }
    std::string SystemController::format_info() {
        std::ostringstream out;
        out << "<b>🖥️ Hostname:</b><code>" << get_computer_name() << "</code>\n";
        out << "<b>👤 User:</b><code>" << get_username() << "</code>\n\n";

        auto cpu = query_wmi_all(L"SELECT Name, NumberOfCores, NumberOfLogicalProcessors FROM Win32_Processor",
                                 {L"Name", L"NumberOfCores", L"NumberOfLogicalProcessors"});
        if (!cpu.empty()) {
            out << "<b>🧠 CPU:</b>"
            << "<code>" << cpu[0]["Name"] << "</code> | "
            << cpu[0]["NumberOfCores"] << " cores / "
            << cpu[0]["NumberOfLogicalProcessors"] << " threads\n";
        }

        auto gpus = query_wmi_all(L"SELECT Name FROM Win32_VideoController", {L"Name"});
        out << "\n<b>🎮 GPU(s):</b>\n";
        for (auto& gpu : gpus) {
            out << "• <code>" << gpu.at("Name") << "</code>\n";
        }

        auto ram_modules = query_wmi_all(L"SELECT Capacity FROM Win32_PhysicalMemory", {L"Capacity"});
        std::vector<uint64_t> capacities;
        for (const auto& ram : ram_modules)
            capacities.push_back(std::stoull(ram.at("Capacity")));
        if (!capacities.empty())
            out << "\n<b>💾 RAM:</b> "
                << "<code>" << format_capacity_gb(capacities) << "</code>\n";

        auto board = query_wmi_all(L"SELECT Manufacturer, Product FROM Win32_BaseBoard",
                                   {L"Manufacturer", L"Product"});
        if (!board.empty())
            out << "\n<b>🧩 Motherboard:</b> "
                << "<code>" << board[0]["Manufacturer"]
                << " (" << board[0]["Product"] << ")</code>\n";

        auto bios = query_wmi_all(L"SELECT SMBIOSBIOSVersion, ReleaseDate FROM Win32_BIOS",
                                  {L"SMBIOSBIOSVersion", L"ReleaseDate"});
        if (!bios.empty()) {
            std::string date_raw = bios[0]["ReleaseDate"];
            std::string date = date_raw.substr(6, 2) + "." + date_raw.substr(4, 2) + "." + date_raw.substr(0, 4);
            out << "\n<b>📦 BIOS:</b> "
                << "<code>" << bios[0]["SMBIOSBIOSVersion"]
                << " (" << date << ")</code>\n";
        }

        auto drives = query_wmi_all(L"SELECT DeviceID, FileSystem, Size FROM Win32_LogicalDisk WHERE DriveType = 3",
                                    {L"DeviceID", L"FileSystem", L"Size"});
        out << "💽 Drives:\n";
        for (const auto& d : drives) {
            uint64_t size_gb = std::stoull(d.at("Size")) / (1024ULL*1024*1024);
            out << "• <code>" << d.at("DeviceID")
                << " [" << d.at("FileSystem") << "]"
                << " — " << size_gb << "GB</code>\n";
        }

        std::string result = out.str();
        if (result.size() > 4000)
            result = result.substr(0,4000) + "\n\n<em>[...output truncated]</em>";
        return result;
    }
    std::string SystemController::get_username() {
        TCHAR name[UNLEN + 1];
        DWORD size = UNLEN + 1;
        GetUserName(name, &size);
        return Utils::tchar2s(name);
    }
    std::string SystemController::get_computer_name() {
        TCHAR name[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerName(name, &size);
        return Utils::tchar2s(name);
    }
    std::string SystemController::format_capacity_gb(const std::vector<uint64_t>& capacities) {
        uint64_t total_bytes = 0;
        for (const auto cap : capacities)
            total_bytes += cap;

        std::ostringstream oss;
        oss << total_bytes / (1024 * 1024 * 1024) << " GB (" << capacities.size() << "x"
            << capacities.front() / (1024 * 1024 * 1024) << " GB)";
        return oss.str();
    }
    bool SystemController::volume(float &volume) {
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr))
            return false;

        IMMDeviceEnumerator* imm_device_enumerator = nullptr;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator), reinterpret_cast<void **>(&imm_device_enumerator));
        if (FAILED(hr)) {
            CoUninitialize();
            return false;
        }

        IMMDevice* default_device = nullptr;
        hr = imm_device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &default_device);
        if (FAILED(hr)) {
            imm_device_enumerator->Release();
            CoUninitialize();
            return false;
        }

        IAudioEndpointVolume* endpoint_volume = nullptr;
        hr = default_device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, reinterpret_cast<void **>(&endpoint_volume));
        if (FAILED(hr)) {
            default_device->Release();
            imm_device_enumerator->Release();
            CoUninitialize();
            return false;
        }

        if (volume <= .0f)  endpoint_volume->GetMasterVolumeLevelScalar(&volume);
        else                endpoint_volume->SetMasterVolumeLevelScalar(volume, nullptr);

        endpoint_volume->Release();
        default_device->Release();
        imm_device_enumerator->Release();
        CoUninitialize();
        return true;
    }
    void SystemController::hibernate() {
        std::system("shutdown /h");
    }
    bool SystemController::sleep() {
        return !SetSuspendState(FALSE, TRUE, FALSE);
    }
    bool SystemController::kill_process(const uint32_t pid) {
        HANDLE h_process = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!h_process)
            return false;

        const BOOL result = TerminateProcess(h_process, 1);
        CloseHandle(h_process);
        return result == TRUE;
    }
    uint64_t SystemController::get_total_physical_memory() {
        MEMORYSTATUSEX msx{ sizeof(msx) };
        GlobalMemoryStatusEx(&msx);
        return msx.ullTotalPhys;
    }
    SystemController::ProcPerf SystemController::query_process_perf(const uint32_t pid) {
        ProcPerf p{0, 0.0};

        HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!h)
            return p;

        PROCESS_MEMORY_COUNTERS pmc{ sizeof(pmc) };
        if (GetProcessMemoryInfo(h, &pmc, sizeof(pmc))) {
            p.working_set  = pmc.WorkingSetSize;
            p.mem_percent  = 100.0 * static_cast<double>(pmc.WorkingSetSize)
                               / static_cast<double>(get_total_physical_memory());
        }

        CloseHandle(h);
        return p;
    }

}