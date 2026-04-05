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

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CModuleInfo";
            AddProperty(aInfo, "Module ID", &ModuleInfo::mModuleId);
            AddProperty(aInfo, "Module Display Name", &ModuleInfo::mModuleName);
            AddProperty(aInfo, "Build Number", &ModuleInfo::mBuildNumber);
            AddProperty(aInfo, "Display Version Number", &ModuleInfo::mModuleVersion);
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
