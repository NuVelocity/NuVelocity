#ifndef NVE_SEQUENCELOADERMODE3_H
#define NVE_SEQUENCELOADERMODE3_H

#include <SDL3/SDL.h>
#include <vector>

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
        static bool DecodeSequenceStandardHeader(SDL_IOStream* stream, std::vector<uint8_t>& listData,
                                                 std::vector<uint8_t>& imageData,
                                                 std::vector<uint8_t>& alphaChannelData,
                                                 bool& isCompressed, bool& isEmpty,
                                                 int& atlasWidth, int& atlasHeight);
        static bool DecodeSequenceHDHeader(SDL_IOStream* stream, std::vector<uint8_t>& listData,
                                           std::vector<uint8_t>& imageData, bool& isEmpty,
                                           int& atlasWidth, int& atlasHeight);
        static SDL_Surface* BuildSequenceAtlasSurface(bool isHD, bool isCompressed,
                                                      int atlasWidth, int atlasHeight,
                                                      const std::vector<uint8_t>& imageData,
                                                      const std::vector<uint8_t>& alphaChannelData);
        static SDL_Surface* BuildInterleavedRgbaAtlasSurface(int width, int height,
                                                             const std::vector<uint8_t>& imageData);
        static SDL_Surface* TryLoadDdsSurface(const std::vector<uint8_t>& imageData);
        static SDL_Surface* BuildPlanarRgbaAtlasSurface(int width, int height,
                                                        const std::vector<uint8_t>& imageData);
        static SDL_Surface* BuildJpegAtlasSurface(const std::vector<uint8_t>& imageData,
                                                  const std::vector<uint8_t>& alphaChannelData);
        static SDL_Surface* BuildSequenceFrameSurface(SDL_Surface* atlas, const FrameInfo* info);
        static SDL_Surface* BuildOffsetSurface(SDL_Surface* source, int offsetX, int offsetY);
        static SDL_Surface* BuildTransparentSurface(int width, int height);
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCELOADERMODE3_H
