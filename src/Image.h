#ifndef NVE_IMAGE_H
#define NVE_IMAGE_H

#include "Frame.h"
#include "Sequence.h"
#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>

namespace nuvelocity
{
    class Image
    {
    public:
        Image();
        explicit Image(Sequence& sequence);
        explicit Image(Frame& frame);

        bool IsValid() const;
        bool IsSequence() const;
        bool IsFrame() const;

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
            Frame
        };

        Kind mKind;
        Sequence* mSequence;
        Frame* mFrame;
        uint64_t mAnimationStartTick;
    };
} // namespace nuvelocity

#endif // NVE_IMAGE_H
