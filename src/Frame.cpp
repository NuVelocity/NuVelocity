#include "Frame.h"
#include <limits>

namespace nuvelocity
{
    Frame::Frame()
            : mWidth(0)
            , mHeight(0)
            , mBitsPerPixel(0)
            , mPixelData(nullptr)
    {
    }

    Frame::Frame(uint32_t width, uint32_t height, uint32_t bpp)
            : mWidth(width)
            , mHeight(height)
            , mBitsPerPixel(bpp)
    {
        mPixelData = new uint32_t[static_cast<size_t>(width) * height];
    }

    Frame::~Frame()
    {
        delete[] mPixelData;
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

        delete[] mPixelData;
        mPixelData = nullptr;

        mWidth = width;
        mHeight = height;
        mBitsPerPixel = bpp;

        if (mWidth == 0 || mHeight == 0)
        {
            return;
        }

        if (static_cast<size_t>(mWidth) >
            std::numeric_limits<size_t>::max() / static_cast<size_t>(mHeight))
        {
            throw std::overflow_error("Frame::InitFromArgs pixel count overflow");
        }

        const size_t pixelCount = static_cast<size_t>(mWidth) * static_cast<size_t>(mHeight);
        mPixelData = new uint32_t[pixelCount]();
    }

    uint32_t Frame::GetPixel(uint32_t pointX, uint32_t pointY) const
    {
        if (pointX >= mWidth || pointY >= mHeight)
        {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }
        return mPixelData[(pointY * mWidth) + pointX];
    }

    uint32_t Frame::GetWidth() const
    {
        return mWidth;
    }

    uint32_t Frame::GetHeight() const
    {
        return mHeight;
    }

    uint32_t Frame::GetBitsPerPixel() const
    {
        return mBitsPerPixel;
    }

    Frame::iterator Frame::begin()
    {
        return mPixelData;
    }

    Frame::iterator Frame::end()
    {
        return mPixelData != nullptr ? mPixelData + (static_cast<size_t>(mWidth) * mHeight)
                                     : mPixelData;
    }

    Frame::const_iterator Frame::begin() const
    {
        return mPixelData;
    }

    Frame::const_iterator Frame::end() const
    {
        return mPixelData != nullptr ? mPixelData + (static_cast<size_t>(mWidth) * mHeight)
                                     : mPixelData;
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
