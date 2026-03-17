#ifndef NVE_CLASSINFO_H
#define NVE_CLASSINFO_H

#include "Property.h"
#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

namespace nuvelocity
{
    enum class SerializationMode
    {
        Standard, // Normal property-based serialization
        HexArray  // Special mode for hex array data (e.g., CFrame pixel data)
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
        Property* mHexArrayProperty = nullptr; // Property to receive hex array binary data

        void* (*mFactoryFunction)();

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

        void SetHexArrayProperty(Property* prop)
        {
            assert(mHexArrayProperty == nullptr && "Cannot set multiple hex array properties: a "
                                                   "hex array property is already defined");
            mHexArrayProperty = prop;
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
