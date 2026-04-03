#ifndef NVE_DECODEUTILS_H
#define NVE_DECODEUTILS_H

#include <SDL3/SDL.h>
#include <cstdint>
#include <zlib.h>

namespace nuvelocity
{
    class DecodeUtils
    {
    public:
        static constexpr int kDeflateHeaderLength = 2;
        static constexpr int kOffsetToDeflateHeader = 9;
        static constexpr int kOffsetFromDeflateHeader =
            -(kOffsetToDeflateHeader + kDeflateHeaderLength);
        static constexpr int kFontOffsetToDeflateHeader = 12 + kOffsetToDeflateHeader;
        static constexpr int kFontOffsetFromDeflateHeader =
            -(kFontOffsetToDeflateHeader + kDeflateHeaderLength);

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
            return aHeader % 0x1F == 0 && (aHeader & 0x0f00) == (8 << 8);
        }

        static inline int Inflate(uint8_t* dest, uint32_t* destLen, const uint8_t* source,
                                  uint32_t* sourceLen)
        {
            uLongf destLen64 = *destLen;
            uLongf sourceLen64 = *sourceLen;
            return uncompress2(dest, &destLen64, source, &sourceLen64);
        }

        static inline void MergeBitPlane(int plane, int channel, uint32_t width, uint32_t height,
                                         uint8_t* planeData, SDL_Surface* surface)
        {
            int rawIndex = plane * width * height;
            for (int y = 0; y < surface->h; y++)
            {
                for (int x = 0; x < surface->w; x++)
                {
                    uint8_t* pixel = (uint8_t*) surface->pixels + y * surface->pitch + x * 4;
                    uint8_t component = planeData[rawIndex++];

                    if (x != 0 || y != 0)
                    {
                        // Add previous pixel's alpha in current row
                        uint8_t* prevPixel = pixel - 4;
                        component += prevPixel[channel];
                    }

                    pixel[channel] = component;
                }
            }
        }
    };
} // namespace nuvelocity

#endif // NVE_DECODEUTILS_H
