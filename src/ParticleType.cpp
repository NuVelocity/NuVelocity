#include "ParticleType.h"

namespace nuvelocity
{
    constexpr const char* kDefaultParticleType = "!None";
    constexpr int kDefaultWeight = 10;

    ParticleType::ParticleType()
            : mParticleType(kDefaultParticleType)
            , mWeight(kDefaultWeight)
    {
    }

    ParticleType::~ParticleType() = default;
} // namespace nuvelocity
