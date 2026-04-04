#ifndef NVE_SEQUENCELOADERMODE3_H
#define NVE_SEQUENCELOADERMODE3_H

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>

#include "Sequence.h"

namespace nuvelocity
{
    class SequenceLoaderMode3
    {
    public:
        static Sequence* Load(SDL_IOStream* stream);

    private:
        static bool DecodeSequenceStandardHeader(SDL_IOStream* stream,
                                                 uint8_t*& listData,
                                                 size_t& listDataSize,
                                                 uint8_t*& imageData,
                                                 size_t& imageDataSize,
                                                 uint8_t*& alphaChannelData,
                                                 size_t& alphaChannelDataSize,
                                                 bool& isCompressed,
                                                 bool& isEmpty,
                                                 int& atlasWidth,
                                                 int& atlasHeight);
        static SDL_Surface* BuildSequenceAtlasSurface(bool isCompressed,
                                                      int atlasWidth,
                                                      int atlasHeight,
                                                      const uint8_t* imageData,
                                                      size_t imageDataSize,
                                                      const uint8_t* alphaChannelData,
                                                      size_t alphaChannelDataSize);
        static SDL_Surface* BuildPlanarRgbaAtlasSurface(int width,
                                                        int height,
                                                        const uint8_t* imageData,
                                                        size_t imageDataSize);
        static SDL_Surface* BuildJpegAtlasSurface(const uint8_t* imageData,
                                                  size_t imageDataSize,
                                                  const uint8_t* alphaChannelData,
                                                  size_t alphaChannelDataSize);
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCELOADERMODE3_H
