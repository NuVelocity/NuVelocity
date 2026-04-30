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
        NVE_API ParticleType();
        NVE_API ~ParticleType();

        static void InitClassInfo(ClassInfo& info)
        {
            info.mName = "CParticleType";
            AddProperty(info, "Particle Type", &ParticleType::mParticleType);
            AddProperty(info, "Weight", &ParticleType::mWeight);
        }

        NVE_API std::string GetParticleType() const
        {
            return mParticleType;
        }

        NVE_API int GetWeight() const
        {
            return mWeight;
        }

        NVE_API Sequence* GetSequence() const
        {
            return mSequence;
        }

        NVE_API void SetSequence(Sequence* sequence)
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
