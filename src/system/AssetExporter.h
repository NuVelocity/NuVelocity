#ifndef NVE_ASSET_EXPORTER_H
#define NVE_ASSET_EXPORTER_H

#include "API.h"
#include "Sequence.h"
#include "StandAloneFrame.h"

#include <SDL3/SDL.h>
#include <cstddef>
#include <filesystem>
#include <physfs.h>
#include <string>

namespace nuvelocity
{
    class AssetExporter
    {
    public:
        NVE_API static bool ExportStandAloneFrameToTga(const std::string& path,
                                                       const StandAloneFrame& frame);
        NVE_API static bool ExportSequenceToTga(const std::string& path, const Sequence& sequence);

    private:
        static bool WriteAll(PHYSFS_File* file, const void* data, size_t dataSize);
        static bool EnsureDirectoryTree(const std::string& directoryPath);
        static bool
        WriteBinaryFile(const std::string& virtualPath, const void* data, size_t dataSize);
        static bool SaveSurfaceAsUncompressedTga(SDL_Surface* sourceSurface,
                                                 const std::string& virtualPath);

        static inline std::string
        GetSequenceFramePrefix(const std::filesystem::path& sequenceDirectoryPath)
        {
            std::string framePrefix = sequenceDirectoryPath.filename().string();
            if (!framePrefix.empty() && framePrefix[0] == '-')
            {
                framePrefix.erase(0, 1);
            }
            return framePrefix;
        }
    };
} // namespace nuvelocity

#endif // NVE_ASSET_EXPORTER_H
