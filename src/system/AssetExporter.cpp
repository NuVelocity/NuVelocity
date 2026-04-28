#include "AssetExporter.h"

#include "Sequence.h"
#include "StandAloneFrame.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <utility>

#include "model/PropertySerializer.h"

namespace nuvelocity
{
    constexpr const char* kPropertiesFileName = "Properties.txt";
    constexpr const char* kTgaExtension = ".tga";
#ifdef NVE_RESTORE_TGA
    constexpr const char* kRawFrameInfoListFileName = "RawFrameInfoList.txt";
    constexpr const char* kAtlasFileName = "Atlas.tga";
#endif

    bool AssetExporter::WriteAll(PHYSFS_File* file, const void* data, size_t dataSize)
    {
        if (file == nullptr)
        {
            return false;
        }

        const PHYSFS_sint64 written =
            PHYSFS_writeBytes(file, data, static_cast<PHYSFS_uint64>(dataSize));
        return std::cmp_equal(written, dataSize);
    }

    bool AssetExporter::EnsureDirectoryTree(const std::string& directoryPath)
    {
        if (directoryPath.empty() || directoryPath == ".")
        {
            return true;
        }

        const char* writeDir = PHYSFS_getWriteDir();
        if (writeDir == nullptr || *writeDir == '\0')
        {
            return false;
        }

        std::string normalized = directoryPath;
        for (char& character : normalized)
        {
            if (character == '\\')
            {
                character = '/';
            }
        }

        if (!normalized.empty() && normalized.back() == '/')
        {
            normalized.pop_back();
        }

        std::string current;
        std::stringstream parts(normalized);
        std::string token;
        while (std::getline(parts, token, '/'))
        {
            if (token.empty() || token == ".")
            {
                continue;
            }

            if (!current.empty())
            {
                current += '/';
            }
            current += token;

            PHYSFS_Stat currentStat{};
            const int statResult = PHYSFS_stat(current.c_str(), &currentStat);
            if (statResult != 0)
            {
                if (currentStat.filetype != PHYSFS_FILETYPE_DIRECTORY)
                {
                    return false;
                }

                const char* realDir = PHYSFS_getRealDir(current.c_str());
                if (realDir != nullptr && std::string_view(realDir) == writeDir)
                {
                    continue;
                }

                if (PHYSFS_mkdir(current.c_str()) == 0)
                {
                    return false;
                }
                continue;
            }

            if (PHYSFS_mkdir(current.c_str()) == 0)
            {
                return false;
            }
        }

        return true;
    }

    bool AssetExporter::WriteBinaryFile(const std::string& virtualPath,
                                        const void* data,
                                        size_t dataSize)
    {
        const std::filesystem::path path(virtualPath);
        if (!EnsureDirectoryTree(path.parent_path().generic_string()))
        {
            return false;
        }

        PHYSFS_File* file = PHYSFS_openWrite(path.generic_string().c_str());
        if (file == nullptr)
        {
            return false;
        }

        const bool ok = WriteAll(file, data, dataSize);
        PHYSFS_close(file);
        return ok;
    }

    bool AssetExporter::SaveSurfaceAsUncompressedTga(SDL_Surface* sourceSurface,
                                                     const std::string& virtualPath)
    {
        if (sourceSurface == nullptr || sourceSurface->w <= 0 || sourceSurface->h <= 0)
        {
            return false;
        }

        SDL_Surface* rgbaSurface = SDL_ConvertSurface(sourceSurface, SDL_PIXELFORMAT_RGBA32);
        if (rgbaSurface == nullptr)
        {
            return false;
        }

        std::array<uint8_t, 18> header{};
        header[2] = 2;
        header[12] = static_cast<uint8_t>(rgbaSurface->w & 0xFF);
        header[13] = static_cast<uint8_t>((rgbaSurface->w >> 8) & 0xFF);
        header[14] = static_cast<uint8_t>(rgbaSurface->h & 0xFF);
        header[15] = static_cast<uint8_t>((rgbaSurface->h >> 8) & 0xFF);
        header[16] = 32;
        header[17] = 8 | 0x20;

        const size_t rowBytes = static_cast<size_t>(rgbaSurface->w) * 4U;
        std::vector<uint8_t> fileData;
        fileData.resize(header.size() + (rowBytes * static_cast<size_t>(rgbaSurface->h)));
        std::ranges::copy(header, fileData.begin());

        const uint8_t* pixels = static_cast<const uint8_t*>(rgbaSurface->pixels);
        size_t writeOffset = header.size();
        for (int rowIndex = 0; rowIndex < rgbaSurface->h; ++rowIndex)
        {
            const uint8_t* rgbaRow = pixels + (static_cast<size_t>(rowIndex) * rgbaSurface->pitch);
            for (int columnIndex = 0; columnIndex < rgbaSurface->w; ++columnIndex)
            {
                const size_t sourceOffset = static_cast<size_t>(columnIndex) * 4U;
                fileData[writeOffset + sourceOffset + 0U] = rgbaRow[sourceOffset + 2U];
                fileData[writeOffset + sourceOffset + 1U] = rgbaRow[sourceOffset + 1U];
                fileData[writeOffset + sourceOffset + 2U] = rgbaRow[sourceOffset + 0U];
                fileData[writeOffset + sourceOffset + 3U] = rgbaRow[sourceOffset + 3U];
            }
            writeOffset += rowBytes;
        }

        SDL_DestroySurface(rgbaSurface);
        return WriteBinaryFile(virtualPath, fileData.data(), fileData.size());
    }

    bool AssetExporter::ExportStandAloneFrameToTga(const std::string& path,
                                                   const StandAloneFrame& frame)
    {
        const bool tgaExported =
            SaveSurfaceAsUncompressedTga(frame.GetSurface(), path + kTgaExtension);

        // Dump hotspot to file (path + ".hotspot.txt")
        const auto& hotspot = frame.GetHotSpot();
        std::ostringstream hotspotStream;
        hotspotStream << "// StandAloneFrame::mHotSpot\n";
        hotspotStream << "x: " << hotspot.x << "\n";
        hotspotStream << "y: " << hotspot.y << "\n";
        const std::string hotspotStr = hotspotStream.str();
        const bool hotspotExported =
            WriteBinaryFile(path + ".hotspot.txt", hotspotStr.data(), hotspotStr.size());

        return tgaExported && hotspotExported;
    }

    bool AssetExporter::ExportSequenceToTga(const std::string& path, const Sequence& sequence)
    {
        const std::filesystem::path sequenceAssetPath(path);
        std::string baseName = sequenceAssetPath.filename().string();
        if (baseName.empty())
        {
            return false;
        }

        if (baseName[0] != '-')
        {
            baseName.insert(baseName.begin(), '-');
        }

        const std::filesystem::path sequenceDirectoryPath =
            sequenceAssetPath.parent_path() / baseName;
        const std::string framePrefix = GetSequenceFramePrefix(sequenceDirectoryPath);
        if (framePrefix.empty())
        {
            return false;
        }

        std::string serializedSequence;
        PropertySerializer::Serialize(sequence, serializedSequence);
        if (!WriteBinaryFile((sequenceDirectoryPath / kPropertiesFileName).generic_string(),
                             serializedSequence.data(),
                             serializedSequence.size()))
        {
            return false;
        }

        const std::size_t frameCount = sequence.GetFrameCount();
        for (std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex)
        {
            SDL_Surface* surface = sequence.GetSurface(frameIndex);
            if (surface == nullptr)
            {
                return false;
            }

            std::ostringstream frameName;
            frameName << framePrefix << std::setw(4) << std::setfill('0') << frameIndex
                      << kTgaExtension;
            if (!SaveSurfaceAsUncompressedTga(
                    surface, (sequenceDirectoryPath / frameName.str()).generic_string()))
            {
                return false;
            }
        }

#ifdef NVE_RESTORE_TGA
        const std::string& rawListText = sequence.GetRawListText();
        if (!rawListText.empty())
        {
            if (!WriteBinaryFile(
                    (sequenceDirectoryPath / kRawFrameInfoListFileName).generic_string(),
                    rawListText.data(),
                    rawListText.size()))
            {
                return false;
            }
        }

        SDL_Surface* spriteAtlas = sequence.GetSpriteAtlas();
        if (spriteAtlas != nullptr)
        {
            if (!SaveSurfaceAsUncompressedTga(
                    spriteAtlas, (sequenceDirectoryPath / kAtlasFileName).generic_string()))
            {
                return false;
            }
        }
#endif

        return true;
    }
} // namespace nuvelocity
