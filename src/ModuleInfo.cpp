#include "ModuleInfo.h"

namespace nuvelocity
{
    ModuleInfo::ModuleInfo()
            : mModuleName("Unnamed Module")
            , mModuleVersion("0.0.1")
            , mModuleId("com.nuvelocity.unnamedmodule")
            , mBuildNumber(1)
    {
    }

    ModuleInfo::~ModuleInfo() = default;
} // namespace nuvelocity
