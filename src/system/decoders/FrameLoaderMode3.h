#ifndef NVE_FRAMELOADERMODE3_H
#define NVE_FRAMELOADERMODE3_H

#include <SDL3/SDL.h>

#include "StandAloneFrame.h"

namespace nuvelocity
{
    class FrameLoaderMode3
    {
    public:
        static StandAloneFrame* Load(SDL_IOStream* stream);

    private:
        static bool LoadFromStream(StandAloneFrame* frame, SDL_IOStream* stream);
        static bool LoadCompressedDeflateFrame(StandAloneFrame* frame, SDL_IOStream* stream);
        static bool LoadCompressedPackedFrame(StandAloneFrame* frame, SDL_IOStream* stream);
        static bool LoadJpegFrame(StandAloneFrame* frame, SDL_IOStream* stream);
    };
} // namespace nuvelocity

#endif // NVE_FRAMELOADERMODE3_H
