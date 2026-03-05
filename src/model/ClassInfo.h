#ifndef NVE_CLASSINFO_H
#define NVE_CLASSINFO_H

#include "Property.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace nuvelocity
{
    struct ClassInfo
    {
    private:
        Property* mFirstProperty;
        Property* mLastProperty;

    public:
        std::string mName;
        std::unordered_map<std::string, Property*> mProperties;
        ClassInfo* mBaseClassInfo;

        void* (*mFactoryFunction)(const std::vector<std::string>& args);

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
