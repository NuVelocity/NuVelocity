#include "AssetManager.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <format>
#include <physfs.h>
#include <physfssdl3.h>
#include <string_view>
#include <vector>

#include "AssetExporter.h"
#include "Font.h"
#include "FontBitmap.h"
#include "SequenceFrameInfoList.h"
#include "Utils.h"
#include "decoders/DecodeUtils.h"
#include "decoders/FrameLoaderMode3.h"
#include "decoders/SequenceLoaderMode3.h"
#include "decoders/SequenceLoaderMode4.h"
#include "model/PropertySerializer.h"

namespace nuvelocity
{
    constexpr const char* kPropertiesFileName = "Properties.txt";
    constexpr const char* kTgaExtension = ".tga";

#ifdef NVE_RESTORE_TGA
    AssetManager::AssetManager()
            : Manager()
            , mRestoreMode(false) {};
#else
    AssetManager::AssetManager() = default;
#endif

    AssetManager::~AssetManager()
    {
        PHYSFS_deinit();
    }

    bool AssetManager::Initialize(char** argv)
    {
        if (WarnIfAlreadyInitialized("AssetManager"))
        {
            return true;
        }

        if (PHYSFS_init(argv[0]) == 0)
        {
            SDL_LogError(
                NVE_LOG_CATEGORY_ENGINE, "Failed to initialize PhysFS: %s", GetErrorMessage());
            return false;
        }

        // Mount the executable's directory as the base search path
        const std::string basePath = SDL_GetBasePath();
        if (basePath.empty())
        {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE, "Failed to get base path: %s", SDL_GetError());
            return false;
        }
        SDL_Log("Working directory: %s", basePath.c_str());

        const std::string dataPath = std::format("{}Data.dat", basePath);
        if (PHYSFS_mount(dataPath.c_str(), nullptr, 0) == 0)
        {
            SDL_LogError(
                NVE_LOG_CATEGORY_ASSETS, "Failed to mount Data.dat: %s\n", GetErrorMessage());
            return false;
        }

        if (PHYSFS_setWriteDir(basePath.c_str()) == 0)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS,
                         "Failed to set PhysFS write dir '%s': %s",
                         basePath.c_str(),
                         GetErrorMessage());
            return false;
        }

        if (PHYSFS_mkdir("Data") == 0)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS,
                         "Failed to create CWD Data directory: %s",
                         GetErrorMessage());
            return false;
        }

        const std::string baseDataPathText = basePath + "Data";
        if (PHYSFS_mount(baseDataPathText.c_str(), nullptr, 0) == 0)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS,
                         "Failed to mount basepath Data directory '%s': %s",
                         baseDataPathText.c_str(),
                         GetErrorMessage());
            return false;
        }

        if (PHYSFS_setWriteDir(baseDataPathText.c_str()) == 0)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS,
                         "Failed to set PhysFS write dir '%s': %s",
                         baseDataPathText.c_str(),
                         GetErrorMessage());
            return false;
        }

        mInitialized = true;
        return true;
    }

    bool AssetManager::Exists(const std::string& path)
    {
        return PHYSFS_exists(path.c_str()) != 0;
    }

    SDL_IOStream* AssetManager::Load(const std::string& path)
    {
        return Load(path.c_str());
    }

    SDL_IOStream* AssetManager::Load(const char* path)
    {
#if DEBUG
        if (PHYSFS_exists(path) == 0)
        {
            SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS, "Asset '%s' does not exist.", path);
            return nullptr;
        }
        SDL_Log("Loading asset '%s'.", path);
#endif
        return PHYSFSSDL3_openRead(path);
    }

    SDL_IOStream* AssetManager::LoadWithExtension(const std::string& path,
                                                  const std::string& extension)
    {
        return PHYSFSSDL3_openRead((path + extension).c_str());
    }

    SDL_IOStream*
    AssetManager::LoadFromCache(const std::string& path, CacheKind kind, bool& loadedFromCache)
    {
        loadedFromCache = false;

        const std::string sourcePath = path + kTgaExtension;
        std::string cachePath = "Cache/" + path;

        switch (kind)
        {
        case CacheKind::StandAloneFrame:
            cachePath += ".Frame";
            break;
        case CacheKind::Sequence:
            cachePath += ".Sequence";
            break;
        default:
            break;
        }

        if (PHYSFS_exists(sourcePath.c_str()) != 0)
        {
            return PHYSFSSDL3_openRead(sourcePath.c_str());
        }

        if (PHYSFS_exists(cachePath.c_str()) != 0)
        {
            loadedFromCache = true;
            return PHYSFSSDL3_openRead(cachePath.c_str());
        }

        return nullptr;
    }

    StandAloneFrame* AssetManager::LoadStandAloneFrame(const std::string& path)
    {
        StandAloneFrame* frame = nullptr;
        bool loadedFromCache = false;

        SDL_IOStream* sourceStream = LoadWithExtension(path, kTgaExtension);
        if (sourceStream != nullptr)
        {
            SDL_Surface* sourceSurface = IMG_LoadTGA_IO(sourceStream);
            SDL_CloseIO(sourceStream);
            if (sourceSurface == nullptr)
            {
                SDL_LogError(NVE_LOG_CATEGORY_ASSETS,
                             "Failed to decode source frame '%s.tga': %s",
                             path.c_str(),
                             SDL_GetError());
                return nullptr;
            }

            frame = new StandAloneFrame();
            frame->SetSurface(sourceSurface);
            frame->SetSource(AssetSource::SourceAsset);
        }
        else
        {
            SDL_IOStream* stream = LoadFromCache(path, CacheKind::StandAloneFrame, loadedFromCache);
            if (stream == nullptr)
            {
                SDL_LogError(NVE_LOG_CATEGORY_ASSETS,
                             "Failed to load frame '%s': %s",
                             path.c_str(),
                             PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
                return nullptr;
            }

            frame = FrameLoaderMode3::Load(stream);
            SDL_CloseIO(stream);
            if (frame != nullptr)
            {
                frame->SetSource(loadedFromCache ? AssetSource::Cache : AssetSource::SourceAsset);
#ifdef NVE_RESTORE_TGA
                if (loadedFromCache && mRestoreMode &&
                    !AssetExporter::ExportStandAloneFrameToTga(path, *frame))
                {
                    SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                                "Failed to export stand-alone frame '%s' to source TGA",
                                path.c_str());
                }
#endif
            }
        }

        if (frame != nullptr)
        {
            const std::string sourcePropertiesPath = path + ".txt";
            const std::string propertiesText = LoadTextFile(sourcePropertiesPath);
            if (!propertiesText.empty())
            {
                void* dest = frame;
                PropertySerializer::Deserialize(propertiesText, dest);
            }
        }
        return frame;
    }

    Sequence* AssetManager::LoadSequence(const std::string& path)
    {
        Sequence* sourceSequence = LoadSourceSequenceFrames(path);
        if (sourceSequence != nullptr)
        {
            return sourceSequence;
        }

        bool loadedFromCache = false;
        SDL_IOStream* stream = LoadFromCache(path, CacheKind::Sequence, loadedFromCache);
        if (stream == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS,
                         "Failed to load sequence '%s': %s",
                         path.c_str(),
                         PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
            return nullptr;
        }

        const bool hasStandardHeader = DecodeUtils::FrameHasDeflateHeader(stream);
        bool hasFontLikeHeader = false;
        if (!hasStandardHeader)
        {
            hasFontLikeHeader = DecodeUtils::FontFrameHasDeflateHeader(stream);
        }

        SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET);

        Sequence* sequence = nullptr;
        if (hasStandardHeader || hasFontLikeHeader)
        {
            sequence = SequenceLoaderMode3::Load(stream);
            if (sequence == nullptr)
            {
                SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET);
                sequence = SequenceLoaderMode4::Load(stream);
            }
        }
        else
        {
            sequence = SequenceLoaderMode4::Load(stream);
            if (sequence == nullptr)
            {
                SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET);
                sequence = SequenceLoaderMode3::Load(stream);
            }
        }

        SDL_CloseIO(stream);

        if (sequence == nullptr)
        {
            return nullptr;
        }

        if (loadedFromCache)
        {
            sequence->SetSource(AssetSource::Cache);
#ifdef NVE_RESTORE_TGA
            if (mRestoreMode && !AssetExporter::ExportSequenceToTga(path, *sequence))
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to export sequence '%s' to source TGA frames",
                            path.c_str());
            }
#endif
        }

        return sequence;
    }

    Font* AssetManager::LoadFont(const std::string& path)
    {
        Font* font = static_cast<Font*>(LoadPropertyFile(path));
        if (font == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS, "Failed to load font '%s'.", path.c_str());
            return nullptr;
        }
        SDL_IOStream* stream = Load(font->GetFontFamily());
        if (stream == nullptr)
        {
            return nullptr;
        }
        font->AttachFontStream(stream);
        return font;
    }

    FontBitmap* AssetManager::LoadFontBitmap(const std::string& path)
    {
        Sequence* sourceSequence = LoadSourceSequenceFrames(path);
        if (sourceSequence != nullptr)
        {
            auto* fontBitmap = new FontBitmap();
            fontBitmap->SetSequence(std::unique_ptr<Sequence>(sourceSequence));

            // Source TGA folders do not carry the binary font pre-header.
            // Use common ASCII range defaults based on available glyph frames.
            constexpr int kDefaultFirstAscii = 32;
            const int frameCount = static_cast<int>(sourceSequence->GetFrameCount());
            fontBitmap->SetFirstAscii(kDefaultFirstAscii);
            fontBitmap->SetLastAscii(kDefaultFirstAscii + SDL_max(0, frameCount - 1));

            Frame* firstFrame = sourceSequence->GetFrame(0);
            fontBitmap->SetXHeight(firstFrame != nullptr ? firstFrame->GetHeight() : 0);
            return fontBitmap;
        }

        bool loadedFromCache = false;
        SDL_IOStream* stream = LoadFromCache(path, CacheKind::Sequence, loadedFromCache);
        if (stream == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS,
                         "Failed to load font bitmap '%s': %s",
                         path.c_str(),
                         PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
            return nullptr;
        }

        FontBitmap* fontBitmap = SequenceLoaderMode3::LoadFontBitmap(stream);
        SDL_CloseIO(stream);

        if (fontBitmap == nullptr)
        {
            return nullptr;
        }

#ifdef NVE_RESTORE_TGA
        if (loadedFromCache)
        {
            const Sequence* sequence = fontBitmap->GetSequence();
            if (sequence != nullptr && mRestoreMode &&
                !AssetExporter::ExportSequenceToTga(path, *sequence))
            {
                SDL_LogWarn(NVE_LOG_CATEGORY_ASSETS,
                            "Failed to export font bitmap '%s' to source TGA frames",
                            path.c_str());
            }
        }
#endif

        return fontBitmap;
    }

    std::string AssetManager::LoadTextFile(const std::string& path)
    {
        SDL_IOStream* stream = PHYSFSSDL3_openRead(path.c_str());
        if (stream == nullptr)
        {
            return "";
        }

        const int64_t fileSize = SDL_GetIOSize(stream);
        if (fileSize <= 0)
        {
            SDL_CloseIO(stream);
            return "";
        }

        std::string text;
        text.resize(static_cast<size_t>(fileSize));
        if (SDL_ReadIO(stream, text.data(), fileSize) == 0)
        {
            SDL_CloseIO(stream);
            return "";
        }
        SDL_CloseIO(stream);
        return text;
    }

    void AssetManager::DumpPropertyFile(const std::string& path)
    {
        const std::string text = LoadTextFile(path);
        void* dest = nullptr;
        ClassInfo* info = nullptr;

        if (PropertySerializer::Deserialize(text, dest, info))
        {
            info->DumpFor(dest);
        }
    }

    void* AssetManager::LoadPropertyFile(const std::string& path)
    {
        const std::string text = LoadTextFile(path);
        void* dest = nullptr;
        ClassInfo* info = nullptr;

        if (PropertySerializer::Deserialize(text, dest, info))
        {
            return dest;
        }
        return nullptr;
    }

    JWindowSkin* AssetManager::LoadWindowSkin(const std::string& path)
    {
        JWindowSkin* skin = static_cast<JWindowSkin*>(LoadPropertyFile(path));
        if (skin != nullptr)
        {
            skin->Load(this);
        }
        return skin;
    }

    SDL_Surface* AssetManager::LoadSurfaceFromAssetPath(const std::string& assetPath)
    {
        SDL_IOStream* frameStream = PHYSFSSDL3_openRead(assetPath.c_str());
        if (frameStream == nullptr)
        {
            return nullptr;
        }

        SDL_Surface* surface = IMG_LoadTGA_IO(frameStream);
        SDL_CloseIO(frameStream);
        return surface;
    }

    bool AssetManager::LoadFrameSurfaces(const std::filesystem::path& sequenceDirectoryPath,
                                         std::vector<SDL_Surface*>& frames)
    {
        char** files = PHYSFS_enumerateFiles(sequenceDirectoryPath.generic_string().c_str());
        if (files == nullptr)
        {
            return false;
        }

        frames.clear();

        for (char** entry = files; *entry != nullptr; ++entry)
        {
            const std::string fileName(*entry);
            if (!endsWithCaseInsensitive(fileName, kTgaExtension))
            {
                continue;
            }
            const std::filesystem::path framePath = sequenceDirectoryPath / fileName;
            SDL_Surface* frameSurface = LoadSurfaceFromAssetPath(framePath.generic_string());
            if (frameSurface == nullptr)
            {
                PHYSFS_freeList(static_cast<void*>(files));
                return false;
            }

            frames.push_back(frameSurface);
        }

        PHYSFS_freeList(static_cast<void*>(files));

        return !frames.empty();
    }

    void AssetManager::DestroyFrameSurfaces(std::vector<SDL_Surface*>& frames)
    {
        for (SDL_Surface* frame : frames)
        {
            SDL_DestroySurface(frame);
        }
        frames.clear();
    }

    Sequence* AssetManager::LoadSourceSequenceFrames(const std::string& path)
    {
        const std::filesystem::path sourcePath(path);
        const std::string baseName = sourcePath.filename().string();
        if (baseName.empty())
        {
            return nullptr;
        }

        const std::filesystem::path sequenceDirectoryPath =
            sourcePath.parent_path() / ("-" + baseName);
        const std::filesystem::path propertiesPath = sequenceDirectoryPath / kPropertiesFileName;
        if (PHYSFS_exists(propertiesPath.generic_string().c_str()) == 0)
        {
            return nullptr;
        }

        std::vector<SDL_Surface*> frames;
        if (!LoadFrameSurfaces(sequenceDirectoryPath, frames))
        {
            DestroyFrameSurfaces(frames);
            return nullptr;
        }

        Sequence* sequence = nullptr;
        SequenceFrameInfoList* frameInfoList = nullptr;
        const std::string propertyText = LoadTextFile(propertiesPath.generic_string());
        if (!propertyText.empty())
        {
            DecodeUtils::DeserializeSequenceRoots(propertyText, sequence, frameInfoList);
        }

        if (sequence == nullptr)
        {
            sequence = new Sequence();
        }

        if (frameInfoList != nullptr)
        {
            frameInfoList->CopyTo(*sequence, BlitTypeRevision::Type1);
            delete frameInfoList;
        }

        sequence->SetFrames(std::move(frames));
        sequence->SetSource(AssetSource::SourceAsset);
        return sequence;
    }
} // namespace nuvelocity
