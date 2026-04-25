#include "ParticleType.h"

namespace nuvelocity
{
    constexpr const char* DEFAULT_PARTICLE_TYPE = "!None";
    constexpr int DEFAULT_WEIGHT = 10;

    ParticleType::ParticleType()
            : mParticleType(DEFAULT_PARTICLE_TYPE)
            , mWeight(DEFAULT_WEIGHT)
    {
    }

    ParticleType::~ParticleType() = default;
} // namespace nuvelocity
