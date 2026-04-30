#ifndef NVE_PARTICLE_GENERATOR_H
#define NVE_PARTICLE_GENERATOR_H

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
        ParticleGenerator();
        virtual ~ParticleGenerator() = default;

        void Update(Game* game) override;
        void Draw(Game* game) override;

        void Burst(const SDL_FPoint& pos,
                   const nuvelocity::ParticleGeneratorInfo* info,
                   const std::vector<ParticleType*>* customTypes = nullptr,
                   float baseAngle = 0.0F,
                   float posVariation = 0.0F);

        bool IsFinished() const
        {
            return mParticles.empty();
        }

    private:
        std::vector<std::unique_ptr<Particle>> mParticles;
    };
} // namespace nuvelocity

#endif // NVE_PARTICLE_GENERATOR_H
