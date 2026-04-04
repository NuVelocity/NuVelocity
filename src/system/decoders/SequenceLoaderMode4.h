#ifndef NVE_SEQUENCELOADERMODE4_H
#define NVE_SEQUENCELOADERMODE4_H

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>

#include "Sequence.h"

namespace nuvelocity
{
    class SequenceLoaderMode4
    {
    public:
        static Sequence* Load(SDL_IOStream* stream);

    private:
        static bool DecodeSequenceHDHeader(SDL_IOStream* stream,
                                           uint8_t*& listData,
                                           size_t& listDataSize,
                                           uint8_t*& imageData,
                                           size_t& imageDataSize,
                                           bool& isEmpty,
                                           int& atlasWidth,
                                           int& atlasHeight);
        static SDL_Surface* BuildSequenceAtlasSurface(int atlasWidth,
                                                      int atlasHeight,
                                                      const uint8_t* imageData,
                                                      size_t imageDataSize);
        static SDL_Surface* BuildInterleavedRgbaAtlasSurface(int width,
                                                             int height,
                                                             const uint8_t* imageData,
                                                             size_t imageDataSize);
        static SDL_Surface* TryLoadDdsSurface(const uint8_t* imageData, size_t imageDataSize);
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCELOADERMODE4_H
