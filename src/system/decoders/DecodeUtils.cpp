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

    static void DestroyFrameList(std::vector<SDL_Surface*>& frames)
    {
        for (SDL_Surface* frame : frames)
        {
            if (frame != nullptr)
            {
                SDL_DestroySurface(frame);
            }
        }
    }

    static bool BuildFrameSurfaces(SDL_Surface* spriteAtlas,
                                   const Sequence* sequence,
                                   const std::vector<FrameInfo*>& frameInfos,
                                   std::vector<SDL_Surface*>& frames,
                                   std::vector<std::pair<int, int>>& offsets,
                                   int& maxWidth,
                                   int& maxHeight,
                                   int& hotSpotX,
                                   int& hotSpotY,
                                   bool& centerHotSpot)
    {
        if (spriteAtlas == nullptr || sequence == nullptr)
        {
            return false;
        }

        const int baseXOffset = sequence->GetXOffset();
        const int baseYOffset = sequence->GetYOffset();
        centerHotSpot = sequence->GetCenterHotSpot();

        for (size_t i = 0; i < frameInfos.size(); ++i)
        {
            const FrameInfo* frameInfo = frameInfos[i];
            if (frameInfo == nullptr)
            {
                frames[i] = DecodeUtils::BuildTransparentSurface(1, 1);
                continue;
            }

            const int offsetX = baseXOffset + frameInfo->GetUpperLeftXOffset();
            const int offsetY = baseYOffset + frameInfo->GetUpperLeftYOffset();
            offsets[i] = {offsetX, offsetY};

            SDL_Surface* frameSurface = BuildSequenceFrameSurface(spriteAtlas, frameInfo);
            if (frameSurface == nullptr)
            {
                frameSurface = DecodeUtils::BuildTransparentSurface(1, 1);
            }

            if (!centerHotSpot)
            {
                SDL_Surface* offsetSurface = BuildOffsetSurface(frameSurface, offsetX, offsetY);
                SDL_DestroySurface(frameSurface);
                if (offsetSurface == nullptr)
                {
                    offsetSurface = DecodeUtils::BuildTransparentSurface(1, 1);
                }

                frames[i] = offsetSurface;
                maxWidth = std::max(maxWidth, offsetSurface->w);
                maxHeight = std::max(maxHeight, offsetSurface->h);
                continue;
            }

            const float frameWidth = static_cast<float>(frameSurface->w);
            const float frameHeight = static_cast<float>(frameSurface->h);
            const float offsetXF = static_cast<float>(offsetX);
            const float offsetYF = static_cast<float>(offsetY);

            const float deltaX = offsetXF - (frameWidth / 2.0F);
            const float deltaY = offsetYF - (frameHeight / 2.0F);

            float newWidth = frameWidth + (2.0F * std::fabs(deltaX));
            float newHeight = frameHeight + (2.0F * std::fabs(deltaY));

            if (offsetX > 0)
            {
                newWidth += frameWidth * 2.0F;
            }
            if (offsetY > 0)
            {
                newHeight += frameHeight * 2.0F;
            }

            if (newWidth >= static_cast<float>(maxWidth))
            {
                maxWidth = static_cast<int>(std::ceil(newWidth));
                hotSpotX = maxWidth / 2;
            }
            if (newHeight >= static_cast<float>(maxHeight))
            {
                maxHeight = static_cast<int>(std::ceil(newHeight));
                hotSpotY = maxHeight / 2;
            }

            frames[i] = frameSurface;
        }

        return true;
    }

    static bool PadFramesToCommonSize(std::vector<SDL_Surface*>& frames,
                                      const std::vector<std::pair<int, int>>& offsets,
                                      bool centerHotSpot,
                                      int hotSpotX,
                                      int hotSpotY,
                                      int maxWidth,
                                      int maxHeight,
                                      std::vector<SDL_Rect>& unpaddedBounds)
    {
        for (size_t i = 0; i < frames.size(); ++i)
        {
            SDL_Surface* frameSurface = frames[i];
            if (frameSurface == nullptr)
            {
                frameSurface = DecodeUtils::BuildTransparentSurface(1, 1);
            }

            const int dstX = centerHotSpot ? hotSpotX + offsets[i].first : 0;
            const int dstY = centerHotSpot ? hotSpotY + offsets[i].second : 0;

            SDL_Surface* padded = DecodeUtils::BuildTransparentSurface(maxWidth, maxHeight);
            if (padded == nullptr)
            {
                if (frameSurface != nullptr)
                {
                    SDL_DestroySurface(frameSurface);
                }
                return false;
            }

            SDL_Rect dstRect{.x = dstX, .y = dstY, .w = frameSurface->w, .h = frameSurface->h};
            SDL_BlitSurface(frameSurface, nullptr, padded, &dstRect);
            if (i < unpaddedBounds.size())
            {
                unpaddedBounds[i] =
                    SDL_Rect{.x = dstRect.x, .y = dstRect.y, .w = dstRect.w, .h = dstRect.h};
            }
            SDL_DestroySurface(frameSurface);
            frames[i] = padded;
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
                std::vector<SDL_Surface*> emptyFrames;
                emptyFrames.push_back(DecodeUtils::BuildTransparentSurface(1, 1));
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
            std::vector<SDL_Surface*> frames;
            frames.push_back(spriteAtlas);
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
        std::vector<SDL_Surface*> frames(frameInfos.size(), nullptr);
        std::vector<std::pair<int, int>> offsets(frameInfos.size(), {0, 0});
        std::vector<SDL_Rect> unpaddedBounds(frameInfos.size(),
                                             SDL_Rect{.x = 0, .y = 0, .w = 0, .h = 0});

        int maxWidth = 1;
        int maxHeight = 1;
        int hotSpotX = 0;
        int hotSpotY = 0;
        bool centerHotSpot = false;

        const bool builtSurfaces = BuildFrameSurfaces(spriteAtlas,
                                                      sequence,
                                                      frameInfos,
                                                      frames,
                                                      offsets,
                                                      maxWidth,
                                                      maxHeight,
                                                      hotSpotX,
                                                      hotSpotY,
                                                      centerHotSpot);

#ifdef NVE_RESTORE_TGA
        SDL_Surface* atlasCopy = SDL_DuplicateSurface(spriteAtlas);
#endif
        SDL_DestroySurface(spriteAtlas);

#ifdef NVE_RESTORE_TGA
        sequence->SetSpriteAtlas(atlasCopy);
#endif
        if (!builtSurfaces)
        {
            DestroyFrameList(frames);
            return false;
        }

        if (!PadFramesToCommonSize(frames,
                                   offsets,
                                   centerHotSpot,
                                   hotSpotX,
                                   hotSpotY,
                                   maxWidth,
                                   maxHeight,
                                   unpaddedBounds))
        {
            DestroyFrameList(frames);
            return false;
        }

        sequence->SetFrames(std::move(frames));

        for (std::size_t index = 0; index < unpaddedBounds.size(); ++index)
        {
            Frame* frame = sequence->GetFrame(index);
            if (frame == nullptr)
            {
                continue;
            }

            frame->mWidth = unpaddedBounds[index].w;
            frame->mHeight = unpaddedBounds[index].h;
        }

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

        SDL_ClearSurface(surface, 0.0F, 0.0F, 0.0F, 0.0F);
        return surface;
    }
} // namespace nuvelocity
