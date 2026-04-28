#ifndef NVE_PARTICLE_GENERATOR_INFO_H
#define NVE_PARTICLE_GENERATOR_INFO_H

#include "Object.h"
#include "ParticleType.h"
#include <vector>

namespace nuvelocity
{
    class ParticleGeneratorInfo : public Object<ParticleGeneratorInfo>
    {
    public:
        ParticleGeneratorInfo();
        virtual ~ParticleGeneratorInfo();

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CParticleGeneratorInfo";
            AddProperty(info, "Particles", &ParticleGeneratorInfo::mParticles);
            AddProperty(info, "Velocity", &ParticleGeneratorInfo::mVelocity);
            AddProperty(info, "Cone Angle", &ParticleGeneratorInfo::mConeAngle);
            AddProperty(info, "Particle Types", &ParticleGeneratorInfo::mParticleTypes);
        }

        int GetParticles() const
        {
            return mParticles;
        }
        int GetVelocity() const
        {
            return mVelocity;
        }
        float GetConeAngle() const
        {
            return mConeAngle;
        }
        const std::vector<ParticleType*>& GetParticleTypes() const
        {
            return mParticleTypes;
        }

    private:
        int mParticles;
        int mVelocity;
        float mConeAngle;
        std::vector<ParticleType*> mParticleTypes;
    };
} // namespace nuvelocity

#endif // NVE_PARTICLE_GENERATOR_INFO_H
