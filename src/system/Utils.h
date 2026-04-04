#ifndef NVE_UTILS_H
#define NVE_UTILS_H

#include <cctype>
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

inline bool endsWithCaseInsensitive(const std::string& value, const std::string& suffix)
{
    if (value.size() < suffix.size())
    {
        return false;
    }

    const std::size_t offset = value.size() - suffix.size();
    for (std::size_t index = 0; index < suffix.size(); ++index)
    {
        const char currentCharacter =
            static_cast<char>(std::tolower(static_cast<unsigned char>(value[offset + index])));
        const char suffixCharacter =
            static_cast<char>(std::tolower(static_cast<unsigned char>(suffix[index])));
        if (currentCharacter != suffixCharacter)
        {
            return false;
        }
    }

    return true;
}

#endif // NVE_UTILS_H
