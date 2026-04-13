#include <SDL3_image/SDL_image.h>
#include <cstdint>

#include "FrameLoaderMode3.h"

#include "DecodeUtils.h"

namespace nuvelocity
{
    StandAloneFrame* FrameLoaderMode3::Load(SDL_IOStream* stream)
    {
        if (stream == nullptr)
        {
            return nullptr;
        }

        auto* frame = new StandAloneFrame();
        if (!LoadFromStream(frame, stream))
        {
            delete frame;
            return nullptr;
        }

        return frame;
    }

    bool FrameLoaderMode3::LoadFromStream(StandAloneFrame* frame, SDL_IOStream* stream)
    {
        int32_t hotSpotX;
        int32_t hotSpotY;
        uint8_t isCompressed;

        SDL_ReadS32LE(stream, &hotSpotX);
        SDL_ReadS32LE(stream, &hotSpotY);
        SDL_ReadU8(stream, &isCompressed);

        frame->mHotSpot = {hotSpotX, hotSpotY};

        // Case 1: Compressed image data.
        if (isCompressed != 0U)
        {
            // Case 1.1: Deflate-compressed image data, 4-plane/32-bit-plane RGBA.
            if (DecodeUtils::FrameHasDeflateHeader(stream))
            {
                return LoadCompressedDeflateFrame(frame, stream);
            }

            // Case 1.2: Packed image data, 32-bit RGBA8888.
            return LoadCompressedPackedFrame(frame, stream);
        }

        // Case 2: JPEG image data.
        return LoadJpegFrame(frame, stream);
    }

    bool FrameLoaderMode3::LoadCompressedDeflateFrame(StandAloneFrame* frame, SDL_IOStream* stream)
    {
        // Note to callers: this method assumes the stream is positioned after the 9-byte header,
        // and before the deflate header.
        uint32_t deflatedSize = 0;
        uint32_t inflatedSize = 0;
        int width = 0;
        int height = 0;

        // Ignore byte marker for packed data size.
        SDL_ReadU8(stream, nullptr);
        SDL_ReadU32LE(stream, &deflatedSize);
        SDL_ReadU32LE(stream, &inflatedSize);

        auto* sourceImageData = static_cast<uint8_t*>(SDL_malloc(deflatedSize));
        if (SDL_ReadIO(stream, sourceImageData, deflatedSize) != deflatedSize)
        {
            SDL_free(sourceImageData);
            return false;
        }

        auto* imageData = static_cast<uint8_t*>(SDL_malloc(inflatedSize));
        if (DecodeUtils::Inflate(imageData, &inflatedSize, sourceImageData, &deflatedSize) != Z_OK)
        {
            SDL_free(sourceImageData);
            SDL_free(imageData);
            return false;
        }
        SDL_free(sourceImageData);

        SDL_ReadS32LE(stream, &width);
        SDL_ReadS32LE(stream, &height);

        // Create SDL surface from decompressed image data
        SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        if (surface == nullptr)
        {
            SDL_free(imageData);
            return false;
        }
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
        // Process 4 planes of RGBA data with row offset addition
        for (int plane = 0; plane < 4; plane++)
        {
            DecodeUtils::MergeBitPlane(plane, plane, width, height, imageData, surface);
        }
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
        frame->SetSurface(surface);

        SDL_free(imageData);
        return true;
    }

    bool FrameLoaderMode3::LoadCompressedPackedFrame(StandAloneFrame* frame, SDL_IOStream* stream)
    {
        // Note to callers: this method assumes the stream is positioned after the 9-byte header.
        uint32_t inflatedSize = 0;
        int width = 0;
        int height = 0;

        SDL_ReadU32LE(stream, &inflatedSize);
        auto* imageData = static_cast<uint8_t*>(SDL_malloc(inflatedSize));
        if (SDL_ReadIO(stream, imageData, inflatedSize) != inflatedSize)
        {
            SDL_free(imageData);
            return false;
        }

        SDL_ReadS32LE(stream, &width);
        SDL_ReadS32LE(stream, &height);

        // Create SDL surface from packed image data
        SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        if (surface == nullptr)
        {
            SDL_free(imageData);
            return false;
        }
        frame->SetSurface(surface);

        SDL_memcpy(surface->pixels, imageData, inflatedSize);
        SDL_free(imageData);
        return true;
    }

    bool FrameLoaderMode3::LoadJpegFrame(StandAloneFrame* frame, SDL_IOStream* stream)
    {
        // Note to callers: this method assumes the stream is positioned after the 9-byte header.
        uint32_t imageSize = 0;
        SDL_ReadU32LE(stream, &imageSize);

        void* imageData = SDL_malloc(imageSize);
        if (SDL_ReadIO(stream, imageData, imageSize) != imageSize)
        {
            SDL_free(imageData);
            return false;
        }

        SDL_IOStream* imageStream = SDL_IOFromConstMem(imageData, imageSize);
        SDL_Surface* surface = IMG_LoadJPG_IO(imageStream);
        frame->SetSurface(surface);
        SDL_free(imageData);
        SDL_CloseIO(imageStream);

        if (surface == nullptr)
        {
            return false;
        }

        // The original surface does not have an alpha channel.
        if (SDL_ReadIO(stream, nullptr, 0) == 0 && SDL_GetIOStatus(stream) != SDL_IO_STATUS_EOF)
        {
            uint32_t inflatedSize = 0;
            uint32_t deflatedSize = 0;

            // The original surface does not have an alpha channel.
            SDL_Surface* output = SDL_CreateSurface(surface->w, surface->h, SDL_PIXELFORMAT_RGBA32);
            if (output == nullptr)
            {
                return false;
            }
            SDL_SetSurfaceBlendMode(output, SDL_BLENDMODE_NONE);

            // Ignore padding byte.
            SDL_ReadU8(stream, nullptr);
            SDL_ReadU32LE(stream, &inflatedSize);
            deflatedSize = SDL_GetIOSize(stream) - SDL_TellIO(stream);

            auto* sourceMaskData = static_cast<uint8_t*>(SDL_malloc(deflatedSize));
            if (SDL_ReadIO(stream, sourceMaskData, deflatedSize) != deflatedSize)
            {
                SDL_free(sourceMaskData);
                SDL_DestroySurface(output);
                return false;
            }

            auto* maskData = static_cast<uint8_t*>(SDL_malloc(inflatedSize));
            if (DecodeUtils::Inflate(maskData, &inflatedSize, sourceMaskData, &deflatedSize) !=
                Z_OK)
            {
                SDL_free(sourceMaskData);
                SDL_free(maskData);
                SDL_DestroySurface(output);
                return false;
            }
            SDL_free(sourceMaskData);

            SDL_BlitSurface(surface, nullptr, output, nullptr);
            SDL_SetSurfaceBlendMode(output, SDL_BLENDMODE_BLEND);
            frame->SetSurface(output);
            surface = output;

            DecodeUtils::MergeBitPlane(0, 3, surface->w, surface->h, maskData, surface);
            SDL_free(maskData);
        }

        return true;
    }
} // namespace nuvelocity
