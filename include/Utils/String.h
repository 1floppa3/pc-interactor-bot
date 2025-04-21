#pragma once

#include <vector>
#include <string>

namespace Utils {

    inline std::vector<std::string> split(const std::string& str, const char delim, const bool keep_empty = false) {
        std::vector<std::string> parts;
        size_t start = 0;
        while (true) {
            const size_t pos = str.find(delim, start);
            if (pos == std::string::npos) {
                std::string token = str.substr(start);
                if (keep_empty || !token.empty())
                    parts.push_back(std::move(token));
                break;
            }
            std::string token = str.substr(start, pos - start);
            if (keep_empty || !token.empty())
                parts.push_back(std::move(token));
            start = pos + 1;
        }
        return parts;
    }

    inline std::string html_escape(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (const char c : s) {
            switch (c) {
                case '&':  out.append("&amp;");  break;
                case '<':  out.append("&lt;");   break;
                case '>':  out.append("&gt;");   break;
                default:   out.push_back(c);     break;
            }
        }
        return out;
    }

    inline std::string to_lower(std::string s) {
        for (auto& c : s) c = static_cast<char>(std::tolower(c));
        return s;
    }

}