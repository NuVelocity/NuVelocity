#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>
#include <format>
#include <physfs.h>
#include <physfssdl3.h>
// tmp
#include <sstream>
#include <string>
#include <vector>

#include "AssetManager.h"

#include "ModuleInfo.h"
#include "PaletteHolder.h"
#include "Utils.h"
#include "model/Model.h"

namespace nuvelocity
{
    AssetManager::AssetManager()
    {
        // FIXME: THIS SHOULD NOT BE HERE!
        ObjectRegistry::Get().Register(StandAloneFrame::GetClassInfo());
        ObjectRegistry::Get().Register(ModuleInfo::GetClassInfo());
        ObjectRegistry::Get().Register(PaletteHolder::GetClassInfo());
    }

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
        auto* frame = new StandAloneFrame();
        LoadStandAloneFrameFromStream(frame, stream);
        SDL_CloseIO(stream);
        return frame;
    }

    bool AssetManager::LoadStandAloneFrameFromStream(StandAloneFrame* frame, SDL_IOStream* stream)
    {
        int32_t hotSpotX;
        int32_t hotSpotY;
        uint8_t isCompressed;

        SDL_ReadS32LE(stream, &hotSpotX);
        SDL_ReadS32LE(stream, &hotSpotY);
        SDL_ReadU8(stream, &isCompressed);

        uint32_t deflatedSize = 0;
        uint32_t inflatedSize = 0;
        int width = 0;
        int height = 0;

        // Case 1: Compressed image data
        if (isCompressed != 0U)
        {
            // Case 1.1: Deflate-compressed image data, 4-plane/32-bit-plane RGBA
            if (FrameHasDeflateHeader(stream))
            {
                // Ignore byte marker for packed data size.
                SDL_ReadU8(stream, nullptr);
                SDL_ReadU32LE(stream, &deflatedSize);
                SDL_ReadU32LE(stream, &inflatedSize);

                auto* sourceImageData = static_cast<uint8_t*>(SDL_malloc(deflatedSize));
                if (SDL_ReadIO(stream, sourceImageData, deflatedSize) != deflatedSize)
                {
                    SDL_free(sourceImageData);
                    return false;
                }

                auto* imageData = static_cast<uint8_t*>(SDL_malloc(inflatedSize));
                if (Inflate(imageData, &inflatedSize, sourceImageData, &deflatedSize) != Z_OK)
                {
                    SDL_free(sourceImageData);
                    SDL_free(imageData);
                    return false;
                }
                SDL_free(sourceImageData);

                SDL_ReadS32LE(stream, &width);
                SDL_ReadS32LE(stream, &height);

                // Create SDL surface from decompressed image data
                frame->mSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
                if (frame->mSurface == nullptr)
                {
                    SDL_free(imageData);
                    return false;
                }

                // Process 4 planes of RGBA data with row offset addition
                for (int plane = 0; plane < 4; plane++)
                {
                    MergeBitPlane(plane, plane, width, height, imageData, frame->mSurface);
                }

                SDL_free(imageData);
            }
            // Case 1.2: Packed image data, 32-bit RGBA8888
            else
            {
                SDL_ReadU32LE(stream, &inflatedSize);
                auto* imageData = static_cast<uint8_t*>(SDL_malloc(inflatedSize));
                if (SDL_ReadIO(stream, imageData, inflatedSize) != inflatedSize)
                {
                    SDL_free(imageData);
                    return false;
                }

                SDL_ReadS32LE(stream, &width);
                SDL_ReadS32LE(stream, &height);

                frame->mSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
                if (frame->mSurface == nullptr)
                {
                    SDL_free(imageData);
                    return false;
                }

                SDL_memcpy(frame->mSurface->pixels, imageData, inflatedSize);
                SDL_free(imageData);
            }
        }
        else
        {
            // Case 2: JPEG image data
            uint32_t imageSize = 0;
            SDL_ReadU32LE(stream, &imageSize);

            void* imageData = SDL_malloc(imageSize);
            if (SDL_ReadIO(stream, imageData, imageSize) != imageSize)
            {
                SDL_free(imageData);
                return false;
            }

            SDL_IOStream* imageStream = SDL_IOFromConstMem(imageData, imageSize);
            frame->mSurface = IMG_LoadJPG_IO(imageStream);
            SDL_free(imageData);
            SDL_CloseIO(imageStream);

            if (SDL_ReadIO(stream, nullptr, 0) == 0 && SDL_GetIOStatus(stream) != SDL_IO_STATUS_EOF)
            {
                // The original surface does not have an alpha channel.
                SDL_Surface* output = SDL_CreateSurface(frame->mSurface->w, frame->mSurface->h,
                                                        SDL_PIXELFORMAT_RGBA32);
                if (output == nullptr)
                {
                    return false;
                }

                // Ignore padding byte.
                SDL_ReadU8(stream, nullptr);
                SDL_ReadU32LE(stream, &inflatedSize);
                deflatedSize = SDL_GetIOSize(stream) - SDL_TellIO(stream);

                auto* sourceMaskData = static_cast<uint8_t*>(SDL_malloc(deflatedSize));
                if (SDL_ReadIO(stream, sourceMaskData, deflatedSize) != deflatedSize)
                {
                    SDL_free(sourceMaskData);
                    return false;
                }

                auto* maskData = static_cast<uint8_t*>(SDL_malloc(inflatedSize));
                if (Inflate(maskData, &inflatedSize, sourceMaskData, &deflatedSize) != Z_OK)
                {
                    return false;
                }

                SDL_BlitSurface(frame->mSurface, nullptr, output, nullptr);
                SDL_DestroySurface(frame->mSurface);
                frame->mSurface = output;

                MergeBitPlane(0, 3, frame->mSurface->w, frame->mSurface->h, maskData,
                              frame->mSurface);

                SDL_free(maskData);
            }
        }

        return true;
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
} // namespace nuvelocity
