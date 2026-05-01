#include "GPUSpriteBatch.h"

#include <SDL3/SDL_gpu.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "Colors.h"
#include "shaders/sprite_frag_spv.h"
#include "shaders/sprite_vert_spv.h"

namespace nuvelocity
{
    constexpr float kCenterFactor = 0.5F;

    // Maximum vertices per flush — Uint16 indices cap out at 65535.
    // We leave a small margin so the overflow check before push_back is simple.
    constexpr Uint16 kMaxVerticesPerFlush = 65532; // divisible by 4 (one quad)

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
            , mSampler(nullptr)
            , mCurrentTexture(nullptr)
            , mCurrentBlendMode(SDL_BLENDMODE_BLEND)
            , mHasCurrentClipRect(false)
            // Persistent GPU buffer state
            , mVertexGPUBuffer(nullptr)
            , mIndexGPUBuffer(nullptr)
            , mVertexGPUBufferCapacity(0)
            , mIndexGPUBufferCapacity(0)
    {
        if (mDevice != nullptr)
        {
            InitializePipelines();
        }
    }

    GPUSpriteBatch::~GPUSpriteBatch()
    {
        // Cancel any unsubmitted command buffer — shutdown doesn't need the work
        // to reach the screen.
        if (mCommandBuffer != nullptr)
        {
            mVertexData.clear();
            mIndexData.clear();
            SDL_CancelGPUCommandBuffer(mCommandBuffer);
            mCommandBuffer = nullptr;
        }

        if (mDevice != nullptr)
        {
            for (auto& [surface, entry] : mTextureCache)
            {
                if (entry.texture != nullptr)
                {
                    SDL_ReleaseGPUTexture(mDevice, entry.texture);
                }
            }
            mTextureCache.clear();

            for (auto& [mode, pipeline] : mPipelines)
            {
                if (pipeline != nullptr)
                {
                    SDL_ReleaseGPUGraphicsPipeline(mDevice, pipeline);
                }
            }
            mPipelines.clear();

            if (mSampler != nullptr)
            {
                SDL_ReleaseGPUSampler(mDevice, mSampler);
            }
            if (mWhiteTexture != nullptr)
            {
                SDL_ReleaseGPUTexture(mDevice, mWhiteTexture);
            }

            // Release persistent GPU buffers
            if (mVertexGPUBuffer != nullptr)
            {
                SDL_ReleaseGPUBuffer(mDevice, mVertexGPUBuffer);
            }
            if (mIndexGPUBuffer != nullptr)
            {
                SDL_ReleaseGPUBuffer(mDevice, mIndexGPUBuffer);
            }
        }
    }

    void GPUSpriteBatch::InitializePipelines()
    {
        mPipelines[SDL_BLENDMODE_BLEND] = CreatePipelineForBlendMode(SDL_BLENDMODE_BLEND);
        mPipelines[SDL_BLENDMODE_NONE] = CreatePipelineForBlendMode(SDL_BLENDMODE_NONE);
        mPipelines[SDL_BLENDMODE_ADD] = CreatePipelineForBlendMode(SDL_BLENDMODE_ADD);
        mPipelines[SDL_BLENDMODE_MOD] = CreatePipelineForBlendMode(SDL_BLENDMODE_MOD);
        mPipelines[SDL_BLENDMODE_MUL] = CreatePipelineForBlendMode(SDL_BLENDMODE_MUL);

        if (mDevice == nullptr)
        {
            return;
        }

        SDL_GPUSamplerCreateInfo samplerInfo{
            .min_filter = SDL_GPU_FILTER_LINEAR,
            .mag_filter = SDL_GPU_FILTER_LINEAR,
            .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        };
        mSampler = SDL_CreateGPUSampler(mDevice, &samplerInfo);

        // Create and cache the 1x1 white texture for primitive drawing.
        SDL_Surface* ws = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA32);
        if (ws != nullptr)
        {
            Uint32* p = static_cast<Uint32*>(ws->pixels);
            *p = 0xFFFFFFFF;
            EnsureCommandBuffer();
            mWhiteTexture = CreateAndUploadTexture(ws);
            SDL_DestroySurface(ws);
            // Submit the upload immediately so it doesn't bleed into the first
            // frame's command buffer (mixing copy pass with swapchain render pass).
            SDL_SubmitGPUCommandBuffer(mCommandBuffer);
            mCommandBuffer = nullptr;
        }
    }

    SDL_GPUGraphicsPipeline* GPUSpriteBatch::CreatePipelineForBlendMode(SDL_BlendMode mode)
    {
        if (mDevice == nullptr)
        {
            return nullptr;
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
            .format = SDL_GetGPUSwapchainTextureFormat(mDevice, mWindow), .blend_state = {}};

        if (mode == SDL_BLENDMODE_NONE)
        {
            colorTarget.blend_state.enable_blend = false;
        }
        else if (mode == SDL_BLENDMODE_BLEND)
        {
            colorTarget.blend_state.enable_blend = true;
            colorTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
            colorTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            colorTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
            colorTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
            colorTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            colorTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        }
        else if (mode == SDL_BLENDMODE_ADD)
        {
            colorTarget.blend_state.enable_blend = true;
            colorTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
            colorTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
            colorTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
            colorTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
            colorTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
            colorTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        }
        else if (mode == SDL_BLENDMODE_MOD)
        {
            colorTarget.blend_state.enable_blend = true;
            colorTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
            colorTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_COLOR;
            colorTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
            colorTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
            colorTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
            colorTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        }
        else if (mode == SDL_BLENDMODE_MUL)
        {
            colorTarget.blend_state.enable_blend = true;
            colorTarget.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_DST_COLOR;
            colorTarget.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
            colorTarget.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
            colorTarget.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA;
            colorTarget.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
            colorTarget.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        }

        colorTarget.blend_state.color_write_mask = 0xF;

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

        SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineInfo);

        SDL_ReleaseGPUShader(mDevice, vertShader);
        SDL_ReleaseGPUShader(mDevice, fragShader);

        return pipeline;
    }

    // Ensures mVertexGPUBuffer has at least neededBytes capacity, reallocating
    // (with 1.5x growth) if necessary.
    void GPUSpriteBatch::EnsureVertexBufferCapacity(Uint32 neededBytes)
    {
        if (neededBytes <= mVertexGPUBufferCapacity)
        {
            return;
        }

        if (mVertexGPUBuffer != nullptr)
        {
            SDL_ReleaseGPUBuffer(mDevice, mVertexGPUBuffer);
            mVertexGPUBuffer = nullptr;
        }

        Uint32 newCapacity =
            std::max(neededBytes, static_cast<Uint32>(mVertexGPUBufferCapacity * 1.5F));
        // Round up to a 256-byte boundary for alignment friendliness.
        newCapacity = (newCapacity + 255u) & ~255u;

        SDL_GPUBufferCreateInfo info{
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = newCapacity, .props = 0};
        mVertexGPUBuffer = SDL_CreateGPUBuffer(mDevice, &info);
        mVertexGPUBufferCapacity = (mVertexGPUBuffer != nullptr) ? newCapacity : 0;
    }

    void GPUSpriteBatch::EnsureIndexBufferCapacity(Uint32 neededBytes)
    {
        if (neededBytes <= mIndexGPUBufferCapacity)
        {
            return;
        }

        if (mIndexGPUBuffer != nullptr)
        {
            SDL_ReleaseGPUBuffer(mDevice, mIndexGPUBuffer);
            mIndexGPUBuffer = nullptr;
        }

        Uint32 newCapacity =
            std::max(neededBytes, static_cast<Uint32>(mIndexGPUBufferCapacity * 1.5F));
        newCapacity = (newCapacity + 255u) & ~255u;

        SDL_GPUBufferCreateInfo info{
            .usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = newCapacity, .props = 0};
        mIndexGPUBuffer = SDL_CreateGPUBuffer(mDevice, &info);
        mIndexGPUBufferCapacity = (mIndexGPUBuffer != nullptr) ? newCapacity : 0;
    }

    void GPUSpriteBatch::FlushBatch()
    {
        if (mVertexData.empty() || mCommandBuffer == nullptr || mDevice == nullptr ||
            mPipelines.empty() || mSwapchainTexture == nullptr)
        {
            return;
        }

        const Uint32 vertexBufferSize =
            static_cast<Uint32>(mVertexData.size() * sizeof(SpriteVertex));
        const Uint32 indexBufferSize = static_cast<Uint32>(mIndexData.size() * sizeof(Uint16));

        // 1. Grow persistent GPU buffers if needed.
        EnsureVertexBufferCapacity(vertexBufferSize);
        EnsureIndexBufferCapacity(indexBufferSize);

        if (mVertexGPUBuffer == nullptr || mIndexGPUBuffer == nullptr)
        {
            // Allocation failed; drop the batch rather than crash.
            mVertexData.clear();
            mIndexData.clear();
            return;
        }

        // 2. Upload via a single transfer buffer.
        SDL_GPUTransferBufferCreateInfo tbufInfo{.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                                 .size = vertexBufferSize + indexBufferSize,
                                                 .props = 0};
        SDL_GPUTransferBuffer* tbuf = SDL_CreateGPUTransferBuffer(mDevice, &tbufInfo);
        if (tbuf == nullptr)
        {
            mVertexData.clear();
            mIndexData.clear();
            return;
        }

        void* mapped = SDL_MapGPUTransferBuffer(mDevice, tbuf, false);
        SDL_memcpy(mapped, mVertexData.data(), vertexBufferSize);
        SDL_memcpy(
            static_cast<uint8_t*>(mapped) + vertexBufferSize, mIndexData.data(), indexBufferSize);
        SDL_UnmapGPUTransferBuffer(mDevice, tbuf);

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(mCommandBuffer);

        SDL_GPUTransferBufferLocation srcVert{.transfer_buffer = tbuf, .offset = 0};
        SDL_GPUBufferRegion dstVert{
            .buffer = mVertexGPUBuffer, .offset = 0, .size = vertexBufferSize};
        SDL_UploadToGPUBuffer(copyPass, &srcVert, &dstVert, false);

        SDL_GPUTransferBufferLocation srcIndex{.transfer_buffer = tbuf, .offset = vertexBufferSize};
        SDL_GPUBufferRegion dstIndex{
            .buffer = mIndexGPUBuffer, .offset = 0, .size = indexBufferSize};
        SDL_UploadToGPUBuffer(copyPass, &srcIndex, &dstIndex, false);
        SDL_EndGPUCopyPass(copyPass);

        // The transfer buffer is only needed for the copy. Release it immediately.
        SDL_ReleaseGPUTransferBuffer(mDevice, tbuf);

        // 3. Render pass
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

        SDL_GPUGraphicsPipeline* pipeline = nullptr;
        auto it = mPipelines.find(mCurrentBlendMode);
        pipeline = (it != mPipelines.end()) ? it->second : mPipelines[SDL_BLENDMODE_BLEND];

        if (pipeline != nullptr)
        {
            SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
        }

        if (mHasCurrentClipRect)
        {
            SDL_SetGPUScissor(renderPass, &mCurrentClipRect);
        }

        SDL_GPUBufferBinding vbind{.buffer = mVertexGPUBuffer, .offset = 0};
        SDL_BindGPUVertexBuffers(renderPass, 0, &vbind, 1);

        SDL_GPUBufferBinding ibind{.buffer = mIndexGPUBuffer, .offset = 0};
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
    }

    void GPUSpriteBatch::PushQuad(float x0,
                                  float y0,
                                  float x1,
                                  float y1,
                                  float x2,
                                  float y2,
                                  float x3,
                                  float y3,
                                  float u0,
                                  float v0,
                                  float u1_,
                                  float v1_,
                                  float r,
                                  float g,
                                  float b,
                                  float a)
    {
        // Guard against Uint16 overflow — flush the current batch first if we're
        // about to exceed the index range.
        if (static_cast<Uint32>(mVertexData.size()) + 4u > kMaxVerticesPerFlush)
        {
            FlushBatch();
        }

        const Uint16 base = static_cast<Uint16>(mVertexData.size());

        mVertexData.push_back({x0, y0, u0, v0, r, g, b, a});
        mVertexData.push_back({x1, y1, u1_, v0, r, g, b, a});
        mVertexData.push_back({x2, y2, u1_, v1_, r, g, b, a});
        mVertexData.push_back({x3, y3, u0, v1_, r, g, b, a});

        mIndexData.push_back(base + 0);
        mIndexData.push_back(base + 1);
        mIndexData.push_back(base + 2);
        mIndexData.push_back(base + 0);
        mIndexData.push_back(base + 2);
        mIndexData.push_back(base + 3);
    }

    void GPUSpriteBatch::DrawLine(int x1, int y1, int x2, int y2, SDL_Color color, int thickness)
    {
        if (mDevice == nullptr || mWindow == nullptr || mWhiteTexture == nullptr)
        {
            return;
        }

        const float fx1 = static_cast<float>(x1);
        const float fy1 = static_cast<float>(y1);
        const float fx2 = static_cast<float>(x2);
        const float fy2 = static_cast<float>(y2);
        const float fThickness = static_cast<float>(thickness);

        const float dx = fx2 - fx1;
        const float dy = fy2 - fy1;
        const float len = std::sqrt((dx * dx) + (dy * dy));
        if (len < 0.001F)
        {
            return;
        }

        const float wx = -dy / len * (fThickness * 0.5F);
        const float wy = dx / len * (fThickness * 0.5F);

        if (mWhiteTexture != mCurrentTexture)
        {
            FlushBatch();
            mCurrentTexture = mWhiteTexture;
        }

        const float r = color.r / 255.0F;
        const float g = color.g / 255.0F;
        const float b = color.b / 255.0F;
        const float a = color.a / 255.0F;

        PushQuad(fx1 + wx,
                 fy1 + wy,
                 fx1 - wx,
                 fy1 - wy,
                 fx2 - wx,
                 fy2 - wy,
                 fx2 + wx,
                 fy2 + wy,
                 0.0F,
                 0.0F,
                 1.0F,
                 1.0F,
                 r,
                 g,
                 b,
                 a);
    }

    void GPUSpriteBatch::FillRect(const SDL_Rect* rect, SDL_Color color)
    {
        if (mWhiteTexture == nullptr)
        {
            return;
        }

        if (mWhiteTexture != mCurrentTexture)
        {
            FlushBatch();
            mCurrentTexture = mWhiteTexture;
        }

        SDL_Rect dr;
        if (rect != nullptr)
        {
            dr = *rect;
        }
        else if (mSwapchainWidth > 0 && mSwapchainHeight > 0)
        {
            dr = SDL_Rect{.x = 0,
                          .y = 0,
                          .w = static_cast<int>(mSwapchainWidth),
                          .h = static_cast<int>(mSwapchainHeight)};
        }
        else if (mWindow != nullptr)
        {
            int w = 0;
            int h = 0;
            SDL_GetWindowSizeInPixels(mWindow, &w, &h);
            dr = SDL_Rect{.x = 0, .y = 0, .w = w, .h = h};
        }
        else
        {
            return;
        }

        const float r = color.r / 255.0F;
        const float g = color.g / 255.0F;
        const float b = color.b / 255.0F;
        const float a = color.a / 255.0F;

        const float fx = static_cast<float>(dr.x);
        const float fy = static_cast<float>(dr.y);
        const float fw = static_cast<float>(dr.w);
        const float fh = static_cast<float>(dr.h);

        PushQuad(
            fx, fy, fx + fw, fy, fx + fw, fy + fh, fx, fy + fh, 0.0F, 0.0F, 1.0F, 1.0F, r, g, b, a);
    }

    void GPUSpriteBatch::OutlineRect(const SDL_Rect* rect, SDL_Color color, int thickness)
    {
        if (rect == nullptr)
        {
            return;
        }

        DrawLine(rect->x, rect->y, rect->x + rect->w, rect->y, color, thickness);
        DrawLine(
            rect->x + rect->w, rect->y, rect->x + rect->w, rect->y + rect->h, color, thickness);
        DrawLine(
            rect->x + rect->w, rect->y + rect->h, rect->x, rect->y + rect->h, color, thickness);
        DrawLine(rect->x, rect->y + rect->h, rect->x, rect->y, color, thickness);
    }

    void GPUSpriteBatch::SetClipRect(const SDL_Rect* rect)
    {
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
        mClearColor = SDL_FColor{.r = static_cast<float>(color.r) / 255.0F,
                                 .g = static_cast<float>(color.g) / 255.0F,
                                 .b = static_cast<float>(color.b) / 255.0F,
                                 .a = static_cast<float>(color.a) / 255.0F};
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
                              const SDL_Rect* destRect,
                              const SDL_Rect* srcRect,
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

        // Texture cache lookup with stale-entry detection
        SDL_GPUTexture* texture = nullptr;
        {
            auto cacheIt = mTextureCache.find(surface);
            if (cacheIt != mTextureCache.end())
            {
                CachedTexture& entry = cacheIt->second;
                // If the surface's pixel pointer changed the surface was likely
                // re-created at the same address — invalidate the cached GPU texture.
                if (entry.pixelsSnapshot == surface->pixels)
                {
                    texture = entry.texture;
                }
                else
                {
                    SDL_ReleaseGPUTexture(mDevice, entry.texture);
                    mTextureCache.erase(cacheIt);
                }
            }
        }

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

            mTextureCache[surface] = CachedTexture{texture, surface->pixels};
        }

        SDL_BlendMode surfaceMode = SDL_BLENDMODE_BLEND;
        SDL_GetSurfaceBlendMode(surface, &surfaceMode);

        if (texture != mCurrentTexture || surfaceMode != mCurrentBlendMode)
        {
            FlushBatch();
            mCurrentTexture = texture;
            mCurrentBlendMode = surfaceMode;
        }

        SDL_Rect dr = (destRect != nullptr)
                          ? *destRect
                          : SDL_Rect{.x = 0, .y = 0, .w = surface->w, .h = surface->h};
        SDL_Rect sr = (srcRect != nullptr)
                          ? *srcRect
                          : SDL_Rect{.x = 0, .y = 0, .w = surface->w, .h = surface->h};

        const float u1 = static_cast<float>(sr.x) / static_cast<float>(surface->w);
        const float v1 = static_cast<float>(sr.y) / static_cast<float>(surface->h);
        const float u2 = static_cast<float>(sr.x + sr.w) / static_cast<float>(surface->w);
        const float v2 = static_cast<float>(sr.y + sr.h) / static_cast<float>(surface->h);

        const float r = color.r / 255.0F;
        const float g = color.g / 255.0F;
        const float b = color.b / 255.0F;
        const float a = color.a / 255.0F;

        const float fdx = static_cast<float>(dr.x);
        const float fdy = static_cast<float>(dr.y);
        const float fdw = static_cast<float>(dr.w);
        const float fdh = static_cast<float>(dr.h);

        PushQuad(fdx,
                 fdy,
                 fdx + fdw,
                 fdy,
                 fdx + fdw,
                 fdy + fdh,
                 fdx,
                 fdy + fdh,
                 u1,
                 v1,
                 u2,
                 v2,
                 r,
                 g,
                 b,
                 a);

        // Deferred until after the sprite quad is pushed so we don't break the
        // sprite's batch mid-draw. We save/restore texture+blend state so the
        // debug lines don't corrupt the next sprite's batch grouping.
        if (mDrawBounds)
        {
            SDL_GPUTexture* savedTexture = mCurrentTexture;
            SDL_BlendMode savedBlendMode = mCurrentBlendMode;

            OutlineRect(&dr, Colors::Magenta);
            if (srcRect != nullptr)
            {
                OutlineRect(&sr, Colors::Cyan);
            }

            // Restore sprite state so subsequent same-texture draws still batch.
            if (savedTexture != mCurrentTexture || savedBlendMode != mCurrentBlendMode)
            {
                FlushBatch();
                mCurrentTexture = savedTexture;
                mCurrentBlendMode = savedBlendMode;
            }
        }
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

        SDL_Rect destRect{.x = (winWidth - surface->w) / 2,
                          .y = (winHeight - surface->h) / 2,
                          .w = surface->w,
                          .h = surface->h};

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
