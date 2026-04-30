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

    void ParticleGenerator::Burst(const SDL_FPoint& pos,
                                  const ParticleGeneratorInfo* info,
                                  const std::vector<ParticleType*>* customTypes,
                                  float baseAngle,
                                  float posVariation,
                                  float lifeMultiplier)
    {
        if (info == nullptr)
        {
            return;
        }

        const std::vector<ParticleType*>& types = (customTypes != nullptr && !customTypes->empty())
                                                      ? *customTypes
                                                      : info->GetParticleTypes();

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
        std::uniform_real_distribution<float> angleDist(-info->GetConeAngle() * 0.5F,
                                                        info->GetConeAngle() * 0.5F);
        std::uniform_real_distribution<float> lifeDist(0.2F * lifeMultiplier,
                                                       0.6F * lifeMultiplier);
        std::uniform_real_distribution<float> speedVar(0.3F, 1.1F);
        std::uniform_real_distribution<float> posVar(-posVariation, posVariation);

        for (int i = 0; i < info->GetParticles(); ++i)
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

            // Direction: Centered around baseAngle
            float angle = angleDist(gen) + baseAngle;
            float speed = static_cast<float>(info->GetVelocity()) * speedVar(gen);

            SDL_FPoint vel = {.x = std::cos(angle) * speed, .y = std::sin(angle) * speed};
            SDL_FPoint p = {pos.x + posVar(gen), pos.y + posVar(gen)};

            mParticles.push_back(
                std::make_unique<Particle>(selectedType->GetSequence(), p, vel, lifeDist(gen)));
        }
    }
} // namespace nuvelocity
