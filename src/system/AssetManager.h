#ifndef NVE_ASSETMANAGER_H
#define NVE_ASSETMANAGER_H

#include "API.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <string>
#include <zlib.h>

#include "StandAloneFrame.h"
#include "Manager.h"

namespace nuvelocity
{
    class AssetManager : public Manager
    {
    private:
        enum PixelFormat
        {
            PIXEL_FORMAT_UNKNOWN = 0,
            PIXEL_FORMAT_RGB332 = 0x08,
            PIXEL_FORMAT_RGB565 = 0x10,
            PIXEL_FORMAT_RGB888 = 0x18,
            PIXEL_FORMAT_RGBA8888 = 0x20
        };

        enum CacheKind
        {
            CACHE_KIND_UNKNOWN = 0,
            CACHE_KIND_STANDALONE_FRAME = 1,
            CACHE_KIND_SEQUENCE = 2
        };

        static constexpr int kDeflateHeaderLength = 2;
        static constexpr int kOffsetToDeflateHeader = 9;
        static constexpr int kOffsetFromDeflateHeader =
            -(kOffsetToDeflateHeader + kDeflateHeaderLength);
        static constexpr int kFontOffsetToDeflateHeader =
            12 + kOffsetToDeflateHeader;
        static constexpr int kFontOffsetFromDeflateHeader =
            -(kFontOffsetToDeflateHeader + kDeflateHeaderLength);

        static SDL_IOStream* LoadWithExtension(const std::string& path, const std::string& extension) ;

        static SDL_IOStream* LoadFromCache(const std::string& path, CacheKind kind) ;

        static bool LoadStandAloneFrameFromStream(StandAloneFrame* frame, SDL_IOStream* stream);

        static inline const char* GetErrorMessage() ;

        static inline bool FontFrameHasDeflateHeader(SDL_IOStream* aStream)
        {
            SDL_SeekIO(aStream, kFontOffsetToDeflateHeader, SDL_IO_SEEK_CUR);
            uint16_t header;
            SDL_ReadU16BE(aStream, &header);
            SDL_SeekIO(aStream, kFontOffsetFromDeflateHeader, SDL_IO_SEEK_CUR);
            return IsDeflateHeader(header);
        }

        static inline bool FrameHasDeflateHeader(SDL_IOStream* aStream)
        {
            SDL_SeekIO(aStream, kOffsetToDeflateHeader, SDL_IO_SEEK_CUR);
            uint16_t header;
            SDL_ReadU16BE(aStream, &header);
            SDL_SeekIO(aStream, kOffsetFromDeflateHeader, SDL_IO_SEEK_CUR);
            return IsDeflateHeader(header);
        }

        static inline bool IsDeflateHeader(uint16_t aHeader)
        {
            return aHeader % 0x1F == 0 &&
                (aHeader & 0x0f00) == (8 << 8);
        }

        static inline int Inflate(
            uint8_t *dest,
            uint32_t *destLen,
            const uint8_t *source,
            uint32_t *sourceLen)
        {
            uLongf destLen64 = *destLen;
            uLongf sourceLen64 = *sourceLen;
            return uncompress2(dest, &destLen64, source, &sourceLen64);
        }

        static inline void MergeBitPlane(
            int plane,
            int channel,
            uint32_t width,
            uint32_t height,
            uint8_t* planeData,
            SDL_Surface* surface)
        {
            int rawIndex = plane * width * height;
            for (int y = 0; y < surface->h; y++)
            {
                for (int x = 0; x < surface->w; x++)
                {
                    uint8_t *pixel = (uint8_t *)surface->pixels + y * surface->pitch + x * 4;
                    uint8_t component = planeData[rawIndex++];

                    if (x != 0 || y != 0)
                    {
                        // Add previous pixel's alpha in current row
                        uint8_t *prevPixel = pixel - 4;
                        component += prevPixel[channel];
                    }

                    pixel[channel] = component;
                }
            }
        }
    public:
        NVE_API AssetManager();
        NVE_API ~AssetManager();

        NVE_API bool Initialize(char** argv) override;

        NVE_API static bool Exists(const std::string& path) ;

        NVE_API static SDL_IOStream* Load(const char* path) ;

        NVE_API static SDL_IOStream* Load(const std::string& path) ;

        NVE_API static StandAloneFrame* LoadStandAloneFrame(const std::string& path);

        NVE_API static std::string LoadTextFile(const std::string& path) ;

        NVE_API static void DumpPropertyFile(const std::string& path) ;
    };
} // namespace nuvelocity

#endif // NVE_ASSETMANAGER_H