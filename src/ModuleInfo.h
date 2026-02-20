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
            AddProperty(aInfo, "Module Display Name", &ModuleInfo::mModuleName);
            AddProperty(aInfo, "Build Number", &ModuleInfo::mBuildNumber);
            AddProperty(aInfo, "Display Version Number", &ModuleInfo::mModuleVersion);
        }
    private:
        std::string mModuleName;
        std::string mModuleVersion;
        int mBuildNumber;
    };
} // namespace nuvelocity
