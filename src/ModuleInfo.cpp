#include "ModuleInfo.h"

namespace nuvelocity
{
    ModuleInfo::ModuleInfo()
            : mModuleName("Unnamed Module")
            , mModuleVersion("0.0.1")
            , mBuildNumber(1)
    {
    }

    ModuleInfo::~ModuleInfo() = default;
} // namespace nuvelocity
