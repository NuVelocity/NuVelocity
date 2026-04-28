#ifndef NVE_PARTICLE_TYPE_H
#define NVE_PARTICLE_TYPE_H

#include "Object.h"
#include <string>

namespace nuvelocity
{
    class Sequence;

    class ParticleType : public Object<ParticleType>
    {
    public:
        ParticleType();
        ~ParticleType();

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CParticleType";
            AddProperty(info, "Particle Type", &ParticleType::mParticleType);
            AddProperty(info, "Weight", &ParticleType::mWeight);
        }

        std::string GetParticleType() const
        {
            return mParticleType;
        }

        int GetWeight() const
        {
            return mWeight;
        }

        Sequence* GetSequence() const
        {
            return mSequence;
        }

        void SetSequence(Sequence* sequence)
        {
            mSequence = sequence;
        }

    private:
        std::string mParticleType;
        int mWeight;
        Sequence* mSequence = nullptr;
    };
} // namespace nuvelocity

#endif // NVE_PARTICLE_TYPE_H
