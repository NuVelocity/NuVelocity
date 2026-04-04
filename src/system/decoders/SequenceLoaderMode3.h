#ifndef NVE_SEQUENCELOADERMODE3_H
#define NVE_SEQUENCELOADERMODE3_H

#include <SDL3/SDL.h>

#include "Sequence.h"

namespace nuvelocity
{
    class SequenceFrameInfoList;
    class FrameInfo;

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
        static bool DecodeSequenceHDHeader(SDL_IOStream* stream,
                                           uint8_t*& listData,
                                           size_t& listDataSize,
                                           uint8_t*& imageData,
                                           size_t& imageDataSize,
                                           bool& isEmpty,
                                           int& atlasWidth,
                                           int& atlasHeight);
        static SDL_Surface* BuildSequenceAtlasSurface(bool isHD,
                                                      bool isCompressed,
                                                      int atlasWidth,
                                                      int atlasHeight,
                                                      const uint8_t* imageData,
                                                      size_t imageDataSize,
                                                      const uint8_t* alphaChannelData,
                                                      size_t alphaChannelDataSize);
        static SDL_Surface* BuildInterleavedRgbaAtlasSurface(int width,
                                                             int height,
                                                             const uint8_t* imageData,
                                                             size_t imageDataSize);
        static SDL_Surface* TryLoadDdsSurface(const uint8_t* imageData, size_t imageDataSize);
        static SDL_Surface* BuildPlanarRgbaAtlasSurface(int width,
                                                        int height,
                                                        const uint8_t* imageData,
                                                        size_t imageDataSize);
        static SDL_Surface* BuildJpegAtlasSurface(const uint8_t* imageData,
                                                  size_t imageDataSize,
                                                  const uint8_t* alphaChannelData,
                                                  size_t alphaChannelDataSize);
        static SDL_Surface* BuildSequenceFrameSurface(SDL_Surface* atlas, const FrameInfo* info);
        static SDL_Surface* BuildOffsetSurface(SDL_Surface* source, int offsetX, int offsetY);
        static SDL_Surface* BuildTransparentSurface(int width, int height);
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCELOADERMODE3_H
