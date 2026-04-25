#ifndef NVE_ASSET_SOURCE_H
#define NVE_ASSET_SOURCE_H

#include <cstdint>

namespace nuvelocity
{
    enum class AssetSource : uint8_t
    {
        Unknown,
        Cache,
        SourceAsset
    };
} // namespace nuvelocity

#endif // NVE_ASSET_SOURCE_H
