#ifndef NVE_CLASSINFO_H
#define NVE_CLASSINFO_H

#include <string>
#include <unordered_map>
#include "Property.h"

namespace nuvelocity
{
    struct ClassInfo
    {
    private:
        Property* mLastProperty;
    public:
        std::string mName;
        std::unordered_map<std::string, Property*> mProperties;
        ClassInfo* mBaseClassInfo;

        void* (*mFactoryFunction)();

        void AddProperty(Property* prop)
        {
            if (!prop)
            {
                return;
            }
            mProperties[prop->GetName()] = prop;
            mLastProperty = prop;
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
            for (const auto& pair : mProperties)
            {
                pair.second->DumpMetadata();
            }
        }

        void DumpFor(void* obj) const
        {
            SDL_Log("Property values for object of class '%s':", mName.c_str());
            for (const auto& pair : mProperties)
            {
                pair.second->DumpValue(obj);
            }
        }
    };
} // namespace nuvelocity


#endif // NVE_CLASSINFO_H
