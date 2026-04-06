#include "GPUSpriteBatch.h"

#include <SDL3/SDL_gpu.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "shaders/sprite_frag_spv.h"
#include "shaders/sprite_vert_spv.h"

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
            , mPipeline(nullptr)
            , mSampler(nullptr)
            , mCurrentTexture(nullptr)
            , mHasCurrentClipRect(false)
    {
        if (mDevice != nullptr)
        {
            InitializePipeline();
        }
    }

    GPUSpriteBatch::~GPUSpriteBatch()
    {
        // Cancel any unsubmitted command buffer — shutdown doesn't need the work
        // to reach the screen. Submitting would force SDL_DestroyGPUDevice to wait.
        if (mCommandBuffer != nullptr)
        {
            mVertexData.clear();
            mIndexData.clear();
            SDL_CancelGPUCommandBuffer(mCommandBuffer);
            mCommandBuffer = nullptr;
        }
        if (mDevice != nullptr)
        {
            for (auto& [surface, texture] : mTextureCache)
            {
                if (texture != nullptr)
                {
                    SDL_ReleaseGPUTexture(mDevice, texture);
                }
            }
            mTextureCache.clear();

            if (mPipeline != nullptr)
            {
                SDL_ReleaseGPUGraphicsPipeline(mDevice, mPipeline);
            }
            if (mSampler != nullptr)
            {
                SDL_ReleaseGPUSampler(mDevice, mSampler);
            }
            if (mWhiteTexture != nullptr)
            {
                SDL_ReleaseGPUTexture(mDevice, mWhiteTexture);
            }
        }
    }

    void GPUSpriteBatch::InitializePipeline()
    {
        if (mDevice == nullptr)
        {
            return;
        }

        SDL_GPUShaderCreateInfo vertShaderInfo{.code_size = sprite_vert_spv_len,
                                               .code = sprite_vert_spv,
                                               .entrypoint = "main",
                                               .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                               .stage = SDL_GPU_SHADERSTAGE_VERTEX,
                                               .num_samplers = 0,
                                               .num_storage_textures = 0,
                                               .num_storage_buffers = 0,
                                               .num_uniform_buffers = 1,
                                               .props = 0};

        SDL_GPUShaderCreateInfo fragShaderInfo{.code_size = sprite_frag_spv_len,
                                               .code = sprite_frag_spv,
                                               .entrypoint = "main",
                                               .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                               .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                                               .num_samplers = 1,
                                               .num_storage_textures = 0,
                                               .num_storage_buffers = 0,
                                               .num_uniform_buffers = 0,
                                               .props = 0};

        SDL_GPUShader* vertShader = SDL_CreateGPUShader(mDevice, &vertShaderInfo);
        SDL_GPUShader* fragShader = SDL_CreateGPUShader(mDevice, &fragShaderInfo);

        SDL_GPUColorTargetDescription colorTarget = {
            .format = SDL_GetGPUSwapchainTextureFormat(mDevice, mWindow),
            .blend_state = {.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                            .color_blend_op = SDL_GPU_BLENDOP_ADD,
                            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                            .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                            .color_write_mask = 0xF,
                            .enable_blend = true}};

        SDL_GPUVertexBufferDescription vbufDesc = {.slot = 0,
                                                   .pitch = sizeof(nuvelocity::SpriteVertex),
                                                   .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                                                   .instance_step_rate = 0};

        SDL_GPUVertexAttribute vattrs[3] = {{.location = 0,
                                             .buffer_slot = 0,
                                             .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                                             .offset = offsetof(nuvelocity::SpriteVertex, x)},
                                            {.location = 1,
                                             .buffer_slot = 0,
                                             .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                                             .offset = offsetof(nuvelocity::SpriteVertex, u)},
                                            {.location = 2,
                                             .buffer_slot = 0,
                                             .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                                             .offset = offsetof(nuvelocity::SpriteVertex, r)}};

        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {
            .vertex_shader = vertShader,
            .fragment_shader = fragShader,
            .vertex_input_state = {.vertex_buffer_descriptions = &vbufDesc,
                                   .num_vertex_buffers = 1,
                                   .vertex_attributes = vattrs,
                                   .num_vertex_attributes = 3},
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .target_info = {.color_target_descriptions = &colorTarget, .num_color_targets = 1}};

        mPipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineInfo);

        SDL_ReleaseGPUShader(mDevice, vertShader);
        SDL_ReleaseGPUShader(mDevice, fragShader);

        SDL_GPUSamplerCreateInfo samplerInfo{
            .min_filter = SDL_GPU_FILTER_LINEAR,
            .mag_filter = SDL_GPU_FILTER_LINEAR,
            .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        };
        mSampler = SDL_CreateGPUSampler(mDevice, &samplerInfo);

        // Create and cache the 1x1 white texture for primitives
        SDL_Surface* ws = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA32);
        if (ws != nullptr)
        {
            Uint32* p = static_cast<Uint32*>(ws->pixels);
            *p = 0xFFFFFFFF;
            EnsureCommandBuffer();
            mWhiteTexture = CreateAndUploadTexture(ws);
            SDL_DestroySurface(ws);
            // Submit the upload immediately so it doesn't bleed into the first frame's
            // command buffer, which would mix a copy pass with a swapchain render pass.
            SDL_SubmitGPUCommandBuffer(mCommandBuffer);
            mCommandBuffer = nullptr;
        }
    }

    void GPUSpriteBatch::FlushBatch()
    {
        if (mVertexData.empty() || mCommandBuffer == nullptr || mDevice == nullptr ||
            mPipeline == nullptr || mSwapchainTexture == nullptr)
        {
            return;
        }

        // 1. Create/Upload vertices and indices using transfer buffers
        Uint32 vertexBufferSize = static_cast<Uint32>(mVertexData.size() * sizeof(SpriteVertex));
        Uint32 indexBufferSize = static_cast<Uint32>(mIndexData.size() * sizeof(Uint16));

        SDL_GPUTransferBufferCreateInfo tbufInfo{.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                                 .size = vertexBufferSize + indexBufferSize,
                                                 .props = 0};
        SDL_GPUTransferBuffer* tbuf = SDL_CreateGPUTransferBuffer(mDevice, &tbufInfo);
        void* mapped = SDL_MapGPUTransferBuffer(mDevice, tbuf, false);
        SDL_memcpy(mapped, mVertexData.data(), vertexBufferSize);
        SDL_memcpy(
            static_cast<uint8_t*>(mapped) + vertexBufferSize, mIndexData.data(), indexBufferSize);
        SDL_UnmapGPUTransferBuffer(mDevice, tbuf);

        SDL_GPUBufferCreateInfo vbufInfo{
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = vertexBufferSize, .props = 0};
        SDL_GPUBufferCreateInfo ibufInfo{
            .usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = indexBufferSize, .props = 0};
        SDL_GPUBuffer* vbuf = SDL_CreateGPUBuffer(mDevice, &vbufInfo);
        SDL_GPUBuffer* ibuf = SDL_CreateGPUBuffer(mDevice, &ibufInfo);

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(mCommandBuffer);
        SDL_GPUTransferBufferLocation srcVert{.transfer_buffer = tbuf, .offset = 0};
        SDL_GPUBufferRegion dstVert{.buffer = vbuf, .offset = 0, .size = vertexBufferSize};
        SDL_UploadToGPUBuffer(copyPass, &srcVert, &dstVert, false);

        SDL_GPUTransferBufferLocation srcIndex = {.transfer_buffer = tbuf,
                                                  .offset = vertexBufferSize};
        SDL_GPUBufferRegion dstIndex = {.buffer = ibuf, .offset = 0, .size = indexBufferSize};
        SDL_UploadToGPUBuffer(copyPass, &srcIndex, &dstIndex, false);
        SDL_EndGPUCopyPass(copyPass);

        // 2. Render Pass
        bool requiresInitialClear = mNeedsClear && !mHasBlittedThisFrame;
        SDL_GPUColorTargetInfo colorPassInfo = {
            .texture = mSwapchainTexture,
            .clear_color = mClearColor,
            .load_op = requiresInitialClear
                           ? SDL_GPU_LOADOP_CLEAR
                           : (mHasBlittedThisFrame ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR),
            .store_op = SDL_GPU_STOREOP_STORE};

        SDL_GPURenderPass* renderPass =
            SDL_BeginGPURenderPass(mCommandBuffer, &colorPassInfo, 1, nullptr);
        SDL_BindGPUGraphicsPipeline(renderPass, mPipeline);

        if (mHasCurrentClipRect)
        {
            SDL_SetGPUScissor(renderPass, &mCurrentClipRect);
        }

        SDL_GPUBufferBinding vbind{.buffer = vbuf, .offset = 0};
        SDL_BindGPUVertexBuffers(renderPass, 0, &vbind, 1);
        SDL_GPUBufferBinding ibind{.buffer = ibuf, .offset = 0};
        SDL_BindGPUIndexBuffer(renderPass, &ibind, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        if (mCurrentTexture != nullptr)
        {
            SDL_GPUTextureSamplerBinding texBind{.texture = mCurrentTexture, .sampler = mSampler};
            SDL_BindGPUFragmentSamplers(renderPass, 0, &texBind, 1);
        }

        float pushConstants[4] = {2.0F / static_cast<float>(mSwapchainWidth),
                                  -2.0F / static_cast<float>(mSwapchainHeight),
                                  -1.0F,
                                  1.0F};
        SDL_PushGPUVertexUniformData(mCommandBuffer, 0, pushConstants, sizeof(pushConstants));

        SDL_DrawGPUIndexedPrimitives(
            renderPass, static_cast<Uint32>(mIndexData.size()), 1, 0, 0, 0);
        SDL_EndGPURenderPass(renderPass);

        mVertexData.clear();
        mIndexData.clear();
        mHasBlittedThisFrame = true;
        mNeedsClear = false;

        SDL_ReleaseGPUTransferBuffer(mDevice, tbuf);
        SDL_ReleaseGPUBuffer(mDevice, vbuf);
        SDL_ReleaseGPUBuffer(mDevice, ibuf);
    }

    void GPUSpriteBatch::DrawLine(
        float x1, float y1, float x2, float y2, SDL_Color color, float thickness)
    {
        if (mDevice == nullptr || mWindow == nullptr || mWhiteTexture == nullptr)
        {
            return;
        }

        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
            return;

        // Thickness offset (half of total thickness)
        float wx = -dy / len * (thickness * 0.5f);
        float wy = dx / len * (thickness * 0.5f);

        if (mWhiteTexture != mCurrentTexture)
        {
            FlushBatch();
            mCurrentTexture = mWhiteTexture;
        }

        float r = color.r / 255.0f;
        float g = color.g / 255.0f;
        float b = color.b / 255.0f;
        float a = color.a / 255.0f;

        Uint16 baseIdx = (Uint16)mVertexData.size();
        mVertexData.push_back({x1 + wx, y1 + wy, 0.0f, 0.0f, r, g, b, a});
        mVertexData.push_back({x1 - wx, y1 - wy, 1.0f, 0.0f, r, g, b, a});
        mVertexData.push_back({x2 - wx, y2 - wy, 1.0f, 1.0f, r, g, b, a});
        mVertexData.push_back({x2 + wx, y2 + wy, 0.0f, 1.0f, r, g, b, a});

        mIndexData.push_back(baseIdx + 0);
        mIndexData.push_back(baseIdx + 1);
        mIndexData.push_back(baseIdx + 2);
        mIndexData.push_back(baseIdx + 0);
        mIndexData.push_back(baseIdx + 2);
        mIndexData.push_back(baseIdx + 3);
    }

    void GPUSpriteBatch::FillRect(const SDL_FRect* rect, SDL_Color color)
    {
        if (mWhiteTexture == nullptr)
            return;

        if (mWhiteTexture != mCurrentTexture)
        {
            FlushBatch();
            mCurrentTexture = mWhiteTexture;
        }

        SDL_FRect dr;
        if (rect != nullptr)
        {
            dr = *rect;
        }
        else if (mSwapchainWidth > 0 && mSwapchainHeight > 0)
        {
            dr = SDL_FRect{0, 0, static_cast<float>(mSwapchainWidth), static_cast<float>(mSwapchainHeight)};
        }
        else if (mWindow != nullptr)
        {
            int w = 0;
            int h = 0;
            SDL_GetWindowSizeInPixels(mWindow, &w, &h);
            dr = SDL_FRect{0, 0, static_cast<float>(w), static_cast<float>(h)};
        }
        else
        {
            return;
        }

        float r = color.r / 255.0f;
        float g = color.g / 255.0f;
        float b = color.b / 255.0f;
        float a = color.a / 255.0f;

        Uint16 baseIdx = (Uint16)mVertexData.size();
        mVertexData.push_back({dr.x, dr.y, 0, 0, r, g, b, a});
        mVertexData.push_back({dr.x + dr.w, dr.y, 1, 0, r, g, b, a});
        mVertexData.push_back({dr.x + dr.w, dr.y + dr.h, 1, 1, r, g, b, a});
        mVertexData.push_back({dr.x, dr.y + dr.h, 0, 1, r, g, b, a});

        mIndexData.push_back(baseIdx + 0);
        mIndexData.push_back(baseIdx + 1);
        mIndexData.push_back(baseIdx + 2);
        mIndexData.push_back(baseIdx + 0);
        mIndexData.push_back(baseIdx + 2);
        mIndexData.push_back(baseIdx + 3);
    }

    void GPUSpriteBatch::SetClipRect(const SDL_Rect* rect)
    {
        // Only trigger a flush if the clip state is actually changing
        bool isAlreadyClipped = mHasCurrentClipRect;
        bool willBeClipped = (rect != nullptr);

        if (isAlreadyClipped != willBeClipped ||
            (willBeClipped && (mCurrentClipRect.x != rect->x || mCurrentClipRect.y != rect->y ||
                               mCurrentClipRect.w != rect->w || mCurrentClipRect.h != rect->h)))
        {
            FlushBatch();

            if (rect != nullptr)
            {
                mCurrentClipRect = *rect;
                mHasCurrentClipRect = true;
            }
            else
            {
                mHasCurrentClipRect = false;
            }
        }
    }

    void GPUSpriteBatch::Clear(SDL_Color color)
    {
        mClearColor = SDL_FColor{static_cast<float>(color.r) / 255.0F,
                                 static_cast<float>(color.g) / 255.0F,
                                 static_cast<float>(color.b) / 255.0F,
                                 static_cast<float>(color.a) / 255.0F};
        mNeedsClear = true;
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

        if (!SDL_WaitAndAcquireGPUSwapchainTexture(
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

    void GPUSpriteBatch::Draw(SDL_Surface* surface,
                              const SDL_FRect* destRect,
                              const SDL_FRect* srcRect,
                              SDL_Color color)
    {
        if (mDevice == nullptr || mWindow == nullptr || surface == nullptr)
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

        auto cacheIt = mTextureCache.find(surface);
        SDL_GPUTexture* texture = (cacheIt != mTextureCache.end()) ? cacheIt->second : nullptr;

        if (texture == nullptr)
        {
            SDL_Surface* convertedSurface = nullptr;
            SDL_Surface* uploadSurface = EnsureRGBA32Surface(surface, convertedSurface);
            if (uploadSurface == nullptr)
            {
                return;
            }

            texture = CreateAndUploadTexture(uploadSurface);
            if (convertedSurface != nullptr)
            {
                SDL_DestroySurface(convertedSurface);
            }

            if (texture == nullptr)
            {
                return;
            }

            mTextureCache[surface] = texture;
        }

        if (texture != mCurrentTexture)
        {
            FlushBatch();
            mCurrentTexture = texture;
        }

        SDL_FRect dr = destRect ? *destRect : SDL_FRect{0, 0, (float)surface->w, (float)surface->h};
        SDL_FRect sr = srcRect ? *srcRect : SDL_FRect{0, 0, (float)surface->w, (float)surface->h};

        float u1 = sr.x / (float)surface->w;
        float v1 = sr.y / (float)surface->h;
        float u2 = (sr.x + sr.w) / (float)surface->w;
        float v2 = (sr.y + sr.h) / (float)surface->h;

        float r = color.r / 255.0f;
        float g = color.g / 255.0f;
        float b = color.b / 255.0f;
        float a = color.a / 255.0f;

        Uint16 baseIdx = (Uint16)mVertexData.size();
        mVertexData.push_back({dr.x, dr.y, u1, v1, r, g, b, a});
        mVertexData.push_back({dr.x + dr.w, dr.y, u2, v1, r, g, b, a});
        mVertexData.push_back({dr.x + dr.w, dr.y + dr.h, u2, v2, r, g, b, a});
        mVertexData.push_back({dr.x, dr.y + dr.h, u1, v2, r, g, b, a});

        mIndexData.push_back(baseIdx + 0);
        mIndexData.push_back(baseIdx + 1);
        mIndexData.push_back(baseIdx + 2);
        mIndexData.push_back(baseIdx + 0);
        mIndexData.push_back(baseIdx + 2);
        mIndexData.push_back(baseIdx + 3);

        // We don't release the texture here because it's now part of a batch
        // In a real system, we'd cache textures. For now, we'll flush to be safe if textures are
        // short-lived. FlushBatch();
    }

    void GPUSpriteBatch::DrawCentered(SDL_Surface* surface)
    {
        if (mDevice == nullptr || mWindow == nullptr || surface == nullptr)
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

        Draw(surface, &destRect, nullptr);
    }

    void GPUSpriteBatch::Flush()
    {
        FlushBatch();
        SubmitCommandBuffer();
    }

    void GPUSpriteBatch::Present()
    {
        FlushBatch();
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
            mNeedsClear = false;
            mCurrentTexture = nullptr;
        }
    }
} // namespace nuvelocity
