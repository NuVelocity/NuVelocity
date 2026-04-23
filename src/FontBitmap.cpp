#include "FontBitmap.h"

#include "Frame.h"
#include "Sequence.h"
#include <cmath>
#include <utility>

namespace nuvelocity
{
    constexpr int kMinGlyphSize = 1;

    static float ResolveBitmapScale(const FontBitmap& fontBitmap, int pointSize)
    {
        if (pointSize <= 0)
        {
            return 1.0F;
        }

        const int xHeight = fontBitmap.GetXHeight();
        if (xHeight <= 0)
        {
            return 1.0F;
        }

        const float scale = static_cast<float>(pointSize) / static_cast<float>(xHeight);
        return scale > 0.0F ? scale : 1.0F;
    }

    static int GetFallbackSpaceAdvance(const FontBitmap& fontBitmap, float scale)
    {
        const int xHeight = fontBitmap.GetXHeight();
        const int base = xHeight > 0 ? (xHeight / 2) : 6;
        return SDL_max(kMinGlyphSize,
                       static_cast<int>(std::lround(static_cast<float>(base) * scale)));
    }

    static std::size_t ResolveGlyphIndex(const FontBitmap& fontBitmap, uint8_t asciiCode)
    {
        return static_cast<std::size_t>(static_cast<int>(asciiCode) - fontBitmap.GetFirstAscii());
    }

    static bool
    TryGetBitmapGlyphFrame(const FontBitmap& fontBitmap, uint8_t asciiCode, Frame*& frame)
    {
        frame = nullptr;

        const Sequence* sequence = fontBitmap.GetSequence();
        if (sequence == nullptr)
        {
            return false;
        }

        const int firstAscii = fontBitmap.GetFirstAscii();
        const int lastAscii = fontBitmap.GetLastAscii();
        const int asciiValue = static_cast<int>(asciiCode);
        if (asciiValue < firstAscii || asciiValue > lastAscii)
        {
            return false;
        }

        const std::size_t glyphIndex = ResolveGlyphIndex(fontBitmap, asciiCode);
        if (glyphIndex >= sequence->GetFrameCount())
        {
            return false;
        }

        frame = sequence->GetFrame(glyphIndex);
        return frame != nullptr;
    }

    FontBitmap::FontBitmap()
            : mFirstAscii(0)
            , mLastAscii(0)
            , mIsFixedWidth(false)
            , mXHeight(0)
            , mColorable(false)
    {
    }

    int FontBitmap::GetFirstAscii() const
    {
        return mFirstAscii;
    }

    int FontBitmap::GetLastAscii() const
    {
        return mLastAscii;
    }

    bool FontBitmap::IsFixedWidth() const
    {
        return mIsFixedWidth;
    }

    int FontBitmap::GetXHeight() const
    {
        return mXHeight;
    }

    void FontBitmap::SetFirstAscii(int firstAscii)
    {
        mFirstAscii = firstAscii;
    }

    void FontBitmap::SetLastAscii(int lastAscii)
    {
        mLastAscii = lastAscii;
    }

    void FontBitmap::SetFixedWidth(bool isFixedWidth)
    {
        mIsFixedWidth = isFixedWidth;
    }

    void FontBitmap::SetXHeight(int xHeight)
    {
        mXHeight = xHeight;
    }

    bool FontBitmap::IsColorable() const
    {
        return mColorable;
    }

    void FontBitmap::SetColorable(bool colorable)
    {
        mColorable = colorable;
    }

    void FontBitmap::SetSequence(std::unique_ptr<Sequence>&& sequence)
    {
        mSequence = std::move(sequence);
    }

    const Sequence* FontBitmap::GetSequence() const
    {
        return mSequence.get();
    }

    Sequence* FontBitmap::GetSequence()
    {
        return mSequence.get();
    }

    bool
    FontBitmap::MeasureString(const std::string& text, int pointSize, int& width, int& height) const
    {
        width = 0;
        height = 0;

        if (mSequence == nullptr)
        {
            return false;
        }

        const float scale = ResolveBitmapScale(*this, pointSize);
        const int spaceAdvance = GetFallbackSpaceAdvance(*this, scale);

        int totalWidth = 0;
        int maxHeight = SDL_max(pointSize, kMinGlyphSize);

        for (char ch : text)
        {
            const uint8_t asciiCode = static_cast<uint8_t>(ch);
            if (asciiCode == static_cast<uint8_t>(' '))
            {
                totalWidth += spaceAdvance;
                continue;
            }

            Frame* glyph = nullptr;
            if (!TryGetBitmapGlyphFrame(*this, asciiCode, glyph))
            {
                totalWidth += spaceAdvance;
                continue;
            }

            const int glyphWidth = SDL_max(
                kMinGlyphSize,
                static_cast<int>(std::lround(static_cast<float>(glyph->GetWidth()) * scale)));
            const int glyphHeight = SDL_max(
                kMinGlyphSize,
                static_cast<int>(std::lround(static_cast<float>(glyph->GetHeight()) * scale)));
            totalWidth += glyphWidth;
            maxHeight = SDL_max(maxHeight, glyphHeight);
        }

        width = totalWidth;
        height = maxHeight;
        return true;
    }

    void FontBitmap::DrawString(SpriteBatch* batch,
                                const std::string& text,
                                const SDL_Rect& bounds,
                                const SDL_Color& color,
                                int pointSize,
                                TextAlignment alignment,
                                bool verticalCenter,
                                int underlineIndex) const
    {
        if (batch == nullptr || text.empty() || mSequence == nullptr)
        {
            return;
        }

        int measuredWidth = 0;
        int measuredHeight = 0;
        if (!MeasureString(text, pointSize, measuredWidth, measuredHeight))
        {
            return;
        }

        int x = bounds.x;
        switch (alignment)
        {
        case TextAlignment::Center:
            x = bounds.x + (bounds.w - measuredWidth) / 2;
            break;
        case TextAlignment::Right:
            x = bounds.x + bounds.w - measuredWidth;
            break;
        case TextAlignment::Left:
        default:
            break;
        }

        int y = bounds.y;
        if (verticalCenter)
        {
            y = bounds.y + (bounds.h - measuredHeight) / 2;
        }

        const float scale = ResolveBitmapScale(*this, pointSize);
        const int spaceAdvance = GetFallbackSpaceAdvance(*this, scale);

        int cursorX = x;
        for (char ch : text)
        {
            const uint8_t asciiCode = static_cast<uint8_t>(ch);
            if (asciiCode == static_cast<uint8_t>(' '))
            {
                cursorX += spaceAdvance;
                continue;
            }

            Frame* glyph = nullptr;
            if (!TryGetBitmapGlyphFrame(*this, asciiCode, glyph))
            {
                cursorX += spaceAdvance;
                continue;
            }

            SDL_Surface* glyphSurface = glyph->GetSurface();
            if (glyphSurface == nullptr)
            {
                cursorX += spaceAdvance;
                continue;
            }

            const int glyphWidth = SDL_max(1, static_cast<int>(std::lround(static_cast<float>(glyph->GetWidth()) * scale)));
            const int glyphHeight = SDL_max(1, static_cast<int>(std::lround(static_cast<float>(glyph->GetHeight()) * scale)));
            SDL_Rect dstRect{.x = cursorX + glyph->mHotSpot.x,
                              .y = y + glyph->mHotSpot.y,
                              .w = glyphWidth,
                              .h = glyphHeight};

            // Use SpriteBatch to draw the glyph surface with color modulation.
            // If not colorable, use white but preserve the requested alpha.
            const SDL_Color drawColor = mColorable ? color : SDL_Color{255, 255, 255, color.a};
            batch->Draw(glyphSurface, &dstRect, nullptr, drawColor);

            cursorX += glyphWidth;
        }

        if (underlineIndex >= 0 && underlineIndex < static_cast<int>(text.size()))
        {
            const std::string prefix = text.substr(0, static_cast<std::size_t>(underlineIndex));
            const std::string underlinedCharacter =
                text.substr(static_cast<std::size_t>(underlineIndex), 1);

            int prefixWidth = 0;
            int prefixHeight = 0;
            int characterWidth = 0;
            int characterHeight = 0;
            MeasureString(prefix, pointSize, prefixWidth, prefixHeight);
            MeasureString(underlinedCharacter, pointSize, characterWidth, characterHeight);

            const int lineY = y + SDL_max(0, measuredHeight - 2);
            const int lineStartX = x + prefixWidth;
            const int lineEndX = lineStartX + SDL_max(1, characterWidth);

            // Use SpriteBatch for line drawing
            batch->DrawLine(lineStartX, lineY, lineEndX, lineY, color);
        }
    }

    void FontBitmap::DrawStringAt(SpriteBatch* batch,
                                  const std::string& text,
                                  int x,
                                  int y,
                                  const SDL_Color& color,
                                  int pointSize,
                                  TextAlignment alignment,
                                  bool verticalCenter,
                                  int underlineIndex) const
    {
        if (batch == nullptr || text.empty() || mSequence == nullptr)
        {
            return;
        }
        DrawString(batch,
                   text,
                   SDL_Rect{.x = x, .y = y, .w = 0, .h = 0},
                   color,
                   pointSize,
                   alignment,
                   verticalCenter,
                   underlineIndex);
    }

    FontBitmap::~FontBitmap() = default;
} // namespace nuvelocity
