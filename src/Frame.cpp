#include "Frame.h"
#include <limits>

namespace nuvelocity
{
    constexpr uint8_t kBitsPerByte = 8;
    constexpr uint8_t kBpp8 = 8;
    constexpr uint8_t kBpp16 = 16;
    constexpr uint8_t kBpp24 = 24;
    constexpr uint8_t kBpp32 = 32;
    constexpr uint8_t kShift8 = 8;
    constexpr uint8_t kShift16 = 16;
    constexpr uint8_t kShift24 = 24;

    SDL_PixelFormat GetPixelFormatForBpp(int bpp)
    {
        switch (bpp)
        {
        case kBpp8:
            return SDL_PIXELFORMAT_INDEX8;
        case kBpp16:
            return SDL_PIXELFORMAT_RGB565;
        case kBpp24:
            return SDL_PIXELFORMAT_RGB24;
        case kBpp32:
            return SDL_PIXELFORMAT_RGBA32;
        default:
            throw std::invalid_argument("Unsupported bits-per-pixel value");
        }
    }

    Frame::Frame()
            : mWidth(0)
            , mHeight(0)
            , mBitsPerPixel(0)
            , mPixelData(nullptr)
            , mSurface(nullptr)
            , mTexture(nullptr)
            , mTextureRenderer(nullptr)
            , mHotSpot({.x = 0, .y = 0})
            , mAnchor({.x = 0, .y = 0})
    {
    }

    Frame::Frame(int width, int height, int bpp)
            : mWidth(0)
            , mHeight(0)
            , mBitsPerPixel(0)
            , mPixelData(nullptr)
            , mSurface(nullptr)
            , mTexture(nullptr)
            , mTextureRenderer(nullptr)
            , mHotSpot({.x = 0, .y = 0})
            , mAnchor({.x = 0, .y = 0})
    {
        Initialize(width, height, bpp);
    }

    Frame::~Frame()
    {
        if (mSurface != nullptr)
        {
            SDL_DestroySurface(mSurface);
        }
        if (mTexture != nullptr)
        {
            SDL_DestroyTexture(mTexture);
        }
    }

    void Frame::Initialize(int width, int height, int bpp)
    {
        if (bpp <= 0 || (bpp % kBitsPerByte) != 0)
        {
            throw std::invalid_argument("Bits per pixel must be a non-zero multiple of 8");
        }
        if (width < 0 || height < 0)
        {
            throw std::invalid_argument("Frame dimensions must be non-negative");
        }

        mWidth = width;
        mHeight = height;
        mBitsPerPixel = bpp;

        if (width == 0 || height == 0)
        {
            SetSurface(nullptr);
            return;
        }

        if (static_cast<size_t>(width) >
            std::numeric_limits<size_t>::max() / static_cast<size_t>(height))
        {
            throw std::overflow_error("Frame pixel count overflow");
        }

        SDL_Surface* surface = SDL_CreateSurface(width, height, GetPixelFormatForBpp(bpp));
        if (surface == nullptr)
        {
            throw std::runtime_error("Failed to create frame surface");
        }
        SetSurface(surface);
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    }

    void Frame::InitFromArgs(const std::vector<std::string>& args)
    {
        if (args.size() < 3)
        {
            throw std::invalid_argument("Frame::InitFromArgs expects 3 arguments: cols,rows,bits");
        }

        int cols = 0;
        int rows = 0;
        int bits = 0;
        try
        {
            cols = std::stoi(args[0]);
            rows = std::stoi(args[1]);
            bits = std::stoi(args[2]);
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument(std::string("Frame::InitFromArgs parse failure: ") +
                                        e.what());
        }

        Initialize(cols, rows, bits);
    }

    uint32_t Frame::GetPixel(int pointX, int pointY) const
    {
        if (mSurface == nullptr)
        {
            return 0;
        }

        if (pointX < 0 || pointY < 0 || pointX >= mSurface->w || pointY >= mSurface->h)
        {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }

        const size_t bytesPerPixel = static_cast<size_t>(SDL_BYTESPERPIXEL(mSurface->format));
        const size_t byteIndex =
            (static_cast<size_t>(pointY) * static_cast<size_t>(mSurface->pitch)) +
            (static_cast<size_t>(pointX) * bytesPerPixel);
        const uint8_t* bytes = static_cast<const uint8_t*>(mSurface->pixels);

        switch (mBitsPerPixel)
        {
        case kBpp8:
            return bytes[byteIndex];
        case kBpp16:
            return static_cast<uint32_t>(bytes[byteIndex]) |
                   (static_cast<uint32_t>(bytes[byteIndex + 1]) << kShift8);
        case kBpp24:
            return static_cast<uint32_t>(bytes[byteIndex]) |
                   (static_cast<uint32_t>(bytes[byteIndex + 1]) << kShift8) |
                   (static_cast<uint32_t>(bytes[byteIndex + 2]) << kShift16);
        case kBpp32:
            return static_cast<uint32_t>(bytes[byteIndex]) |
                   (static_cast<uint32_t>(bytes[byteIndex + 1]) << kShift8) |
                   (static_cast<uint32_t>(bytes[byteIndex + 2]) << kShift16) |
                   (static_cast<uint32_t>(bytes[byteIndex + 3]) << kShift24);
        default:
            throw std::logic_error("Unsupported bits-per-pixel value");
        }
    }

    int Frame::GetWidth() const
    {
        return mWidth;
    }

    int Frame::GetHeight() const
    {
        return mHeight;
    }

    int Frame::GetBitsPerPixel() const
    {
        return mBitsPerPixel;
    }

    SDL_Surface* Frame::GetSurface() const
    {
        return mSurface;
    }

    void Frame::SetSurface(SDL_Surface* surface)
    {
        if (mSurface != nullptr)
        {
            SDL_DestroySurface(mSurface);
        }
        mSurface = surface;

        if (mSurface != nullptr)
        {
            mWidth = mSurface->w;
            mHeight = mSurface->h;
            mBitsPerPixel = SDL_BITSPERPIXEL(mSurface->format);
            mPixelData = static_cast<uint8_t*>(mSurface->pixels);
        }
        else
        {
            mWidth = 0;
            mHeight = 0;
            mBitsPerPixel = 0;
            mPixelData = nullptr;
        }

        if (mTexture != nullptr)
        {
            SDL_DestroyTexture(mTexture);
            mTexture = nullptr;
            mTextureRenderer = nullptr;
        }
    }

    SDL_Texture* Frame::GetTexture(SDL_Renderer* renderer)
    {
        if (renderer == nullptr || mSurface == nullptr)
        {
            return nullptr;
        }

        if (mTexture != nullptr && renderer == mTextureRenderer)
        {
            return mTexture;
        }

        if (mTexture != nullptr)
        {
            SDL_DestroyTexture(mTexture);
            mTexture = nullptr;
        }
        mTextureRenderer = nullptr;

        mTexture = SDL_CreateTextureFromSurface(renderer, mSurface);
        if (mTexture != nullptr)
        {
            SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);
            mTextureRenderer = renderer;
        }

        return mTexture;
    }

    SDL_Point Frame::GetHotSpot() const
    {
        return mHotSpot;
    }

    void Frame::SetHotSpot(int x, int y)
    {
        mHotSpot.x = x;
        mHotSpot.y = y;
    }

    SDL_Point Frame::GetAnchor() const
    {
        return mAnchor;
    }

    void Frame::SetAnchor(int x, int y)
    {
        mAnchor.x = x;
        mAnchor.y = y;
    }

    std::vector<SDL_FPoint> Frame::GetCollisionPolygon() const
    {
        if (mSurface == nullptr)
        {
            return {};
        }

        float width = static_cast<float>(mWidth);
        float height = static_cast<float>(mHeight);
        float hotSpotX = static_cast<float>(mHotSpot.x);
        float hotSpotY = static_cast<float>(mHotSpot.y);
        return {{.x = hotSpotX, .y = hotSpotY},
                {.x = hotSpotX + width, .y = hotSpotY},
                {.x = hotSpotX + width, .y = hotSpotY + height},
                {.x = hotSpotX, .y = hotSpotY + height}};
    }

    Frame::iterator Frame::begin()
    {
        if (mSurface == nullptr || mSurface->pixels == nullptr)
        {
            return nullptr;
        }
        return static_cast<uint8_t*>(mSurface->pixels);
    }

    Frame::iterator Frame::end()
    {
        if (mSurface == nullptr || mSurface->pixels == nullptr)
        {
            return nullptr;
        }
        return static_cast<uint8_t*>(mSurface->pixels) +
               (static_cast<size_t>(mSurface->pitch) * static_cast<size_t>(mSurface->h));
    }

    Frame::const_iterator Frame::begin() const
    {
        if (mSurface == nullptr || mSurface->pixels == nullptr)
        {
            return nullptr;
        }
        return static_cast<const uint8_t*>(mSurface->pixels);
    }

    Frame::const_iterator Frame::end() const
    {
        if (mSurface == nullptr || mSurface->pixels == nullptr)
        {
            return nullptr;
        }
        return static_cast<const uint8_t*>(mSurface->pixels) +
               (static_cast<size_t>(mSurface->pitch) * static_cast<size_t>(mSurface->h));
    }

    Frame::const_iterator Frame::cbegin() const
    {
        return begin();
    }

    Frame::const_iterator Frame::cend() const
    {
        return end();
    }
} // namespace nuvelocity
