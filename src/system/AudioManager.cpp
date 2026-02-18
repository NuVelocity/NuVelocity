#include "AudioManager.h"

namespace nuvelocity
{
    AudioManager::AudioManager() = default;

    AudioManager::~AudioManager()
    {
        MIX_Quit();
    }

    bool AudioManager::Initialize(char** argv)
    {
        if (WarnIfAlreadyInitialized("AudioManager"))
        {
            return true;
        }

        if (!MIX_Init())
        {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE,
                "Failed to initialize SDL_mixer: %s",
                SDL_GetError());
            return false;
        }

        mMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (mMixer == nullptr) {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE,
                "Failed to create mixer device: %s",
                SDL_GetError());
            return false;
        }

        mBgmTrack = MIX_CreateTrack(mMixer);
        if (mBgmTrack == nullptr) {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE,
                "Failed to create BGM track: %s",
                SDL_GetError());
            return false;
        }

        mInitialized = true;
        return true;
    }

    bool AudioManager::AssignBgm(const std::string& aId, SDL_IOStream* aStream)
    { 
        auto *music = MIX_LoadAudio_IO(mMixer, aStream, false, true);
        if (music == nullptr) {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE,
                "Failed to load BGM track: %s",
                SDL_GetError());
            return false;
        }
        mBgmInputs[aId] = music;
        return true;
    }

    bool AudioManager::PlayBgm(const std::string& aId, int aFadeMS, int aLoops)
    {
        if (mCurrentBgm == aId)
        {
            return true;
        }
        mCurrentBgm = aId;
        MIX_SetTrackAudio(mBgmTrack, mBgmInputs[aId]);
        SDL_PropertiesID properties = SDL_CreateProperties();
        SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, aLoops);
        SDL_SetNumberProperty(properties, MIX_PROP_PLAY_FADE_IN_FRAMES_NUMBER,
            MIX_TrackMSToFrames(mBgmTrack, aFadeMS));
        return MIX_PlayTrack(mBgmTrack, properties);
    }
    
    bool AudioManager::StopBgm(const std::string& aId, int aFadeMS)
    {
        return MIX_StopTrack(mBgmTrack, MIX_TrackMSToFrames(mBgmTrack, aFadeMS));
    }

    bool AudioManager::AssignSfx(const std::string& aId, SDL_IOStream* aStream)
    {
        auto *track = MIX_CreateTrack(mMixer);
        auto *sfx = MIX_LoadAudio_IO(mMixer, aStream, false, true);
        if (sfx == nullptr) {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE,
                "Failed to load SFX track: %s",
                SDL_GetError());
            return false;
        }
        mSfxInputs[aId] = sfx;
        return true;
    }

    bool AudioManager::PlaySfx(const std::string& aId, int aFadeMS, int aLoops)
    {
        return MIX_PlayAudio(mMixer, mSfxInputs[aId]);
    }

} // namespace nuvelocity
