#ifndef NVE_MANAGER_H
#define NVE_MANAGER_H

#include "API.h"
#include <SDL3/SDL_log.h>
#include "LogCategory.h"

namespace nuvelocity
{
    class Manager
    {
    public:
        virtual ~Manager() = default;
        virtual bool Initialize(char** argv) = 0;
    protected:
        bool mInitialized;

        Manager()
            : mInitialized(false)
        {
        }
        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;

        inline bool WarnIfAlreadyInitialized(const char* managerName)
        {
            if (mInitialized)
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ENGINE,
                    "%s is already initialized", managerName);
                return true;
            }
            return false;
        }
    };
} // namespace nuvelocity

#endif // NVE_MANAGER_H
