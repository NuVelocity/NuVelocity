#ifndef NVE_FRAME_H
#define NVE_FRAME_H

#include <SDL3/SDL.h>

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

        uint32_t GetPixel(int pointX, int pointY) const;
        int GetWidth() const;
        int GetHeight() const;
        uint8_t GetBitsPerPixel() const;

        NVE_API SDL_Surface* GetSurface() const;
        NVE_API void SetSurface(SDL_Surface* surface);
        NVE_API SDL_Texture* GetTexture(SDL_Renderer* renderer);

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

    private:
        void Initialize(int width, int height, uint8_t bpp);

        int mWidth;
        int mHeight;
        uint8_t mBitsPerPixel;
        uint8_t* mPixelData;
        SDL_Surface* mSurface;
        SDL_Texture* mTexture;
        SDL_Renderer* mTextureRenderer;
    };
} // namespace nuvelocity

#endif // NVE_FRAME_H
