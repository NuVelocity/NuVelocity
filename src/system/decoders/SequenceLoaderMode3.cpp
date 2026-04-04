#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "SequenceLoaderMode3.h"

#include "BlitType.h"
#include "DecodeUtils.h"
#include "FrameInfo.h"
#include "SequenceFrameInfoList.h"
#include "model/ClassInfo.h"
#include "model/Object.h"
#include "model/PropertySerializer.h"

namespace nuvelocity
{
    constexpr uint8_t kSequenceSignatureStandard = 0x01;
    constexpr const char* kSequenceClassName = "CSequence";
    constexpr const char* kSequenceFrameInfoListClassName = "CSequenceFrameInfoList";

    static bool DeserializeSequenceRoots(const std::string& listText,
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

    Sequence* SequenceLoaderMode3::Load(SDL_IOStream* stream)
    {
        if (stream == nullptr)
        {
            return nullptr;
        }

        uint8_t* listData = nullptr;
        size_t listDataSize = 0;
        uint8_t* imageData = nullptr;
        size_t imageDataSize = 0;
        uint8_t* alphaChannelData = nullptr;
        size_t alphaChannelDataSize = 0;
        bool isCompressed = false;
        bool isEmpty = false;
        bool isHD = false;
        int atlasWidth = 0;
        int atlasHeight = 0;

        // Legacy/standard mode stores a deflate header after the first 9 bytes.
        bool hasStandardHeader = DecodeUtils::FrameHasDeflateHeader(stream);
        if (!hasStandardHeader)
        {
            // Font files store the same marker at a different offset. If neither marker
            // is found we assume this is an HD sequence.
            const bool hasFontLikeHeader = DecodeUtils::FontFrameHasDeflateHeader(stream);
            isHD = !hasFontLikeHeader;
        }

        const bool decoded = isHD ? DecodeSequenceHDHeader(stream,
                                                           listData,
                                                           listDataSize,
                                                           imageData,
                                                           imageDataSize,
                                                           isEmpty,
                                                           atlasWidth,
                                                           atlasHeight)
                                  : DecodeSequenceStandardHeader(stream,
                                                                 listData,
                                                                 listDataSize,
                                                                 imageData,
                                                                 imageDataSize,
                                                                 alphaChannelData,
                                                                 alphaChannelDataSize,
                                                                 isCompressed,
                                                                 isEmpty,
                                                                 atlasWidth,
                                                                 atlasHeight);
        if (!decoded)
        {
            SDL_free(listData);
            SDL_free(imageData);
            SDL_free(alphaChannelData);
            return nullptr;
        }

        std::string listText(reinterpret_cast<const char*>(listData), listDataSize);

        SequenceFrameInfoList* frameInfoList = nullptr;
        Sequence* sequence = nullptr;
        bool hasFrameInfoList = false;

        if (!DeserializeSequenceRoots(listText, sequence, frameInfoList))
        {
            SDL_free(listData);
            SDL_free(imageData);
            SDL_free(alphaChannelData);
            return nullptr;
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

        if (imageDataSize == 0)
        {
            if (isEmpty)
            {
                std::vector<SDL_Surface*> emptyFrames;
                emptyFrames.push_back(BuildTransparentSurface(1, 1));
                sequence->SetFrames(std::move(emptyFrames));
            }

            SDL_free(listData);
            SDL_free(imageData);
            SDL_free(alphaChannelData);
            delete frameInfoList;
            return sequence;
        }

        SDL_Surface* spriteAtlas = BuildSequenceAtlasSurface(isHD,
                                                             isCompressed,
                                                             atlasWidth,
                                                             atlasHeight,
                                                             imageData,
                                                             imageDataSize,
                                                             alphaChannelData,
                                                             alphaChannelDataSize);
        SDL_free(listData);
        SDL_free(imageData);
        SDL_free(alphaChannelData);
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

        const std::vector<FrameInfo*>& frameInfos = frameInfoList->GetValues();
        std::vector<SDL_Surface*> frames(frameInfos.size(), nullptr);
        std::vector<std::pair<int, int>> offsets(frameInfos.size(), {0, 0});

        int maxWidth = 1;
        int maxHeight = 1;
        int hotSpotX = 0;
        int hotSpotY = 0;

        const int baseXOffset = sequence->GetXOffset();
        const int baseYOffset = sequence->GetYOffset();
        const bool centerHotSpot = sequence->GetCenterHotSpot();

        for (size_t i = 0; i < frameInfos.size(); ++i)
        {
            const FrameInfo* frameInfo = frameInfos[i];
            if (frameInfo == nullptr)
            {
                frames[i] = BuildTransparentSurface(1, 1);
                continue;
            }

            const int offsetX = baseXOffset + frameInfo->GetUpperLeftXOffset();
            const int offsetY = baseYOffset + frameInfo->GetUpperLeftYOffset();
            offsets[i] = {offsetX, offsetY};

            SDL_Surface* frameSurface = BuildSequenceFrameSurface(spriteAtlas, frameInfo);
            if (frameSurface == nullptr)
            {
                frameSurface = BuildTransparentSurface(1, 1);
            }

            if (centerHotSpot)
            {
                const float frameWidth = static_cast<float>(frameSurface->w);
                const float frameHeight = static_cast<float>(frameSurface->h);
                const float offsetXF = static_cast<float>(offsetX);
                const float offsetYF = static_cast<float>(offsetY);

                const float deltaX = offsetXF - (frameWidth / 2);
                const float deltaY = offsetYF - (frameHeight / 2);

                float newWidth = frameWidth + (2 * std::fabs(deltaX));
                float newHeight = frameHeight + (2 * std::fabs(deltaY));

                if (offsetX > 0)
                {
                    newWidth += frameWidth * 2;
                }
                if (offsetY > 0)
                {
                    newHeight += frameHeight * 2;
                }

                if (newWidth >= maxWidth)
                {
                    maxWidth = static_cast<int>(std::ceil(newWidth));
                    hotSpotX = maxWidth / 2;
                }
                if (newHeight >= maxHeight)
                {
                    maxHeight = static_cast<int>(std::ceil(newHeight));
                    hotSpotY = maxHeight / 2;
                }

                frames[i] = frameSurface;
            }
            else
            {
                SDL_Surface* offsetSurface = BuildOffsetSurface(frameSurface, offsetX, offsetY);
                SDL_DestroySurface(frameSurface);
                if (offsetSurface == nullptr)
                {
                    offsetSurface = BuildTransparentSurface(1, 1);
                }

                frames[i] = offsetSurface;
                maxWidth = std::max(maxWidth, offsetSurface->w);
                maxHeight = std::max(maxHeight, offsetSurface->h);
            }
        }

        SDL_DestroySurface(spriteAtlas);

        for (size_t i = 0; i < frames.size(); ++i)
        {
            SDL_Surface* frameSurface = frames[i];
            if (frameSurface == nullptr)
            {
                frameSurface = BuildTransparentSurface(1, 1);
            }

            const int dstX = centerHotSpot ? hotSpotX + offsets[i].first : 0;
            const int dstY = centerHotSpot ? hotSpotY + offsets[i].second : 0;

            SDL_Surface* padded = BuildTransparentSurface(maxWidth, maxHeight);
            if (padded == nullptr)
            {
                for (SDL_Surface* cleanup : frames)
                {
                    if (cleanup != nullptr)
                    {
                        SDL_DestroySurface(cleanup);
                    }
                }
                delete frameInfoList;
                delete sequence;
                return nullptr;
            }

            SDL_Rect dstRect{.x = dstX, .y = dstY, .w = frameSurface->w, .h = frameSurface->h};
            SDL_BlitSurface(frameSurface, nullptr, padded, &dstRect);
            SDL_DestroySurface(frameSurface);
            frames[i] = padded;
        }

        sequence->SetFrames(std::move(frames));
        delete frameInfoList;
        return sequence;
    }

    bool SequenceLoaderMode3::DecodeSequenceStandardHeader(SDL_IOStream* stream,
                                                           uint8_t*& listData,
                                                           size_t& listDataSize,
                                                           uint8_t*& imageData,
                                                           size_t& imageDataSize,
                                                           uint8_t*& alphaChannelData,
                                                           size_t& alphaChannelDataSize,
                                                           bool& isCompressed,
                                                           bool& isEmpty,
                                                           int& atlasWidth,
                                                           int& atlasHeight)
    {
        listData = nullptr;
        listDataSize = 0;
        imageData = nullptr;
        imageDataSize = 0;
        alphaChannelData = nullptr;
        alphaChannelDataSize = 0;

        const auto fail = [&]()
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
            return false;
        };

        uint8_t signature = 0;
        if (!SDL_ReadU8(stream, &signature) || signature != kSequenceSignatureStandard)
        {
            return false;
        }

        uint32_t frameInfoDeflatedSize = 0;
        uint32_t frameInfoInflatedSize = 0;
        if (!SDL_ReadU32LE(stream, &frameInfoDeflatedSize) ||
            !SDL_ReadU32LE(stream, &frameInfoInflatedSize))
        {
            return false;
        }

        uint8_t* compressedList = nullptr;
        if (frameInfoDeflatedSize > 0)
        {
            compressedList = static_cast<uint8_t*>(SDL_malloc(frameInfoDeflatedSize));
            if (compressedList == nullptr)
            {
                return false;
            }

            if (SDL_ReadIO(stream, compressedList, frameInfoDeflatedSize) != frameInfoDeflatedSize)
            {
                SDL_free(compressedList);
                return false;
            }
        }

        listDataSize = static_cast<size_t>(frameInfoInflatedSize);
        if (frameInfoInflatedSize > 0)
        {
            listData = static_cast<uint8_t*>(SDL_malloc(frameInfoInflatedSize));
            if (listData == nullptr)
            {
                SDL_free(compressedList);
                return false;
            }

            uint32_t frameInfoDeflatedSizeCopy = frameInfoDeflatedSize;
            uint32_t frameInfoInflatedSizeCopy = frameInfoInflatedSize;
            const int inflateResult = DecodeUtils::Inflate(
                listData, &frameInfoInflatedSizeCopy, compressedList, &frameInfoDeflatedSizeCopy);
            SDL_free(compressedList);
            if (inflateResult != Z_OK || frameInfoInflatedSizeCopy != frameInfoInflatedSize)
            {
                return fail();
            }
        }
        else
        {
            SDL_free(compressedList);
        }

        isEmpty = SDL_TellIO(stream) >= SDL_GetIOSize(stream);
        if (isEmpty)
        {
            return true;
        }

        uint8_t compressedFlag = 0;
        if (!SDL_ReadU8(stream, &compressedFlag))
        {
            return fail();
        }
        isCompressed = compressedFlag != 0U;

        if (isCompressed)
        {
            // Unknown metadata byte.
            uint8_t scan1 = 0;
            SDL_ReadU8(stream, &scan1);

            uint32_t imageDeflatedSize = 0;
            uint32_t imageInflatedSize = 0;
            if (!SDL_ReadU32LE(stream, &imageDeflatedSize) ||
                !SDL_ReadU32LE(stream, &imageInflatedSize))
            {
                return fail();
            }

            uint8_t* compressedImage = nullptr;
            if (imageDeflatedSize > 0)
            {
                compressedImage = static_cast<uint8_t*>(SDL_malloc(imageDeflatedSize));
                if (compressedImage == nullptr)
                {
                    return fail();
                }

                if (SDL_ReadIO(stream, compressedImage, imageDeflatedSize) != imageDeflatedSize)
                {
                    SDL_free(compressedImage);
                    return fail();
                }
            }

            imageDataSize = static_cast<size_t>(imageInflatedSize);
            if (imageInflatedSize > 0)
            {
                imageData = static_cast<uint8_t*>(SDL_malloc(imageInflatedSize));
                if (imageData == nullptr)
                {
                    SDL_free(compressedImage);
                    return fail();
                }

                uint32_t imageDeflatedSizeCopy = imageDeflatedSize;
                uint32_t imageInflatedSizeCopy = imageInflatedSize;
                const int inflateResult = DecodeUtils::Inflate(
                    imageData, &imageInflatedSizeCopy, compressedImage, &imageDeflatedSizeCopy);
                SDL_free(compressedImage);
                if (inflateResult != Z_OK || imageInflatedSizeCopy != imageInflatedSize)
                {
                    return fail();
                }
            }
            else
            {
                SDL_free(compressedImage);
            }

            if (!SDL_ReadS32LE(stream, &atlasWidth) || !SDL_ReadS32LE(stream, &atlasHeight))
            {
                return fail();
            }

            return true;
        }

        uint32_t imageSize = 0;
        if (!SDL_ReadU32LE(stream, &imageSize))
        {
            return fail();
        }

        imageDataSize = static_cast<size_t>(imageSize);
        if (imageSize > 0)
        {
            imageData = static_cast<uint8_t*>(SDL_malloc(imageSize));
            if (imageData == nullptr)
            {
                return fail();
            }

            if (SDL_ReadIO(stream, imageData, imageSize) != imageSize)
            {
                return fail();
            }
        }

        // 1-byte padding.
        uint8_t padding = 0;
        SDL_ReadU8(stream, &padding);

        uint32_t maskInflatedSize = 0;
        if (!SDL_ReadU32LE(stream, &maskInflatedSize))
        {
            return fail();
        }

        const int64_t remaining = SDL_GetIOSize(stream) - SDL_TellIO(stream);
        if (remaining < 0)
        {
            return fail();
        }

        uint8_t* compressedMask = nullptr;
        if (remaining > 0)
        {
            compressedMask = static_cast<uint8_t*>(SDL_malloc(static_cast<size_t>(remaining)));
            if (compressedMask == nullptr)
            {
                return fail();
            }

            if (SDL_ReadIO(stream, compressedMask, static_cast<size_t>(remaining)) !=
                static_cast<size_t>(remaining))
            {
                SDL_free(compressedMask);
                return fail();
            }
        }

        alphaChannelDataSize = static_cast<size_t>(maskInflatedSize);
        if (maskInflatedSize == 0)
        {
            SDL_free(compressedMask);
            return true;
        }

        alphaChannelData = static_cast<uint8_t*>(SDL_malloc(maskInflatedSize));
        if (alphaChannelData == nullptr)
        {
            SDL_free(compressedMask);
            return fail();
        }

        uint32_t compressedMaskSize = static_cast<uint32_t>(remaining);
        uint32_t maskInflatedSizeCopy = maskInflatedSize;
        const int inflateResult = DecodeUtils::Inflate(
            alphaChannelData, &maskInflatedSizeCopy, compressedMask, &compressedMaskSize);
        SDL_free(compressedMask);

        if (inflateResult != Z_OK || maskInflatedSizeCopy != maskInflatedSize)
        {
            return fail();
        }

        return true;
    }

    bool SequenceLoaderMode3::DecodeSequenceHDHeader(SDL_IOStream* stream,
                                                     uint8_t*& listData,
                                                     size_t& listDataSize,
                                                     uint8_t*& imageData,
                                                     size_t& imageDataSize,
                                                     bool& isEmpty,
                                                     int& atlasWidth,
                                                     int& atlasHeight)
    {
        listData = nullptr;
        listDataSize = 0;
        imageData = nullptr;
        imageDataSize = 0;

        const auto fail = [&]()
        {
            SDL_free(listData);
            SDL_free(imageData);
            listData = nullptr;
            listDataSize = 0;
            imageData = nullptr;
            imageDataSize = 0;
            return false;
        };

        uint32_t embeddedListsSize = 0;
        if (!SDL_ReadU32LE(stream, &embeddedListsSize))
        {
            return false;
        }

        listDataSize = static_cast<size_t>(embeddedListsSize);
        if (embeddedListsSize > 0)
        {
            listData = static_cast<uint8_t*>(SDL_malloc(embeddedListsSize));
            if (listData == nullptr)
            {
                return false;
            }

            if (SDL_ReadIO(stream, listData, embeddedListsSize) != embeddedListsSize)
            {
                return fail();
            }
        }

        isEmpty = SDL_TellIO(stream) >= SDL_GetIOSize(stream);
        if (isEmpty)
        {
            return true;
        }

        const int64_t imageStart = SDL_TellIO(stream);
        const int64_t imageEnd = SDL_GetIOSize(stream);
        if (imageStart < 0 || imageEnd < imageStart)
        {
            return fail();
        }

        std::array<uint8_t, 4> ddsSignature{0, 0, 0, 0};
        if ((imageEnd - imageStart) >= 4)
        {
            if (SDL_ReadIO(stream, ddsSignature.data(), 4) != 4)
            {
                return fail();
            }
            SDL_SeekIO(stream, imageStart, SDL_IO_SEEK_SET);

            if (ddsSignature[0] == 'D' && ddsSignature[1] == 'D' && ddsSignature[2] == 'S' &&
                ddsSignature[3] == ' ')
            {
                const size_t ddsSize = static_cast<size_t>(imageEnd - imageStart);
                imageDataSize = ddsSize;
                if (ddsSize > 0)
                {
                    imageData = static_cast<uint8_t*>(SDL_malloc(ddsSize));
                    if (imageData == nullptr)
                    {
                        return fail();
                    }

                    if (SDL_ReadIO(stream, imageData, ddsSize) != ddsSize)
                    {
                        return fail();
                    }
                }

                atlasWidth = 0;
                atlasHeight = 0;
                return true;
            }
        }

        // Unknown metadata byte.
        uint8_t scan2 = 0;
        SDL_ReadU8(stream, &scan2);

        uint32_t imageSize = 0;
        if (!SDL_ReadU32LE(stream, &imageSize))
        {
            return fail();
        }

        imageDataSize = static_cast<size_t>(imageSize);
        if (imageSize > 0)
        {
            imageData = static_cast<uint8_t*>(SDL_malloc(imageSize));
            if (imageData == nullptr)
            {
                return fail();
            }

            if (SDL_ReadIO(stream, imageData, imageSize) != imageSize)
            {
                return fail();
            }
        }

        if (!SDL_ReadS32LE(stream, &atlasWidth) || !SDL_ReadS32LE(stream, &atlasHeight))
        {
            return fail();
        }

        return true;
    }

    SDL_Surface* SequenceLoaderMode3::BuildSequenceAtlasSurface(bool isHD,
                                                                bool isCompressed,
                                                                int atlasWidth,
                                                                int atlasHeight,
                                                                const uint8_t* imageData,
                                                                size_t imageDataSize,
                                                                const uint8_t* alphaChannelData,
                                                                size_t alphaChannelDataSize)
    {
        if (imageData == nullptr || imageDataSize == 0)
        {
            return nullptr;
        }

        if (isHD)
        {
            if (SDL_Surface* ddsSurface = TryLoadDdsSurface(imageData, imageDataSize))
            {
                return ddsSurface;
            }
            return BuildInterleavedRgbaAtlasSurface(
                atlasWidth, atlasHeight, imageData, imageDataSize);
        }

        if (isCompressed)
        {
            return BuildPlanarRgbaAtlasSurface(atlasWidth, atlasHeight, imageData, imageDataSize);
        }

        return BuildJpegAtlasSurface(
            imageData, imageDataSize, alphaChannelData, alphaChannelDataSize);
    }

    SDL_Surface* SequenceLoaderMode3::BuildInterleavedRgbaAtlasSurface(int width,
                                                                       int height,
                                                                       const uint8_t* imageData,
                                                                       size_t imageDataSize)
    {
        if (width <= 0 || height <= 0)
        {
            return nullptr;
        }

        const size_t expectedSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4U;
        if (imageDataSize < expectedSize)
        {
            return nullptr;
        }

        SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        if (surface == nullptr)
        {
            return nullptr;
        }

        SDL_memcpy(surface->pixels, imageData, expectedSize);
        return surface;
    }

    SDL_Surface* SequenceLoaderMode3::TryLoadDdsSurface(const uint8_t* imageData,
                                                        size_t imageDataSize)
    {
        if (imageData == nullptr || imageDataSize < 4)
        {
            return nullptr;
        }

        if (imageData[0] != 'D' || imageData[1] != 'D' || imageData[2] != 'S' ||
            imageData[3] != ' ')
        {
            return nullptr;
        }

        SDL_IOStream* ddsStream = SDL_IOFromConstMem(imageData, static_cast<int>(imageDataSize));
        if (ddsStream == nullptr)
        {
            return nullptr;
        }

        SDL_Surface* decoded = IMG_Load_IO(ddsStream, false);
        SDL_CloseIO(ddsStream);
        return decoded;
    }

    SDL_Surface* SequenceLoaderMode3::BuildPlanarRgbaAtlasSurface(int width,
                                                                  int height,
                                                                  const uint8_t* imageData,
                                                                  size_t imageDataSize)
    {
        if (width <= 0 || height <= 0)
        {
            return nullptr;
        }

        const size_t planeSize = static_cast<size_t>(width) * static_cast<size_t>(height);
        const size_t expectedSize = planeSize * 4U;
        if (imageDataSize < expectedSize)
        {
            return nullptr;
        }

        SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        if (surface == nullptr)
        {
            return nullptr;
        }

        for (int plane = 0; plane < 4; ++plane)
        {
            DecodeUtils::MergeBitPlane(plane,
                                       plane,
                                       static_cast<uint32_t>(width),
                                       static_cast<uint32_t>(height),
                                       const_cast<uint8_t*>(imageData),
                                       surface);
        }

        return surface;
    }

    SDL_Surface* SequenceLoaderMode3::BuildJpegAtlasSurface(const uint8_t* imageData,
                                                            size_t imageDataSize,
                                                            const uint8_t* alphaChannelData,
                                                            size_t alphaChannelDataSize)
    {
        if (imageData == nullptr || imageDataSize == 0)
        {
            return nullptr;
        }

        SDL_IOStream* imageStream = SDL_IOFromConstMem(imageData, static_cast<int>(imageDataSize));
        if (imageStream == nullptr)
        {
            return nullptr;
        }

        SDL_Surface* loaded = IMG_LoadJPG_IO(imageStream);
        SDL_CloseIO(imageStream);
        if (loaded == nullptr)
        {
            return nullptr;
        }

        if (alphaChannelData == nullptr || alphaChannelDataSize == 0)
        {
            return loaded;
        }

        SDL_Surface* output = SDL_CreateSurface(loaded->w, loaded->h, SDL_PIXELFORMAT_RGBA32);
        if (output == nullptr)
        {
            SDL_DestroySurface(loaded);
            return nullptr;
        }

        SDL_BlitSurface(loaded, nullptr, output, nullptr);
        SDL_DestroySurface(loaded);

        DecodeUtils::MergeBitPlane(0,
                                   3,
                                   static_cast<uint32_t>(output->w),
                                   static_cast<uint32_t>(output->h),
                                   const_cast<uint8_t*>(alphaChannelData),
                                   output);
        return output;
    }

    SDL_Surface* SequenceLoaderMode3::BuildSequenceFrameSurface(SDL_Surface* atlas,
                                                                const FrameInfo* info)
    {
        if (atlas == nullptr || info == nullptr)
        {
            return nullptr;
        }

        const int frameWidth = info->GetRight() - info->GetLeft();
        const int frameHeight = info->GetBottom() - info->GetTop();
        if (frameWidth <= 0 || frameHeight <= 0)
        {
            return BuildTransparentSurface(1, 1);
        }

        SDL_Surface* frame = BuildTransparentSurface(frameWidth, frameHeight);
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

    SDL_Surface*
    SequenceLoaderMode3::BuildOffsetSurface(SDL_Surface* source, int offsetX, int offsetY)
    {
        if (source == nullptr)
        {
            return nullptr;
        }

        const int width = source->w + std::abs(offsetX);
        const int height = source->h + std::abs(offsetY);

        SDL_Surface* offsetSurface = BuildTransparentSurface(width, height);
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

    SDL_Surface* SequenceLoaderMode3::BuildTransparentSurface(int width, int height)
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
