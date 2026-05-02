#include "ParticleGenerator.h"
#include "ParticleGeneratorInfo.h"
#include "ParticleType.h"
#include <Game.h>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

namespace nuvelocity
{
    ParticleGenerator::ParticleGenerator() = default;

    void ParticleGenerator::Update(Game* game)
    {
        const float dt = game->GetDeltaTime();

        // Update active generation
        if (mIsActive && mInfo != nullptr)
        {
            mTotalElapsed += dt;

            // Check for duration expiration
            if (mDuration > 0.0F && mTotalElapsed >= mDuration)
            {
                mIsActive = false;
            }

            if (mIsActive)
            {
                if (mType == EmitterType::Continuous)
                {
                    mEmissionTimer += dt;
                    if (mRateOrInterval > 0.0F)
                    {
                        float interval = 1.0F / mRateOrInterval;
                        while (mEmissionTimer >= interval)
                        {
                            Emit(1);
                            mEmissionTimer -= interval;
                        }
                    }
                }
                else if (mType == EmitterType::Pulse)
                {
                    mPulseTimer += dt;
                    if (mPulseTimer >= mRateOrInterval)
                    {
                        Emit(mInfo->GetParticles());
                        mPulseTimer = 0.0F;
                    }
                }
            }
        }

        for (auto& particle : mParticles)
        {
            particle->Update(game);
        }

        // Remove dead particles
        std::erase_if(mParticles, [](const std::unique_ptr<Particle>& p) { return p->IsDead(); });
    }

    void ParticleGenerator::Draw(Game* game)
    {
        for (auto& particle : mParticles)
        {
            particle->Draw(game);
        }
    }

    void ParticleGenerator::Start(const SDL_FPoint& pos,
                                  const ParticleGeneratorInfo* info,
                                  EmitterType type,
                                  EmitterShape shape,
                                  float rateOrInterval,
                                  float duration,
                                  float width,
                                  float height,
                                  const std::vector<ParticleType*>* customTypes,
                                  float baseAngle,
                                  float posVariation,
                                  float lifeMultiplier,
                                  bool doFadeOut)
    {
        if (info == nullptr)
        {
            return;
        }

        mInfo = info;
        mPosition = pos;
        mType = type;
        mShape = shape;
        mRateOrInterval = rateOrInterval;
        mDuration = duration;
        mWidth = width;
        mHeight = height;
        mBaseAngle = baseAngle;
        mPosVariation = posVariation;
        mLifeMultiplier = lifeMultiplier;
        mDoFadeOut = doFadeOut;
        mIsActive = true;
        mTotalElapsed = 0.0F;
        mEmissionTimer = 0.0F;
        mPulseTimer = 0.0F;

        mCustomTypes.clear();
        if (customTypes != nullptr)
        {
            mCustomTypes = *customTypes;
        }

        if (mType == EmitterType::Burst)
        {
            Emit(mInfo->GetParticles());
            mIsActive = false; // Burst is one-shot
        }
    }

    void ParticleGenerator::Burst(const SDL_FPoint& pos,
                                  const ParticleGeneratorInfo* info,
                                  const std::vector<ParticleType*>* customTypes,
                                  float baseAngle,
                                  float posVariation,
                                  float lifeMultiplier,
                                  bool doFadeOut)
    {
        Start(pos,
              info,
              EmitterType::Burst,
              EmitterShape::Cone,
              0.0F,
              0.0F,
              0.0F,
              0.0F,
              customTypes,
              baseAngle,
              posVariation,
              lifeMultiplier,
              doFadeOut);
    }

    void ParticleGenerator::Emit(int count)
    {
        if (mInfo == nullptr || count <= 0)
        {
            return;
        }

        const std::vector<ParticleType*>& types =
            (!mCustomTypes.empty()) ? mCustomTypes : mInfo->GetParticleTypes();

        if (types.empty())
        {
            return;
        }

        int totalWeight = 0;
        for (auto* pt : types)
        {
            totalWeight += pt->GetWeight();
        }
        if (totalWeight <= 0)
        {
            return;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> weightDist(0, totalWeight - 1);
        std::uniform_real_distribution<float> lifeDist(0.2F * mLifeMultiplier,
                                                       0.6F * mLifeMultiplier);
        std::uniform_real_distribution<float> speedVar(0.3F, 1.1F);
        std::uniform_real_distribution<float> posVar(-mPosVariation, mPosVariation);

        for (int i = 0; i < count; ++i)
        {
            int w = weightDist(gen);
            ParticleType* selectedType = nullptr;
            for (auto* pt : types)
            {
                if (w < pt->GetWeight())
                {
                    selectedType = pt;
                    break;
                }
                w -= pt->GetWeight();
            }

            if (selectedType == nullptr || selectedType->GetSequence() == nullptr)
            {
                continue;
            }

            SDL_FPoint p = mPosition;
            float angle = mBaseAngle;

            if (mShape == EmitterShape::Rectangle)
            {
                std::uniform_real_distribution<float> xDist(-mWidth * 0.5F, mWidth * 0.5F);
                std::uniform_real_distribution<float> yDist(-mHeight * 0.5F, mHeight * 0.5F);
                p.x += xDist(gen);
                p.y += yDist(gen);
            }
            else if (mShape == EmitterShape::Circle)
            {
                std::uniform_real_distribution<float> angleDist(0,
                                                                2.0F * std::numbers::pi_v<float>);
                angle = angleDist(gen);
            }
            else // Cone
            {
                std::uniform_real_distribution<float> angleDist(-mInfo->GetConeAngle() * 0.5F,
                                                                mInfo->GetConeAngle() * 0.5F);
                angle += angleDist(gen);
            }

            p.x += posVar(gen);
            p.y += posVar(gen);

            float speed = static_cast<float>(mInfo->GetVelocity()) * speedVar(gen);
            SDL_FPoint vel = {.x = std::cos(angle) * speed, .y = std::sin(angle) * speed};

            mParticles.push_back(std::make_unique<Particle>(
                selectedType->GetSequence(), p, vel, lifeDist(gen), mDoFadeOut));
        }
    }
} // namespace nuvelocity
