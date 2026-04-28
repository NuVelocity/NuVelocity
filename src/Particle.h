#ifndef NVE_PARTICLE_H
#define NVE_PARTICLE_H

#include <GameComponent.h>
#include <SDL3/SDL.h>

namespace nuvelocity
{
    class Sequence;

    class Particle : public GameComponent
    {
    public:
        Particle(nuvelocity::Sequence* seq, SDL_FPoint pos, SDL_FPoint vel, float lifeTime);
        virtual ~Particle() = default;

        void Update(Game* game) override;
        void Draw(Game* game) override;

        bool IsDead() const
        {
            return mLife <= 0.0F;
        }

    private:
        nuvelocity::Sequence* mSequence;
        SDL_FPoint mPosition;
        SDL_FPoint mVelocity;
        float mLife;
        float mMaxLife;
        uint64_t mStartTick;
    };
} // namespace nuvelocity

#endif // NVE_PARTICLE_H
