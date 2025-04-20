#pragma once

#include <codecvt>
#include <locale>
#include <string>

namespace Utils {

    inline std::wstring s2ws(const std::string& str)
    {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX> converterX;

        return converterX.from_bytes(str);
    }

    inline std::string ws2s(const std::wstring& wstr)
    {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX> converterX;

        return converterX.to_bytes(wstr);
    }

    inline std::string tchar2s(const TCHAR* tchar_str) {
#ifdef UNICODE
        // TCHAR == wchar_t → codecvt
        std::wstring wstr(tchar_str);
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type> converter;
        return converter.to_bytes(wstr);
#else
        // TCHAR == char → string
        return tchar_str;
#endif
    }

}