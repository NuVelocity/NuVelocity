#include "GPUSpriteBatch.h"

#include "../Image.h"

#include <algorithm>
#include <cmath>

namespace nuvelocity
{
    constexpr float kCenterFactor = 0.5F;

    Uint32 FloatToUint32(float value)
    {
        const long rounded = std::lround(value);
        return static_cast<Uint32>(std::max<long>(0L, rounded));
    }

    static SDL_Surface* EnsureRGBA32Surface(SDL_Surface* surface, SDL_Surface*& convertedSurface)
    {
        convertedSurface = nullptr;
        if (surface == nullptr)
        {
            return nullptr;
        }

        if (surface->format == SDL_PIXELFORMAT_RGBA32)
        {
            return surface;
        }

        convertedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        return convertedSurface;
    }

    GPUSpriteBatch::GPUSpriteBatch(SDL_GPUDevice* device)
            : GPUSpriteBatch(device, nullptr)
    {
    }

    GPUSpriteBatch::GPUSpriteBatch(SDL_GPUDevice* device, SDL_Window* window)
            : mDevice(device)
            , mWindow(window)
            , mCommandBuffer(nullptr)
            , mSwapchainTexture(nullptr)
            , mSwapchainWidth(0)
            , mSwapchainHeight(0)
            , mHasBlittedThisFrame(false)
    {
    }

    GPUSpriteBatch::~GPUSpriteBatch()
    {
        Flush();
    }

    void GPUSpriteBatch::EnsureCommandBuffer()
    {
        if (mCommandBuffer == nullptr && mDevice != nullptr)
        {
            mCommandBuffer = SDL_AcquireGPUCommandBuffer(mDevice);
            mSwapchainTexture = nullptr;
            mSwapchainWidth = 0;
            mSwapchainHeight = 0;
            mHasBlittedThisFrame = false;
        }
    }

    bool GPUSpriteBatch::EnsureSwapchainTexture()
    {
        if (mCommandBuffer == nullptr || mDevice == nullptr || mWindow == nullptr)
        {
            return false;
        }

        if (mSwapchainTexture != nullptr)
        {
            return true;
        }

        if (!SDL_AcquireGPUSwapchainTexture(
                mCommandBuffer, mWindow, &mSwapchainTexture, &mSwapchainWidth, &mSwapchainHeight))
        {
            return false;
        }

        return mSwapchainTexture != nullptr;
    }

    SDL_GPUTexture* GPUSpriteBatch::CreateAndUploadTexture(SDL_Surface* surface)
    {
        if (surface == nullptr || mDevice == nullptr || mCommandBuffer == nullptr)
        {
            return nullptr;
        }

        SDL_GPUTextureCreateInfo textureInfo{.type = SDL_GPU_TEXTURETYPE_2D,
                                             .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                                             .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                                             .width = static_cast<Uint32>(surface->w),
                                             .height = static_cast<Uint32>(surface->h),
                                             .layer_count_or_depth = 1,
                                             .num_levels = 1,
                                             .sample_count = SDL_GPU_SAMPLECOUNT_1,
                                             .props = 0};

        SDL_GPUTexture* texture = SDL_CreateGPUTexture(mDevice, &textureInfo);
        if (texture == nullptr)
        {
            return nullptr;
        }

        const Uint32 transferSize = static_cast<Uint32>(surface->pitch * surface->h);
        SDL_GPUTransferBufferCreateInfo transferInfo{
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = transferSize, .props = 0};

        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(mDevice, &transferInfo);
        if (transferBuffer == nullptr)
        {
            SDL_ReleaseGPUTexture(mDevice, texture);
            return nullptr;
        }

        void* mapped = SDL_MapGPUTransferBuffer(mDevice, transferBuffer, false);
        if (mapped == nullptr)
        {
            SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);
            SDL_ReleaseGPUTexture(mDevice, texture);
            return nullptr;
        }

        SDL_memcpy(mapped, surface->pixels, transferSize);
        SDL_UnmapGPUTransferBuffer(mDevice, transferBuffer);

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(mCommandBuffer);
        if (copyPass == nullptr)
        {
            SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);
            SDL_ReleaseGPUTexture(mDevice, texture);
            return nullptr;
        }

        SDL_GPUTextureTransferInfo sourceInfo{.transfer_buffer = transferBuffer,
                                              .offset = 0,
                                              .pixels_per_row =
                                                  static_cast<Uint32>(surface->pitch / 4),
                                              .rows_per_layer = static_cast<Uint32>(surface->h)};
        SDL_GPUTextureRegion destinationInfo{.texture = texture,
                                             .mip_level = 0,
                                             .layer = 0,
                                             .x = 0,
                                             .y = 0,
                                             .z = 0,
                                             .w = static_cast<Uint32>(surface->w),
                                             .h = static_cast<Uint32>(surface->h),
                                             .d = 1};

        SDL_UploadToGPUTexture(copyPass, &sourceInfo, &destinationInfo, false);
        SDL_EndGPUCopyPass(copyPass);

        SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);
        return texture;
    }

    void
    GPUSpriteBatch::DrawImage(Image& image, const SDL_FRect* destRect, const SDL_FRect* srcRect)
    {
        if (mDevice == nullptr || mWindow == nullptr)
        {
            return;
        }

        SDL_Surface* surface = image.GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        EnsureCommandBuffer();
        if (mCommandBuffer == nullptr)
        {
            return;
        }

        if (!EnsureSwapchainTexture() || mSwapchainTexture == nullptr)
        {
            return;
        }

        SDL_Surface* convertedSurface = nullptr;
        SDL_Surface* uploadSurface = EnsureRGBA32Surface(surface, convertedSurface);
        if (uploadSurface == nullptr)
        {
            return;
        }

        SDL_GPUTexture* texture = CreateAndUploadTexture(uploadSurface);
        if (convertedSurface != nullptr)
        {
            SDL_DestroySurface(convertedSurface);
        }

        if (texture == nullptr)
        {
            return;
        }

        SDL_FRect resolvedDestRect{};
        if (destRect != nullptr)
        {
            resolvedDestRect = *destRect;
        }
        else
        {
            resolvedDestRect.x = 0.0F;
            resolvedDestRect.y = 0.0F;
            resolvedDestRect.w = static_cast<float>(uploadSurface->w);
            resolvedDestRect.h = static_cast<float>(uploadSurface->h);
        }

        SDL_FRect resolvedSrcRect{};
        if (srcRect != nullptr)
        {
            resolvedSrcRect = *srcRect;
        }
        else
        {
            resolvedSrcRect.x = 0.0F;
            resolvedSrcRect.y = 0.0F;
            resolvedSrcRect.w = static_cast<float>(uploadSurface->w);
            resolvedSrcRect.h = static_cast<float>(uploadSurface->h);
        }

        SDL_GPUBlitInfo blitInfo{};
        blitInfo.source.texture = texture;
        blitInfo.source.mip_level = 0;
        blitInfo.source.layer_or_depth_plane = 0;
        blitInfo.source.x = FloatToUint32(resolvedSrcRect.x);
        blitInfo.source.y = FloatToUint32(resolvedSrcRect.y);
        blitInfo.source.w = FloatToUint32(resolvedSrcRect.w);
        blitInfo.source.h = FloatToUint32(resolvedSrcRect.h);
        blitInfo.destination.texture = mSwapchainTexture;
        blitInfo.destination.mip_level = 0;
        blitInfo.destination.layer_or_depth_plane = 0;
        blitInfo.destination.x = FloatToUint32(resolvedDestRect.x);
        blitInfo.destination.y = FloatToUint32(resolvedDestRect.y);
        blitInfo.destination.w = FloatToUint32(resolvedDestRect.w);
        blitInfo.destination.h = FloatToUint32(resolvedDestRect.h);
        blitInfo.load_op = mHasBlittedThisFrame ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR;
        blitInfo.clear_color = SDL_FColor{.r = 0.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F};
        blitInfo.flip_mode = SDL_FLIP_NONE;
        blitInfo.filter = SDL_GPU_FILTER_LINEAR;
        blitInfo.cycle = false;

        SDL_BlitGPUTexture(mCommandBuffer, &blitInfo);
        SDL_ReleaseGPUTexture(mDevice, texture);
        mHasBlittedThisFrame = true;
    }

    void GPUSpriteBatch::DrawImageCentered(Image& image)
    {
        if (mDevice == nullptr || mWindow == nullptr)
        {
            return;
        }

        SDL_Surface* surface = image.GetSurface();
        if (surface == nullptr)
        {
            return;
        }

        int winWidth = 0;
        int winHeight = 0;
        SDL_GetWindowSizeInPixels(mWindow, &winWidth, &winHeight);

        SDL_FRect destRect{
            .x = (static_cast<float>(winWidth) - static_cast<float>(surface->w)) * kCenterFactor,
            .y = (static_cast<float>(winHeight) - static_cast<float>(surface->h)) * kCenterFactor,
            .w = static_cast<float>(surface->w),
            .h = static_cast<float>(surface->h)};

        DrawImage(image, &destRect, nullptr);
    }

    void GPUSpriteBatch::Flush()
    {
        SubmitCommandBuffer();
    }

    void GPUSpriteBatch::SubmitCommandBuffer()
    {
        if (mCommandBuffer != nullptr && mDevice != nullptr)
        {
            SDL_SubmitGPUCommandBuffer(mCommandBuffer);
            mCommandBuffer = nullptr;
            mSwapchainTexture = nullptr;
            mSwapchainWidth = 0;
            mSwapchainHeight = 0;
            mHasBlittedThisFrame = false;
        }
    }
} // namespace nuvelocity
