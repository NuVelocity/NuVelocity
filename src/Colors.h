#ifndef NVE_COLORS_H
#define NVE_COLORS_H

#include <SDL3/SDL_pixels.h>

namespace nuvelocity
{
    class Colors
    {
    public:
        static constexpr SDL_Color White{.r = 255, .g = 255, .b = 255, .a = 255};
        static constexpr SDL_Color Black{.r = 0, .g = 0, .b = 0, .a = 255};
        static constexpr SDL_Color Red{.r = 255, .g = 0, .b = 0, .a = 255};
        static constexpr SDL_Color Green{.r = 0, .g = 255, .b = 0, .a = 255};
        static constexpr SDL_Color Blue{.r = 0, .g = 0, .b = 255, .a = 255};
        static constexpr SDL_Color Yellow{.r = 255, .g = 255, .b = 0, .a = 255};
        static constexpr SDL_Color Magenta{.r = 255, .g = 0, .b = 255, .a = 255};
        static constexpr SDL_Color Cyan{.r = 0, .g = 255, .b = 255, .a = 255};
        static constexpr SDL_Color Transparent{.r = 0, .g = 0, .b = 0, .a = 0};
    };
} // namespace nuvelocity

#endif // NVE_COLORS_H
