#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <format>
#include <physfs.h>
#include <physfssdl3.h>
#include <string>

#include "AssetManager.h"

#include "Utils.h"
#include "decoders/FrameLoaderMode3.h"
#include "decoders/SequenceLoaderMode3.h"
#include "model/PropertySerializer.h"

namespace nuvelocity
{
    AssetManager::AssetManager() = default;

    AssetManager::~AssetManager()
    {
        PHYSFS_deinit();
    }

    inline const char* AssetManager::GetErrorMessage()
    {
        return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
    }

    bool AssetManager::Initialize(char** argv)
    {
        if (WarnIfAlreadyInitialized("AssetManager"))
        {
            return true;
        }

        if (PHYSFS_init(argv[0]) == 0)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE, "Failed to initialize PhysFS: %s",
                         GetErrorMessage());
            return false;
        }

        // Mount the executable's directory as the base search path
        std::string basePath = SDL_GetBasePath();
        if (basePath.empty())
        {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE, "Failed to get base path: %s", SDL_GetError());
            return false;
        }
        SDL_Log("Working directory: %s", basePath.c_str());

        std::string dataPath = std::format("{}Data.dat", basePath);
        if (PHYSFS_mount(dataPath.c_str(), nullptr, 0) == 0)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS, "Failed to mount Data.dat: %s\n",
                         GetErrorMessage());
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
        std::string fullPath = path + extension;
        return Load(fullPath);
    }

    SDL_IOStream* AssetManager::LoadFromCache(const std::string& path, CacheKind kind)
    {
        std::string fullPath = path;
        std::string cachedPath = "Cache/" + fullPath;
        switch (kind)
        {
        case CACHE_KIND_STANDALONE_FRAME:
            fullPath += ".tga";
            cachedPath += ".Frame";
            break;
        case CACHE_KIND_SEQUENCE:
            fullPath += ".tga";
            cachedPath += ".Sequence";
            break;
        default:
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS, "Unknown cache kind for asset '%s'",
                         path.c_str());
            break;
        }
        if (Exists(cachedPath))
        {
            return Load(cachedPath);
        }
        return Load(fullPath);
    }

    StandAloneFrame* AssetManager::LoadStandAloneFrame(const std::string& path)
    {
        auto* stream = LoadFromCache(path, CACHE_KIND_STANDALONE_FRAME);
        if (stream == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS, "Failed to load frame '%s': %s", path.c_str(),
                         GetErrorMessage());
            return nullptr;
        }

        StandAloneFrame* frame = FrameLoaderMode3::Load(stream);
        SDL_CloseIO(stream);
        return frame;
    }

    Sequence* AssetManager::LoadSequence(const std::string& path)
    {
        auto* stream = LoadFromCache(path, CACHE_KIND_SEQUENCE);
        if (stream == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ASSETS, "Failed to load sequence '%s': %s", path.c_str(),
                         GetErrorMessage());
            return nullptr;
        }

        Sequence* sequence = SequenceLoaderMode3::Load(stream);
        SDL_CloseIO(stream);
        return sequence;
    }

    std::string AssetManager::LoadTextFile(const std::string& path)
    {
        auto* stream = Load(path);
        if (stream == nullptr)
        {
            return "";
        }

        int64_t fileSize = SDL_GetIOSize(stream);
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
        auto text = LoadTextFile(path);
        void* dest = nullptr;
        ClassInfo* info = nullptr;

        if (PropertySerializer::Deserialize(text, dest, info))
        {
            info->DumpFor(dest);
        }
    }

    void* AssetManager::LoadPropertyFile(const std::string& path)
    {
        auto text = LoadTextFile(path);
        void* dest = nullptr;
        ClassInfo* info = nullptr;

        if (PropertySerializer::Deserialize(text, dest, info))
        {
            return dest;
        }
        return nullptr;
    }
} // namespace nuvelocity
