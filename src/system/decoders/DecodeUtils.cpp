#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "DecodeUtils.h"

#include "BlitType.h"
#include "FrameInfo.h"
#include "Sequence.h"
#include "SequenceFrameInfoList.h"
#include "model/ClassInfo.h"
#include "model/Object.h"
#include "model/PropertySerializer.h"

namespace nuvelocity
{
    constexpr const char* kSequenceClassName = "CSequence";
    constexpr const char* kSequenceFrameInfoListClassName = "CSequenceFrameInfoList";

    static SDL_Surface* BuildSequenceFrameSurface(SDL_Surface* atlas, const FrameInfo* info)
    {
        if (atlas == nullptr || info == nullptr)
        {
            return nullptr;
        }

        const int frameWidth = info->GetRight() - info->GetLeft();
        const int frameHeight = info->GetBottom() - info->GetTop();
        if (frameWidth <= 0 || frameHeight <= 0)
        {
            return DecodeUtils::BuildTransparentSurface(1, 1);
        }

        SDL_Surface* frame = DecodeUtils::BuildTransparentSurface(frameWidth, frameHeight);
        if (frame == nullptr)
        {
            return nullptr;
        }

        SDL_Rect srcRect{
            .x = info->GetLeft(), .y = info->GetTop(), .w = frameWidth, .h = frameHeight};
        SDL_Rect dstRect{.x = 0, .y = 0, .w = frameWidth, .h = frameHeight};
        SDL_BlitSurface(atlas, &srcRect, frame, &dstRect);
        return frame;
    }

    static SDL_Surface* BuildOffsetSurface(SDL_Surface* source, int offsetX, int offsetY)
    {
        if (source == nullptr)
        {
            return nullptr;
        }

        const int width = source->w + std::abs(offsetX);
        const int height = source->h + std::abs(offsetY);

        SDL_Surface* offsetSurface = DecodeUtils::BuildTransparentSurface(width, height);
        if (offsetSurface == nullptr)
        {
            return nullptr;
        }

        SDL_Rect dstRect{.x = offsetX > 0 ? offsetX : 0,
                         .y = offsetY > 0 ? offsetY : 0,
                         .w = source->w,
                         .h = source->h};
        SDL_BlitSurface(source, nullptr, offsetSurface, &dstRect);
        return offsetSurface;
    }

    static bool BuildFrameSurfaces(SDL_Surface* spriteAtlas,
                                   Sequence* sequence,
                                   const std::vector<FrameInfo*>& frameInfos,
                                   std::vector<std::unique_ptr<Frame>>& frames)
    {
        if (spriteAtlas == nullptr || sequence == nullptr)
        {
            return false;
        }

        const int baseXOffset = sequence->GetXOffset();
        const int baseYOffset = sequence->GetYOffset();

        int maxW = 0;
        int maxH = 0;

        for (size_t i = 0; i < frameInfos.size(); ++i)
        {
            const FrameInfo* frameInfo = frameInfos[i];
            if (frameInfo == nullptr)
            {
                SDL_Surface* transparent = DecodeUtils::BuildTransparentSurface(1, 1);
                SDL_SetSurfaceBlendMode(transparent, SDL_BLENDMODE_BLEND);
                frames[i] = std::make_unique<Frame>();
                frames[i]->SetSurface(transparent);
                frames[i]->SetHotSpot(0, 0);
                if (1 > maxW)
                {
                    maxW = 1;
                }
                if (1 > maxH)
                {
                    maxH = 1;
                }
                continue;
            }

            const int offsetX = baseXOffset + frameInfo->GetUpperLeftXOffset();
            const int offsetY = baseYOffset + frameInfo->GetUpperLeftYOffset();

            SDL_Surface* frameSurface = BuildSequenceFrameSurface(spriteAtlas, frameInfo);
            if (frameSurface == nullptr)
            {
                frameSurface = DecodeUtils::BuildTransparentSurface(1, 1);
            }

            if (frameSurface->w > maxW)
            {
                maxW = frameSurface->w;
            }
            if (frameSurface->h > maxH)
            {
                maxH = frameSurface->h;
            }

            SDL_SetSurfaceBlendMode(frameSurface, SDL_BLENDMODE_BLEND);
            if (sequence->GetDoDither())
            {
                DecodeUtils::DitherSurface(frameSurface);
            }
            frames[i] = std::make_unique<Frame>();
            frames[i]->SetSurface(frameSurface);
            frames[i]->SetHotSpot(offsetX, offsetY);
        }

        sequence->SetAnchor(maxW, maxH);
        for (auto& frame : frames)
        {
            if (frame)
            {
                frame->SetAnchor(maxW, maxH);
            }
        }

        return true;
    }

    void DecodeUtils::FreeDecodedBuffers(uint8_t*& listData,
                                         size_t& listDataSize,
                                         uint8_t*& imageData,
                                         size_t& imageDataSize,
                                         uint8_t*& alphaChannelData,
                                         size_t& alphaChannelDataSize)
    {
        SDL_free(listData);
        SDL_free(imageData);
        SDL_free(alphaChannelData);
        listData = nullptr;
        listDataSize = 0;
        imageData = nullptr;
        imageDataSize = 0;
        alphaChannelData = nullptr;
        alphaChannelDataSize = 0;
    }

    bool DecodeUtils::ReadChunk(SDL_IOStream* stream, size_t size, uint8_t*& chunkData)
    {
        chunkData = nullptr;
        if (size == 0)
        {
            return true;
        }

        chunkData = static_cast<uint8_t*>(SDL_malloc(size));
        if (chunkData == nullptr)
        {
            return false;
        }

        if (SDL_ReadIO(stream, chunkData, size) != size)
        {
            SDL_free(chunkData);
            chunkData = nullptr;
            return false;
        }

        return true;
    }

    bool DecodeUtils::InflateChunk(const uint8_t* compressedChunk,
                                   uint32_t compressedSize,
                                   uint32_t inflatedSize,
                                   uint8_t*& inflatedChunk,
                                   size_t& inflatedChunkSize)
    {
        inflatedChunk = nullptr;
        inflatedChunkSize = static_cast<size_t>(inflatedSize);
        if (inflatedSize == 0)
        {
            return true;
        }

        inflatedChunk = static_cast<uint8_t*>(SDL_malloc(inflatedSize));
        if (inflatedChunk == nullptr)
        {
            return false;
        }

        uint32_t compressedSizeCopy = compressedSize;
        uint32_t inflatedSizeCopy = inflatedSize;
        const int inflateResult = DecodeUtils::Inflate(
            inflatedChunk, &inflatedSizeCopy, compressedChunk, &compressedSizeCopy);
        if (inflateResult != Z_OK || inflatedSizeCopy != inflatedSize)
        {
            SDL_free(inflatedChunk);
            inflatedChunk = nullptr;
            inflatedChunkSize = 0;
            return false;
        }

        return true;
    }

    bool DecodeUtils::ReadAndInflateChunk(SDL_IOStream* stream,
                                          uint32_t compressedSize,
                                          uint32_t inflatedSize,
                                          uint8_t*& output,
                                          size_t& outputSize)
    {
        uint8_t* compressed = nullptr;
        if (!DecodeUtils::ReadChunk(stream, compressedSize, compressed))
        {
            return false;
        }

        const bool inflated =
            InflateChunk(compressed, compressedSize, inflatedSize, output, outputSize);
        SDL_free(compressed);
        return inflated;
    }

    bool DecodeUtils::ProcessSequenceListText(const std::string& listText,
                                              Sequence*& sequence,
                                              SequenceFrameInfoList*& frameInfoList,
                                              bool& hasFrameInfoList)
    {
        hasFrameInfoList = false;
        if (!DecodeUtils::DeserializeSequenceRoots(listText, sequence, frameInfoList))
        {
            return false;
        }

        if (sequence == nullptr)
        {
            sequence = new Sequence();
        }

        if (frameInfoList != nullptr)
        {
            hasFrameInfoList = true;
            frameInfoList->CopyTo(*sequence, BlitTypeRevision::Type1);
        }

        return true;
    }

    Sequence* DecodeUtils::FinalizeSequence(Sequence* sequence,
                                            SequenceFrameInfoList* frameInfoList,
                                            bool hasFrameInfoList,
                                            bool hasImageData,
                                            bool isEmpty,
                                            SDL_Surface* spriteAtlas,
                                            const std::string& rawListText)
    {
        if (!hasImageData)
        {
            if (isEmpty)
            {
                std::vector<std::unique_ptr<Frame>> emptyFrames;
                auto frame = std::make_unique<Frame>();
                frame->SetSurface(DecodeUtils::BuildTransparentSurface(1, 1));
                frame->SetAnchor(1, 1);
                emptyFrames.push_back(std::move(frame));
                sequence->SetAnchor(1, 1);
                sequence->SetFrames(std::move(emptyFrames));
            }
            delete frameInfoList;
            return sequence;
        }

        if (spriteAtlas == nullptr)
        {
            delete frameInfoList;
            delete sequence;
            return nullptr;
        }

        if (!hasFrameInfoList)
        {
            std::vector<std::unique_ptr<Frame>> frames;
            auto frame = std::make_unique<Frame>();
            const int w = spriteAtlas->w;
            const int h = spriteAtlas->h;
            frame->SetSurface(spriteAtlas);
            frame->SetAnchor(w, h);
            frames.push_back(std::move(frame));
            sequence->SetAnchor(w, h);
            sequence->SetFrames(std::move(frames));
            delete frameInfoList;
            return sequence;
        }

        if (!DecodeUtils::BuildFramesFromAtlas(sequence, frameInfoList, spriteAtlas))
        {
            delete frameInfoList;
            delete sequence;
            return nullptr;
        }

#ifdef NVE_RESTORE_TGA
        sequence->SetRawListText(rawListText);
#endif

        delete frameInfoList;
        return sequence;
    }

    bool DecodeUtils::DeserializeSequenceRoots(const std::string& listText,
                                               Sequence*& sequence,
                                               SequenceFrameInfoList*& frameInfoList)
    {
        sequence = nullptr;
        frameInfoList = nullptr;

        std::stringstream listStream(listText);
        bool sawKnownRoot = false;

        // A sequence list blob carries up to two top-level roots:
        // CSequence and CSequenceFrameInfoList.
        for (int i = 0; i < 2; ++i)
        {
            listStream >> std::ws;
            if (listStream.eof())
            {
                return sawKnownRoot;
            }

            ObjectBase* root = nullptr;
            ClassInfo* info = nullptr;
            if (!PropertySerializer::Deserialize(listStream, root, info) || root == nullptr ||
                info == nullptr)
            {
                delete root;
                return i > 0 && sawKnownRoot;
            }

            if (info->mName == kSequenceClassName)
            {
                sawKnownRoot = true;
                if (sequence == nullptr)
                {
                    sequence = static_cast<Sequence*>(root);
                }
                else
                {
                    delete root;
                }
                continue;
            }

            if (info->mName == kSequenceFrameInfoListClassName)
            {
                sawKnownRoot = true;
                if (frameInfoList == nullptr)
                {
                    frameInfoList = static_cast<SequenceFrameInfoList*>(root);
                }
                else
                {
                    delete root;
                }
                continue;
            }

            delete root;
            if (i == 0)
            {
                return false;
            }
        }

        return sawKnownRoot;
    }

    bool DecodeUtils::BuildFramesFromAtlas(Sequence* sequence,
                                           SequenceFrameInfoList* frameInfoList,
                                           SDL_Surface* spriteAtlas)
    {
        if (sequence == nullptr || frameInfoList == nullptr || spriteAtlas == nullptr)
        {
            SDL_DestroySurface(spriteAtlas);
            return false;
        }

        const std::vector<FrameInfo*>& frameInfos = frameInfoList->GetValues();
        std::vector<std::unique_ptr<Frame>> frames(frameInfos.size());

        const bool builtSurfaces = BuildFrameSurfaces(spriteAtlas, sequence, frameInfos, frames);

#ifdef NVE_RESTORE_TGA
        SDL_Surface* atlasCopy = SDL_DuplicateSurface(spriteAtlas);
#endif
        SDL_DestroySurface(spriteAtlas);

#ifdef NVE_RESTORE_TGA
        sequence->SetSpriteAtlas(atlasCopy);
#endif
        if (!builtSurfaces)
        {
            return false;
        }

        sequence->SetFrames(std::move(frames));

        return true;
    }

    SDL_Surface* DecodeUtils::BuildTransparentSurface(int width, int height)
    {
        const int safeWidth = width > 0 ? width : 1;
        const int safeHeight = height > 0 ? height : 1;

        SDL_Surface* surface = SDL_CreateSurface(safeWidth, safeHeight, SDL_PIXELFORMAT_RGBA32);
        if (surface == nullptr)
        {
            return nullptr;
        }
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

        SDL_ClearSurface(surface, 0.0F, 0.0F, 0.0F, 0.0F);
        return surface;
    }

    static int Quantize(int val, int levels)
    {
        const int q = (std::clamp(val, 0, 255) * (levels - 1) + 127) / 255;
        return (q * 255) / (levels - 1);
    }

    static void DistributeError(std::vector<int16_t>& err, int er, int x, int y, int w)
    {
        err[y * (w + 2) + x + 2] += static_cast<int16_t>((er * 7) / 16);
        err[(y + 1) * (w + 2) + x] += static_cast<int16_t>((er * 3) / 16);
        err[(y + 1) * (w + 2) + x + 1] += static_cast<int16_t>((er * 5) / 16);
        err[(y + 1) * (w + 2) + x + 2] += static_cast<int16_t>((er * 1) / 16);
    }

    void DecodeUtils::DitherSurface(SDL_Surface* surface)
    {
        if (surface == nullptr || surface->format != SDL_PIXELFORMAT_RGBA32)
        {
            return;
        }

        constexpr int kDitherLevels = 27;
        const int w = surface->w;
        const int h = surface->h;
        uint8_t* pixels = static_cast<uint8_t*>(surface->pixels);
        const int pitch = surface->pitch;

        std::vector<int16_t> rError((w + 2) * (h + 1), 0);
        std::vector<int16_t> gError((w + 2) * (h + 1), 0);
        std::vector<int16_t> bError((w + 2) * (h + 1), 0);

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                uint8_t* p = pixels + y * pitch + x * 4;

                int r = p[0] + rError[y * (w + 2) + x + 1];
                int g = p[1] + gError[y * (w + 2) + x + 1];
                int b = p[2] + bError[y * (w + 2) + x + 1];

                int nr = Quantize(r, kDitherLevels);
                int ng = Quantize(g, kDitherLevels);
                int nb = Quantize(b, kDitherLevels);

                p[0] = static_cast<uint8_t>(nr);
                p[1] = static_cast<uint8_t>(ng);
                p[2] = static_cast<uint8_t>(nb);

                DistributeError(rError, r - nr, x, y, w);
                DistributeError(gError, g - ng, x, y, w);
                DistributeError(bError, b - nb, x, y, w);
            }
        }
    }
} // namespace nuvelocity
