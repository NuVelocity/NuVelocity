#ifndef NVE_MODULE_INFO_H
#define NVE_MODULE_INFO_H

#include "model/Model.h"
#include <string>

namespace nuvelocity
{
    class ModuleInfo : public Object<ModuleInfo>
    {
    public:
        ModuleInfo();
        ~ModuleInfo();

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CModuleInfo";
            AddProperty(info, "Module ID", &ModuleInfo::mModuleId);
            AddProperty(info, "Module Display Name", &ModuleInfo::mModuleName);
            AddProperty(info, "Build Number", &ModuleInfo::mBuildNumber);
            AddProperty(info, "Display Version Number", &ModuleInfo::mModuleVersion);
        }

        std::string GetModuleName() const
        {
            return mModuleName;
        }

        std::string GetModuleVersion() const
        {
            return mModuleVersion;
        }

        std::string GetModuleId() const
        {
            return mModuleId;
        }

        int GetBuildNumber() const
        {
            return mBuildNumber;
        }

    private:
        std::string mModuleName;
        std::string mModuleVersion;
        std::string mModuleId;
        int mBuildNumber;
    };
} // namespace nuvelocity

#endif // NVE_MODULE_INFO_H
