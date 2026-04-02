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

    Frame::Frame()
            : mWidth(0)
            , mHeight(0)
            , mBitsPerPixel(0)
            , mPixelCount(0)
            , mBytesPerPixel(0)
            , mPixelData(nullptr)
    {
    }

    Frame::Frame(uint32_t width, uint32_t height, uint8_t bpp)
            : mWidth(0)
            , mHeight(0)
            , mBitsPerPixel(0)
            , mPixelCount(0)
            , mBytesPerPixel(0)
            , mPixelData(nullptr)
    {
        Initialize(width, height, bpp);
    }

    Frame::~Frame()
    {
        delete[] mPixelData;
    }

    void Frame::Initialize(uint32_t width, uint32_t height, uint8_t bpp)
    {
        if (bpp == 0 || (bpp % kBitsPerByte) != 0)
        {
            throw std::invalid_argument("Bits per pixel must be a non-zero multiple of 8");
        }

        delete[] mPixelData;
        mPixelData = nullptr;

        mWidth = width;
        mHeight = height;
        mBitsPerPixel = bpp;
        mBytesPerPixel = static_cast<size_t>(bpp / kBitsPerByte);

        if (width == 0 || height == 0)
        {
            mPixelCount = 0;
            return;
        }

        if (static_cast<size_t>(width) >
            std::numeric_limits<size_t>::max() / static_cast<size_t>(height))
        {
            throw std::overflow_error("Frame pixel count overflow");
        }

        mPixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
        if (mPixelCount > std::numeric_limits<size_t>::max() / mBytesPerPixel)
        {
            throw std::overflow_error("Frame buffer size overflow");
        }

        mPixelData = new uint8_t[mPixelCount * mBytesPerPixel]();
    }

    void Frame::InitFromArgs(const std::vector<std::string>& args)
    {
        if (args.size() < 3)
        {
            throw std::invalid_argument(
                "Frame::InitFromArgs expects 3 arguments: width,height,bitsPerPixel");
        }

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t bpp = 0;
        try
        {
            width = static_cast<uint32_t>(std::stoul(args[0]));
            height = static_cast<uint32_t>(std::stoul(args[1]));
            bpp = static_cast<uint32_t>(std::stoul(args[2]));
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument(std::string("Frame::InitFromArgs parse failure: ") +
                                        e.what());
        }

        Initialize(width, height, static_cast<uint8_t>(bpp));
    }

    uint32_t Frame::GetPixel(uint32_t pointX, uint32_t pointY) const
    {
        if (pointX >= mWidth || pointY >= mHeight)
        {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }

        if (mPixelData == nullptr)
        {
            return 0;
        }

        const size_t pixelIndex = (static_cast<size_t>(pointY) * static_cast<size_t>(mWidth)) +
                                  static_cast<size_t>(pointX);
        const size_t byteIndex = pixelIndex * mBytesPerPixel;

        switch (mBitsPerPixel)
        {
        case kBpp8:
            return mPixelData[byteIndex];
        case kBpp16:
            return static_cast<uint32_t>(mPixelData[byteIndex]) |
                   (static_cast<uint32_t>(mPixelData[byteIndex + 1]) << kShift8);
        case kBpp24:
            return static_cast<uint32_t>(mPixelData[byteIndex]) |
                   (static_cast<uint32_t>(mPixelData[byteIndex + 1]) << kShift8) |
                   (static_cast<uint32_t>(mPixelData[byteIndex + 2]) << kShift16);
        case kBpp32:
            return static_cast<uint32_t>(mPixelData[byteIndex]) |
                   (static_cast<uint32_t>(mPixelData[byteIndex + 1]) << kShift8) |
                   (static_cast<uint32_t>(mPixelData[byteIndex + 2]) << kShift16) |
                   (static_cast<uint32_t>(mPixelData[byteIndex + 3]) << kShift24);
        default:
            throw std::logic_error("Unsupported bits-per-pixel value");
        }
    }

    uint32_t Frame::GetWidth() const
    {
        return mWidth;
    }

    uint32_t Frame::GetHeight() const
    {
        return mHeight;
    }

    uint8_t Frame::GetBitsPerPixel() const
    {
        return mBitsPerPixel;
    }

    Frame::iterator Frame::begin()
    {
        return mPixelData;
    }

    Frame::iterator Frame::end()
    {
        return mPixelData != nullptr ? mPixelData + (mPixelCount * mBytesPerPixel) : mPixelData;
    }

    Frame::const_iterator Frame::begin() const
    {
        return mPixelData;
    }

    Frame::const_iterator Frame::end() const
    {
        return mPixelData != nullptr ? mPixelData + (mPixelCount * mBytesPerPixel) : mPixelData;
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
