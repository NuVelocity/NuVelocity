#ifndef NVE_IMAGE_H
#define NVE_IMAGE_H

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>

namespace nuvelocity
{
    class Sequence;
    class StandAloneFrame;

    class Image
    {
    public:
        Image();
        explicit Image(Sequence& sequence);
        explicit Image(StandAloneFrame& frame);

        bool IsValid() const;
        bool IsSequence() const;
        bool IsStandAloneFrame() const;

        void ResetAnimation();
        std::size_t GetFrameCount() const;
        std::size_t GetAnimatedFrameIndex() const;
        SDL_Surface* GetSurface() const;
        SDL_Texture* GetTexture(SDL_Renderer* renderer);

    private:
        enum class Kind
        {
            None,
            Sequence,
            StandAloneFrame
        };

        Kind mKind;
        Sequence* mSequence;
        StandAloneFrame* mFrame;
        uint64_t mAnimationStartTick;
    };
} // namespace nuvelocity

#endif // NVE_IMAGE_H
