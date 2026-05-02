#ifndef NVE_CLASSINFO_H
#define NVE_CLASSINFO_H

#include "Property.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace nuvelocity
{
    struct ByteArrayInfo
    {
        int cols = 0;
        int rows = 0;
        int bits = 0;
        int pitch = 0; // Bytes per row in the target buffer (0 means packed)

        bool IsValid() const
        {
            return cols > 0 && rows > 0 && bits > 0 && (bits % 8) == 0;
        }

        size_t ByteCount() const
        {
            return static_cast<size_t>(cols) * static_cast<size_t>(rows) *
                   (static_cast<size_t>(bits) / 8);
        }
    };

    struct ByteArrayTarget
    {
        ByteArrayInfo info = {};
        uint8_t* data = nullptr;
    };

    enum class SerializationMode : uint8_t
    {
        Standard, // Normal property-based serialization
        ByteArray // Special mode for byte array data (e.g., CFrame pixel data)
    };

    struct ClassInfo
    {
    private:
        Property* mFirstProperty;
        Property* mLastProperty;

    public:
        std::string mName;
        std::unordered_map<std::string, Property*> mProperties;
        ClassInfo* mBaseClassInfo;
        SerializationMode mSerializationMode = SerializationMode::Standard;
        Property* mByteArrayProperty = nullptr; // Property to receive byte array binary data

        void* (*mFactoryFunction)();
        ByteArrayInfo (*mByteArrayInfoFunction)(const void* obj) = nullptr;
        void (*mByteArrayInitFunction)(void* obj, const ByteArrayInfo& info) = nullptr;

        void AddProperty(Property* prop)
        {
            if (!prop)
            {
                return;
            }
            mProperties[prop->GetName()] = prop;
            if (mLastProperty != nullptr)
            {
                mLastProperty->mNext = prop;
            }
            if (mFirstProperty == nullptr)
            {
                mFirstProperty = prop;
            }
            mLastProperty = prop;
        }

        void SetByteArrayProperty(Property* prop)
        {
            assert(mByteArrayProperty == nullptr && "Cannot set multiple byte array properties: a "
                                                    "byte array property is already defined");
            mByteArrayProperty = prop;
        }

        Property* GetFirstProperty() const
        {
            return mFirstProperty;
        }

        Property* GetLastProperty() const
        {
            return mLastProperty;
        }

        Property* GetProperty(const std::string& name) const
        {
            auto it = mProperties.find(name);
            return it != mProperties.end() ? it->second : nullptr;
        }

        void DumpMetadata() const
        {
            SDL_Log("Properties for class '%s':", mName.c_str());
            Property* prop = mFirstProperty;
            while (prop != nullptr)
            {
                prop->DumpMetadata();
                prop = prop->mNext;
            }
        }

        void DumpFor(void* obj) const
        {
            SDL_Log("Property values for object of class '%s':", mName.c_str());
            Property* prop = mFirstProperty;
            while (prop != nullptr)
            {
                prop->DumpValue(obj);
                prop = prop->mNext;
            }
        }
    };
} // namespace nuvelocity

#endif // NVE_CLASSINFO_H
