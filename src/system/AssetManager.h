#ifndef NVE_ASSETMANAGER_H
#define NVE_ASSETMANAGER_H

#include "API.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <cstdint>
#include <filesystem>
#include <physfs.h>
#include <string>
#include <unordered_map>
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
        enum class CacheKind : uint8_t
        {
            Unknown = 0,
            StandAloneFrame = 1,
            Sequence = 2
        };

        SDL_IOStream* LoadWithExtension(const std::string& path, const std::string& extension);
        SDL_IOStream* LoadFromCache(const std::string& path, CacheKind kind, bool& loadedFromCache);
        SDL_Surface* LoadSurfaceFromAssetPath(const std::string& assetPath);
        bool LoadFrameSurfaces(const std::filesystem::path& sequenceDirectoryPath,
                               std::vector<std::unique_ptr<Frame>>& frames);
        void DestroyFrameSurfaces(std::vector<std::unique_ptr<Frame>>& frames);
        Sequence* LoadSourceSequenceFrames(const std::string& path);
        std::vector<std::string> EnumerateFiles(const std::string& path);

        static const char* GetErrorMessage()
        {
            return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
        }

        std::unordered_map<std::string, std::unique_ptr<StandAloneFrame>> mStandAloneFrames;
        std::unordered_map<std::string, std::unique_ptr<Sequence>> mSequences;
        std::unordered_map<std::string, std::unique_ptr<Font>> mFonts;
        std::unordered_map<std::string, std::unique_ptr<FontBitmap>> mFontBitmaps;
        std::unordered_map<std::string, SDL_IOStream*> mMusicStreams;
        std::unordered_map<std::string, SDL_IOStream*> mSoundStreams;

#ifdef NVE_RESTORE_TGA
        bool mRestoreMode;
#endif

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
        NVE_API FontBitmap* LoadFontBitmapFromFrame(const std::string& path);
        NVE_API SDL_IOStream* LoadMusic(const std::string& path);
        NVE_API SDL_IOStream* LoadSound(const std::string& path);

        NVE_API StandAloneFrame* TryLoadStandAloneFrame(const std::string& path);
        NVE_API Sequence* TryLoadSequence(const std::string& path);

        NVE_API std::vector<std::pair<std::string, std::string>> EnumerateRoundSets();
        NVE_API std::vector<std::pair<std::string, std::string>> EnumerateRawFontBitmaps();
        NVE_API std::vector<std::pair<std::string, std::string>> EnumerateFonts();

        NVE_API std::string LoadTextFile(const std::string& path);

        NVE_API void DumpPropertyFile(const std::string& path);

        NVE_API void* LoadPropertyFile(const std::string& path);
        NVE_API void* LoadBrickInfo(const std::string& path);
        NVE_API void* LoadBackgroundDefinition(const std::string& path);

        NVE_API JWindowSkin* LoadWindowSkin(const std::string& path);

#ifdef NVE_RESTORE_TGA
        NVE_API void SetRestoreMode(bool enabled)
        {
            mRestoreMode = enabled;
        }
#endif
    };
} // namespace nuvelocity

#endif // NVE_ASSETMANAGER_H
