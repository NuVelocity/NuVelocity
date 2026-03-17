#ifndef NVE_UTILS_H
#define NVE_UTILS_H

#include <string>

inline const char* ws = " \t\n\r\f\v";
inline const char kTokenEquals = '=';

inline std::string& rtrim(std::string& s, const char* t = ws)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

inline std::string& ltrim(std::string& s, const char* t = ws)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

inline std::string& trim(std::string& s, const char* t = ws)
{
    return ltrim(rtrim(s, t), t);
}

inline bool isKeyValuePair(const std::string& line)
{
    return line.find(kTokenEquals) != std::string::npos;
}

inline std::pair<std::string, std::string>* parseKeyValuePair(const std::string& line)
{
    if (!isKeyValuePair(line))
    {
        return nullptr;
    }

    size_t pos = line.find(kTokenEquals);
    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);

    return new std::pair<std::string, std::string>(trim(key), trim(value));
}

#endif // NVE_UTILS_H
