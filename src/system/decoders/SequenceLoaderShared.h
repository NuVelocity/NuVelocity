#ifndef NVE_SEQUENCELOADERSHARED_H
#define NVE_SEQUENCELOADERSHARED_H

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace nuvelocity
{
    class Sequence;
    class SequenceFrameInfoList;

    class SequenceLoaderShared
    {
    public:
        static void FreeDecodedBuffers(uint8_t*& listData,
                                       size_t& listDataSize,
                                       uint8_t*& imageData,
                                       size_t& imageDataSize,
                                       uint8_t*& alphaChannelData,
                                       size_t& alphaChannelDataSize);
        static bool DeserializeSequenceRoots(const std::string& listText,
                                             Sequence*& sequence,
                                             SequenceFrameInfoList*& frameInfoList);
        static bool BuildFramesFromAtlas(Sequence* sequence,
                                         SequenceFrameInfoList* frameInfoList,
                                         SDL_Surface* spriteAtlas);
        static SDL_Surface* BuildTransparentSurface(int width, int height);
    };
} // namespace nuvelocity

#endif // NVE_SEQUENCELOADERSHARED_H
