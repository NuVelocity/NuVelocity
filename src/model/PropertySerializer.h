#ifndef NVE_PROPERTY_SERIALIZER_H
#define NVE_PROPERTY_SERIALIZER_H

#include "API.h"
#include "ClassInfo.h"
#include <string>

namespace nuvelocity
{
    class PropertySerializer
    {
    public:
        template <typename T>
        static void Serialize(const T& value, std::string& output);

        template <typename T>
        static bool Deserialize(const std::string& input, T*& output, ClassInfo*& info);

        template <typename T>
        static bool Deserialize(const std::string& input, T*& output);
    };
} // namespace nuvelocity

#endif // NVE_PROPERTY_SERIALIZER_H
