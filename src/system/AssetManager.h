#ifndef NVE_ASSETMANAGER_H
#define NVE_ASSETMANAGER_H

#include "API.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <filesystem>
#include <physfs.h>
#include <string>
#include <vector>

#include "Manager.h"
#include "Sequence.h"
#include "StandAloneFrame.h"

namespace nuvelocity
{
    class Font;
    class FontBitmap;
    class SequenceFrameInfoList;

    class AssetManager : public Manager
    {
    private:
        enum class CacheKind
        {
            Unknown = 0,
            StandAloneFrame = 1,
            Sequence = 2
        };

        static SDL_IOStream* LoadWithExtension(const std::string& path,
                                               const std::string& extension);
        static SDL_IOStream*
        LoadFromCache(const std::string& path, CacheKind kind, bool& loadedFromCache);
        static SDL_Surface* LoadSurfaceFromAssetPath(const std::string& assetPath);
        static bool LoadFrameSurfaces(const std::filesystem::path& sequenceDirectoryPath,
                                      std::vector<SDL_Surface*>& frames);
        static void DestroyFrameSurfaces(std::vector<SDL_Surface*>& frames);
        static Sequence* LoadSourceSequenceFrames(const std::string& path);

        static inline const char* GetErrorMessage()
        {
            return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
        }

    public:
        NVE_API AssetManager();
        NVE_API ~AssetManager();

        NVE_API bool Initialize(char** argv) override;

        NVE_API static bool Exists(const std::string& path);

        NVE_API static SDL_IOStream* Load(const char* path);

        NVE_API static SDL_IOStream* Load(const std::string& path);

        NVE_API static StandAloneFrame* LoadStandAloneFrame(const std::string& path);
        NVE_API static Sequence* LoadSequence(const std::string& path);
        NVE_API static Font* LoadFont(const std::string& path);
        NVE_API static FontBitmap* LoadFontBitmap(const std::string& path);

        NVE_API static std::string LoadTextFile(const std::string& path);

        NVE_API static void DumpPropertyFile(const std::string& path);

        NVE_API static void* LoadPropertyFile(const std::string& path);
    };
} // namespace nuvelocity

#endif // NVE_ASSETMANAGER_H
