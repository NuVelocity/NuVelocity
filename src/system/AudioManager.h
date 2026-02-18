#ifndef NVE_AUDIOMANAGER_H
#define NVE_AUDIOMANAGER_H

#include "API.h"
#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "Manager.h"

namespace nuvelocity
{
    class AudioManager : public Manager
    {
    private:
        MIX_Mixer* mMixer;
        std::unordered_map<std::string, MIX_Audio*> mBgmInputs;
        std::unordered_map<std::string, MIX_Audio*> mSfxInputs;

        std::string mCurrentBgm;
        MIX_Track* mBgmTrack;

    public:
        NVE_API AudioManager();
        NVE_API ~AudioManager();

        NVE_API bool Initialize(char** argv) override;

        NVE_API bool AssignBgm(const std::string& aId, SDL_IOStream* aStream);
        NVE_API bool PlayBgm(const std::string& aId, int aFadeMS = 0, int aLoops = -1);
        NVE_API bool StopBgm(const std::string& aId, int aFadeMS = 0);

        NVE_API bool AssignSfx(const std::string& aId, SDL_IOStream* aStream);
        NVE_API bool PlaySfx(const std::string& aId, int aFadeMS = 0, int aLoops = 0);
    };
} // namespace nuvelocity

#endif // NVE_AUDIOMANAGER_H
