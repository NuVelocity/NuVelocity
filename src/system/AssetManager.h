#ifndef NVE_ASSETMANAGER_H
#define NVE_ASSETMANAGER_H

#include "API.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <filesystem>
#include <physfs.h>
#include <string>
#include <vector>

#include "Font.h"
#include "FontBitmap.h"
#include "Manager.h"
#include "Sequence.h"
#include "SequenceFrameInfoList.h"
#include "StandAloneFrame.h"
#include "system/ui/skin/JWindowSkin.h"

namespace nuvelocity
{
    class AssetManager : public Manager
    {
    private:
        enum class CacheKind
        {
            Unknown = 0,
            StandAloneFrame = 1,
            Sequence = 2
        };

        SDL_IOStream* LoadWithExtension(const std::string& path, const std::string& extension);
        SDL_IOStream* LoadFromCache(const std::string& path, CacheKind kind, bool& loadedFromCache);
        SDL_Surface* LoadSurfaceFromAssetPath(const std::string& assetPath);
        bool LoadFrameSurfaces(const std::filesystem::path& sequenceDirectoryPath,
                               std::vector<SDL_Surface*>& frames);
        void DestroyFrameSurfaces(std::vector<SDL_Surface*>& frames);
        Sequence* LoadSourceSequenceFrames(const std::string& path);

        static inline const char* GetErrorMessage()
        {
            return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
        }

    public:
        NVE_API AssetManager();
        NVE_API ~AssetManager();

        NVE_API bool Initialize(char** argv) override;

        NVE_API bool Exists(const std::string& path);

        NVE_API SDL_IOStream* Load(const char* path);

        NVE_API SDL_IOStream* Load(const std::string& path);

        NVE_API StandAloneFrame* LoadStandAloneFrame(const std::string& path);
        NVE_API Sequence* LoadSequence(const std::string& path);
        NVE_API Font* LoadFont(const std::string& path);
        NVE_API FontBitmap* LoadFontBitmap(const std::string& path);

        NVE_API std::string LoadTextFile(const std::string& path);

        NVE_API void DumpPropertyFile(const std::string& path);

        NVE_API void* LoadPropertyFile(const std::string& path);

        NVE_API JWindowSkin* LoadWindowSkin(const std::string& path);
    };
} // namespace nuvelocity

#endif // NVE_ASSETMANAGER_H
