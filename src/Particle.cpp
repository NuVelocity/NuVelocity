#include "Particle.h"
#include <Game.h>
#include <Sequence.h>
#include <cmath>
#include <system/SpriteBatch.h>

namespace nuvelocity
{
    Particle::Particle(nuvelocity::Sequence* seq, SDL_FPoint pos, SDL_FPoint vel, float lifeTime)
            : mSequence(seq)
            , mPosition(pos)
            , mVelocity(vel)
            , mLife(lifeTime)
            , mMaxLife(lifeTime)
    {
        mStartTick = SDL_GetTicks();
    }

    void Particle::Update(Game* game)
    {
        float dt = game->GetDeltaTime();
        mLife -= dt;

        if (mLife > 0.0F)
        {
            mPosition.x += mVelocity.x * dt;
            mPosition.y += mVelocity.y * dt;
        }
    }

    void Particle::Draw(Game* game)
    {
        if (mSequence == nullptr || IsDead())
        {
            return;
        }

        const uint64_t elapsed = SDL_GetTicks() - mStartTick;
        const float fps = mSequence->GetFramesPerSecond();
        const std::size_t frameCount = mSequence->GetFrameCount();

        if (frameCount == 0)
            return;

        const std::size_t frameIndex =
            static_cast<std::size_t>((static_cast<double>(elapsed) * fps) / 1000.0) % frameCount;

        nuvelocity::Frame* frame = mSequence->GetFrame(frameIndex);
        if (frame != nullptr)
        {
            SDL_Surface* surface = frame->GetSurface();
            SDL_Rect destRect = {
                .x = static_cast<int>(std::lround(mPosition.x)) + frame->GetHotSpot().x,
                .y = static_cast<int>(std::lround(mPosition.y)) + frame->GetHotSpot().y,
                .w = surface->w,
                .h = surface->h};

            float alphaPercent = mLife / mMaxLife;
            uint8_t alpha = static_cast<uint8_t>(std::clamp(alphaPercent * 255.0F, 0.0F, 255.0F));
            game->mSpriteBatch->Draw(surface, &destRect, nullptr, {255, 255, 255, alpha});
        }
    }
} // namespace nuvelocity
