#ifndef NVE_FRAME_H
#define NVE_FRAME_H

#include "model/Model.h"
#include <string>

namespace nuvelocity
{
    class Frame : public Object<Frame>
    {
    public:
        using iterator = uint8_t*;
        using const_iterator = const uint8_t*;

        Frame();
        Frame(uint32_t width, uint32_t height, uint8_t bpp);
        ~Frame();

        void InitFromArgs(const std::vector<std::string>& args) override;

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CFrame";
            aInfo.mSerializationMode = SerializationMode::HexArray;
            AddProperty(aInfo, "Width", &Frame::mWidth);
            AddProperty(aInfo, "Height", &Frame::mHeight);
            AddProperty(aInfo, "Bits Per Pixel", &Frame::mBitsPerPixel);

            // Add pixel data property and mark it to receive hex array data
            AddProperty(aInfo, "Pixel Data", &Frame::mPixelData);
            aInfo.SetHexArrayProperty(aInfo.GetProperty("Pixel Data"));
        }

        uint32_t GetPixel(uint32_t pointX, uint32_t pointY) const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        uint8_t GetBitsPerPixel() const;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

    private:
        void Initialize(uint32_t width, uint32_t height, uint8_t bpp);

        uint32_t mWidth;
        uint32_t mHeight;
        uint8_t mBitsPerPixel;
        size_t mPixelCount;
        size_t mBytesPerPixel;

        uint8_t* mPixelData;
    };
} // namespace nuvelocity

#endif // NVE_FRAME_H
