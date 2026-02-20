#ifndef NVE_OBJECT_REGISTRY_H
#define NVE_OBJECT_REGISTRY_H

#include "ClassInfo.h"
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace nuvelocity
{
    class ObjectRegistry
    {
    private:
        ObjectRegistry() = default;
        std::unordered_map<std::string, ClassInfo*> mClasses;

    public:
        static ObjectRegistry& Get()
        {
            static ObjectRegistry instance;
            return instance;
        }

        void Register(ClassInfo* info)
        {
            mClasses[info->mName] = info;
        }

        ClassInfo* Find(const std::string& name)
        {
            try
            {
                return mClasses.at(name);
            }
            catch (const std::out_of_range&)
            {
                return nullptr;
            }
        }
    };
} // namespace nuvelocity

#endif // NVE_OBJECT_REGISTRY_H
