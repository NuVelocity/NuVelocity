#ifndef NVE_ASSETMANAGER_H
#define NVE_ASSETMANAGER_H

#include "API.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <string>

#include "Manager.h"
#include "Sequence.h"
#include "StandAloneFrame.h"

namespace nuvelocity
{
    class AssetManager : public Manager
    {
    private:
        enum CacheKind
        {
            CACHE_KIND_UNKNOWN = 0,
            CACHE_KIND_STANDALONE_FRAME = 1,
            CACHE_KIND_SEQUENCE = 2
        };

        static SDL_IOStream* LoadWithExtension(const std::string& path,
                                               const std::string& extension);

        static SDL_IOStream* LoadFromCache(const std::string& path, CacheKind kind);

        static inline const char* GetErrorMessage();

    public:
        NVE_API AssetManager();
        NVE_API ~AssetManager();

        NVE_API bool Initialize(char** argv) override;

        NVE_API static bool Exists(const std::string& path);

        NVE_API static SDL_IOStream* Load(const char* path);

        NVE_API static SDL_IOStream* Load(const std::string& path);

        NVE_API static StandAloneFrame* LoadStandAloneFrame(const std::string& path);
        NVE_API static Sequence* LoadSequence(const std::string& path);

        NVE_API static std::string LoadTextFile(const std::string& path);

        NVE_API static void DumpPropertyFile(const std::string& path);

        NVE_API static void* LoadPropertyFile(const std::string& path);
    };
} // namespace nuvelocity

#endif // NVE_ASSETMANAGER_H
