#ifndef NVE_FRAME_H
#define NVE_FRAME_H

#include <SDL3/SDL.h>

#include "model/Model.h"
#include <stdexcept>
#include <string>

namespace nuvelocity
{
    class Frame : public Object<Frame>
    {
    public:
        using iterator = uint8_t*;
        using const_iterator = const uint8_t*;

        Frame();
        Frame(int width, int height, int bpp);
        ~Frame();

        void InitFromArgs(const std::vector<std::string>& args) override;

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CFrame";
            aInfo.mSerializationMode = SerializationMode::ByteArray;

            // Add pixel data property and mark it to receive byte array data
            AddProperty(aInfo, "Pixel Data", &Frame::mPixelData);
            aInfo.SetByteArrayProperty(aInfo.GetProperty("Pixel Data"));

            // Register byte array info function for serialization
            aInfo.mByteArrayInfoFunction = [](const void* obj) -> ByteArrayInfo
            {
                const Frame* frame = static_cast<const Frame*>(obj);
                return ByteArrayInfo{frame->mWidth, frame->mHeight, frame->mBitsPerPixel};
            };

            // Register byte array init function to initialize from cols, rows, bits during
            // deserialization
            aInfo.mByteArrayInitFunction = [](void* obj, const ByteArrayInfo& info)
            {
                Frame* frame = static_cast<Frame*>(obj);
                frame->Initialize(info.cols, info.rows, info.bits);
            };
        }

        uint32_t GetPixel(int pointX, int pointY) const;
        int GetWidth() const;
        int GetHeight() const;
        int GetBitsPerPixel() const;

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
        void Initialize(int width, int height, int bpp);

        int mWidth;
        int mHeight;
        int mBitsPerPixel;
        uint8_t* mPixelData;
        SDL_Surface* mSurface;
        SDL_Texture* mTexture;
        SDL_Renderer* mTextureRenderer;
    };
} // namespace nuvelocity

#endif // NVE_FRAME_H
