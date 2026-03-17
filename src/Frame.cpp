#include "Frame.h"

namespace nuvelocity::frs42
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

    Frame::~Frame() = default;

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
} // namespace nuvelocity::frs42
