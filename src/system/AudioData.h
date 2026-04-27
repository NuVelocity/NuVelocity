#ifndef NVE_AUDIODATA_H
#define NVE_AUDIODATA_H

#include <SDL3/SDL.h>
#include <string>

namespace nuvelocity
{
    class AudioData
    {
    public:
        ~AudioData()
        {
            if (stream != nullptr)
            {
                SDL_CloseIO(stream);
            }
        }

        std::string path;
        SDL_IOStream* stream;
    };
} // namespace nuvelocity

#endif // NVE_AUDIODATA_H
