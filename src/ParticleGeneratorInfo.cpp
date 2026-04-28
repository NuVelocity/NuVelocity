#include "ParticleGeneratorInfo.h"

namespace nuvelocity
{
    ParticleGeneratorInfo::ParticleGeneratorInfo()
            : mParticles(0)
            , mVelocity(0)
            , mConeAngle(0.0F)
    {
    }

    ParticleGeneratorInfo::~ParticleGeneratorInfo()
    {
        for (auto* pt : mParticleTypes)
        {
            delete pt;
        }
    }
} // namespace nuvelocity
