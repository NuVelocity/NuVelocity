#include "PropertySerializer.h"
#include "ObjectRegistry.h"
#include "Utils.h"
#include "system/LogCategory.h"
#include <sstream>
#include <stdexcept>
#include <vector>

namespace nuvelocity
{
    template <typename T>
    void PropertySerializer::Serialize(const T& value, std::string& output)
    {
        throw std::logic_error("Serialize not implemented for this type");
    }

    template <typename T>
    bool PropertySerializer::Deserialize(const std::string& input, T*& output, ClassInfo*& info)
    {
        std::vector<std::string> lines;
        std::stringstream stream(input);
        std::string line;

        info = nullptr;
        output = nullptr;

        while (std::getline(stream, line))
        {
            line = trim(line);
            if (line.empty())
            {
                continue;
            }
            if (line == "{" || line == "}")
            {
                continue;
            }
            if (line.find('=') != std::string::npos)
            {
                if (info == nullptr || output == nullptr)
                {
                    SDL_LogWarn(NVE_LOG_CATEGORY_PROPSYS,
                                "Missing object context for property assignment: '%s'",
                                line.c_str());
                    continue;
                }
                std::string key = line.substr(0, line.find('='));
                std::string value = line.substr(line.find('=') + 1);
                if (auto* prop = info->GetProperty(key); prop != nullptr)
                {
                    prop->SetValue(output, value);
                }
                else
                {
                    SDL_LogWarn(NVE_LOG_CATEGORY_PROPSYS, "Unknown property '%s' for class '%s'",
                                key.c_str(), info->mName.c_str());
                }
                // SDL_Log("%s: %s", key.c_str(), value.c_str());
            }
            else
            {
                info = ObjectRegistry::Get().Find(line);
                if (info != nullptr)
                {
                    output = info->mFactoryFunction();
                    SDL_Log("Created object of type: %s", info->mName.c_str());
                }
                else
                {
                    SDL_Log("%s", line.c_str());
                }
            }
        }

        if (output == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_PROPSYS,
                         "Failed to deserialize object: No class information found.");
            return false;
        }

        return true;
    }

    template <typename T>
    bool PropertySerializer::Deserialize(const std::string& input, T*& output)
    {
        ClassInfo* info = nullptr;
        return Deserialize(input, output, info);
    }
} // namespace nuvelocity
