#ifndef NVE_PARTICLE_GENERATOR_H
#define NVE_PARTICLE_GENERATOR_H

#include "API.h"
#include "EmitterType.h"
#include "Particle.h"
#include <GameComponent.h>
#include <memory>
#include <vector>

namespace nuvelocity
{
    class ParticleGeneratorInfo;
    class ParticleType;

    class ParticleGenerator : public GameComponent
    {
    public:
        NVE_API ParticleGenerator();
        NVE_API virtual ~ParticleGenerator() = default;

        NVE_API void Update(Game* game) override;
        NVE_API void Draw(Game* game) override;

        NVE_API void Start(const SDL_FPoint& pos,
                           const nuvelocity::ParticleGeneratorInfo* info,
                           EmitterType type,
                           EmitterShape shape = EmitterShape::Cone,
                           float rateOrInterval = 0.0F,
                           float duration = 0.0F,
                           float width = 0.0F,
                           float height = 0.0F,
                           const std::vector<ParticleType*>* customTypes = nullptr,
                           float baseAngle = 0.0F,
                           float posVariation = 0.0F,
                           float lifeMultiplier = 1.0F,
                           bool doFadeOut = false);

        NVE_API void Burst(const SDL_FPoint& pos,
                           const nuvelocity::ParticleGeneratorInfo* info,
                           const std::vector<ParticleType*>* customTypes = nullptr,
                           float baseAngle = 0.0F,
                           float posVariation = 0.0F,
                           float lifeMultiplier = 1.0F,
                           bool doFadeOut = false);

        NVE_API void Stop()
        {
            mIsActive = false;
        }

        NVE_API void SetPosition(const SDL_FPoint& pos)
        {
            mPosition = pos;
        }

        NVE_API bool IsFinished() const
        {
            return !mIsActive && mParticles.empty();
        }

    private:
        void Emit(int count);

        std::vector<std::unique_ptr<Particle>> mParticles;
        const nuvelocity::ParticleGeneratorInfo* mInfo = nullptr;
        std::vector<ParticleType*> mCustomTypes;
        SDL_FPoint mPosition = {0, 0};
        EmitterType mType = EmitterType::Burst;
        EmitterShape mShape = EmitterShape::Cone;
        float mRateOrInterval = 0.0F;
        float mDuration = 0.0F;
        float mWidth = 0.0F;
        float mHeight = 0.0F;
        float mBaseAngle = 0.0F;
        float mPosVariation = 0.0F;
        float mLifeMultiplier = 1.0F;
        float mEmissionTimer = 0.0F;
        float mPulseTimer = 0.0F;
        float mTotalElapsed = 0.0F;
        bool mIsActive = false;
        bool mDoFadeOut = false;
    };
} // namespace nuvelocity

#endif // NVE_PARTICLE_GENERATOR_H
