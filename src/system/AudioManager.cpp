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
            SDL_LogError(
                NVE_LOG_CATEGORY_ENGINE, "Failed to initialize SDL_mixer: %s", SDL_GetError());
            return false;
        }

        mMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (mMixer == nullptr)
        {
            SDL_LogError(
                NVE_LOG_CATEGORY_ENGINE, "Failed to create mixer device: %s", SDL_GetError());
            return false;
        }

        mBgmTrack = MIX_CreateTrack(mMixer);
        if (mBgmTrack == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE, "Failed to create BGM track: %s", SDL_GetError());
            return false;
        }

        mInitialized = true;
        return true;
    }

    bool AudioManager::RegisterBgm(AudioData* data)
    {
        if (mBgmInputs[data->path] != nullptr)
        {
            return true;
        }
        auto* music = MIX_LoadAudio_IO(mMixer, data->stream, false, true);
        data->stream = nullptr;
        if (music == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE, "Failed to load BGM track: %s", SDL_GetError());
            return false;
        }
        mBgmInputs[data->path] = music;
        return true;
    }

    bool AudioManager::PlayBgm(const std::string& id, int fadeMs, int loops)
    {
        if (mCurrentBgm == id)
        {
            return true;
        }
        mCurrentBgm = id;
        MIX_SetTrackAudio(mBgmTrack, mBgmInputs[id]);
        SDL_PropertiesID properties = SDL_CreateProperties();
        SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        SDL_SetNumberProperty(properties,
                              MIX_PROP_PLAY_FADE_IN_FRAMES_NUMBER,
                              MIX_TrackMSToFrames(mBgmTrack, fadeMs));
        return MIX_PlayTrack(mBgmTrack, properties);
    }

    bool AudioManager::StopBgm(const std::string& id, int fadeMs)
    {
        return MIX_StopTrack(mBgmTrack, MIX_TrackMSToFrames(mBgmTrack, fadeMs));
    }

    bool AudioManager::RegisterSfx(AudioData* data)
    {
        if (mSfxInputs[data->path] != nullptr)
        {
            return true;
        }
        auto* track = MIX_CreateTrack(mMixer);
        auto* sfx = MIX_LoadAudio_IO(mMixer, data->stream, false, true);
        data->stream = nullptr;
        if (sfx == nullptr)
        {
            SDL_LogError(NVE_LOG_CATEGORY_ENGINE, "Failed to load SFX track: %s", SDL_GetError());
            return false;
        }
        mSfxInputs[data->path] = sfx;
        return true;
    }

    bool AudioManager::PlaySfx(const std::string& id, int fadeMs, int loops)
    {
        return MIX_PlayAudio(mMixer, mSfxInputs[id]);
    }

} // namespace nuvelocity
