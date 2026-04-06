#ifndef NVE_DECODEUTILS_H
#define NVE_DECODEUTILS_H

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <zlib.h>

namespace nuvelocity
{
    class Sequence;
    class SequenceFrameInfoList;

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

        static inline int
        Inflate(uint8_t* dest, uint32_t* destLen, const uint8_t* source, uint32_t* sourceLen)
        {
            uLongf destLen64 = *destLen;
            uLongf sourceLen64 = *sourceLen;
            return uncompress2(dest, &destLen64, source, &sourceLen64);
        }

        static inline void MergeBitPlane(int plane,
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
                    uint8_t* pixel = (uint8_t*)surface->pixels + y * surface->pitch + x * 4;
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

        static bool ReadChunk(SDL_IOStream* stream, size_t size, uint8_t*& chunkData);
        static bool InflateChunk(const uint8_t* compressedChunk,
                                 uint32_t compressedSize,
                                 uint32_t inflatedSize,
                                 uint8_t*& inflatedChunk,
                                 size_t& inflatedChunkSize);
        static bool ReadAndInflateChunk(SDL_IOStream* stream,
                                        uint32_t compressedSize,
                                        uint32_t inflatedSize,
                                        uint8_t*& output,
                                        size_t& outputSize);
        static void FreeDecodedBuffers(uint8_t*& listData,
                                       size_t& listDataSize,
                                       uint8_t*& imageData,
                                       size_t& imageDataSize,
                                       uint8_t*& alphaChannelData,
                                       size_t& alphaChannelDataSize);
        static bool DeserializeSequenceRoots(const std::string& listText,
                                             Sequence*& sequence,
                                             SequenceFrameInfoList*& frameInfoList);
        static bool ProcessSequenceListText(const std::string& listText,
                                            Sequence*& sequence,
                                            SequenceFrameInfoList*& frameInfoList,
                                            bool& hasFrameInfoList);
        static Sequence* FinalizeSequence(Sequence* sequence,
                                          SequenceFrameInfoList* frameInfoList,
                                          bool hasFrameInfoList,
                                          size_t imageDataSize,
                                          bool isEmpty,
                                          SDL_Surface* spriteAtlas);
        static bool BuildFramesFromAtlas(Sequence* sequence,
                                         SequenceFrameInfoList* frameInfoList,
                                         SDL_Surface* spriteAtlas);
        static SDL_Surface* BuildTransparentSurface(int width, int height);
    };
} // namespace nuvelocity

#endif // NVE_DECODEUTILS_H
