#ifndef NVE_PARTICLE_TYPE_H
#define NVE_PARTICLE_TYPE_H

#include "model/Model.h"
#include <string>

namespace nuvelocity
{
    class ParticleType : public Object<ParticleType>
    {
    public:
        ParticleType();
        ~ParticleType();

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CParticleType";
            AddProperty(aInfo, "Particle Type", &ParticleType::mParticleType);
            AddProperty(aInfo, "Weight", &ParticleType::mWeight);
        }

        std::string GetParticleType() const
        {
            return mParticleType;
        }

        int GetWeight() const
        {
            return mWeight;
        }

    private:
        std::string mParticleType;
        int mWeight;
    };
} // namespace nuvelocity

#endif // NVE_PARTICLE_TYPE_H
