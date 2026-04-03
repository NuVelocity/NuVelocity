#include <SDL3_image/SDL_image.h>
#include <cmath>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "SequenceLoaderMode3.h"

#include "BlitType.h"
#include "FrameInfo.h"
#include "DecodeUtils.h"
#include "SequenceFrameInfoList.h"
#include "model/ClassInfo.h"
#include "model/Object.h"
#include "model/PropertySerializer.h"

namespace nuvelocity
{
    constexpr uint8_t kSequenceSignatureStandard = 0x01;
    constexpr const char* kSequenceClassName = "CSequence";
    constexpr const char* kSequenceFrameInfoListClassName = "CSequenceFrameInfoList";

    static bool DeserializeSequenceRoots(const std::string& listText, Sequence*& sequence,
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
                return i > 0 ? sawKnownRoot : false;
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

        std::vector<uint8_t> listData;
        std::vector<uint8_t> imageData;
        std::vector<uint8_t> alphaChannelData;
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

        const bool decoded = isHD
                                 ? DecodeSequenceHDHeader(stream, listData, imageData, isEmpty,
                                                          atlasWidth, atlasHeight)
                                 : DecodeSequenceStandardHeader(stream, listData, imageData,
                                                                alphaChannelData, isCompressed,
                                                                isEmpty, atlasWidth, atlasHeight);
        if (!decoded)
        {
            return nullptr;
        }

        std::string listText(reinterpret_cast<const char*>(listData.data()), listData.size());

        SequenceFrameInfoList* frameInfoList = nullptr;
        Sequence* sequence = nullptr;
        bool hasFrameInfoList = false;

        if (!DeserializeSequenceRoots(listText, sequence, frameInfoList))
        {
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

        if (imageData.empty())
        {
            if (isEmpty)
            {
                std::vector<SDL_Surface*> emptyFrames;
                emptyFrames.push_back(BuildTransparentSurface(1, 1));
                sequence->SetFrames(std::move(emptyFrames));
            }

            delete frameInfoList;
            return sequence;
        }

        SDL_Surface* spriteAtlas =
            BuildSequenceAtlasSurface(isHD, isCompressed, atlasWidth, atlasHeight, imageData,
                                      alphaChannelData);
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
                const float deltaX =
                    static_cast<float>(offsetX) - static_cast<float>(frameSurface->w) / 2.0F;
                const float deltaY =
                    static_cast<float>(offsetY) - static_cast<float>(frameSurface->h) / 2.0F;

                float newWidth =
                    static_cast<float>(frameSurface->w) + 2.0F * std::fabs(deltaX);
                float newHeight =
                    static_cast<float>(frameSurface->h) + 2.0F * std::fabs(deltaY);

                if (offsetX > 0)
                {
                    newWidth += static_cast<float>(frameSurface->w) * 2.0F;
                }
                if (offsetY > 0)
                {
                    newHeight += static_cast<float>(frameSurface->h) * 2.0F;
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
            else
            {
                SDL_Surface* offsetSurface = BuildOffsetSurface(frameSurface, offsetX, offsetY);
                SDL_DestroySurface(frameSurface);
                if (offsetSurface == nullptr)
                {
                    offsetSurface = BuildTransparentSurface(1, 1);
                }

                frames[i] = offsetSurface;
                if (offsetSurface->w > maxWidth)
                {
                    maxWidth = offsetSurface->w;
                }
                if (offsetSurface->h > maxHeight)
                {
                    maxHeight = offsetSurface->h;
                }
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

            SDL_Rect dstRect{dstX, dstY, frameSurface->w, frameSurface->h};
            SDL_BlitSurface(frameSurface, nullptr, padded, &dstRect);
            SDL_DestroySurface(frameSurface);
            frames[i] = padded;
        }

        sequence->SetFrames(std::move(frames));
        delete frameInfoList;
        return sequence;
    }

    bool SequenceLoaderMode3::DecodeSequenceStandardHeader(SDL_IOStream* stream,
                                                           std::vector<uint8_t>& listData,
                                                           std::vector<uint8_t>& imageData,
                                                           std::vector<uint8_t>& alphaChannelData,
                                                           bool& isCompressed, bool& isEmpty,
                                                           int& atlasWidth, int& atlasHeight)
    {
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

        std::vector<uint8_t> compressedList(frameInfoDeflatedSize);
        if (SDL_ReadIO(stream, compressedList.data(), frameInfoDeflatedSize) != frameInfoDeflatedSize)
        {
            return false;
        }

        listData.resize(frameInfoInflatedSize);
        uint32_t frameInfoDeflatedSizeCopy = frameInfoDeflatedSize;
        uint32_t frameInfoInflatedSizeCopy = frameInfoInflatedSize;
        if (DecodeUtils::Inflate(listData.data(), &frameInfoInflatedSizeCopy,
                     compressedList.data(), &frameInfoDeflatedSizeCopy) != Z_OK ||
            frameInfoInflatedSizeCopy != frameInfoInflatedSize)
        {
            return false;
        }

        isEmpty = SDL_TellIO(stream) >= SDL_GetIOSize(stream);
        if (isEmpty)
        {
            return true;
        }

        uint8_t compressedFlag = 0;
        if (!SDL_ReadU8(stream, &compressedFlag))
        {
            return false;
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
                return false;
            }

            std::vector<uint8_t> compressedImage(imageDeflatedSize);
            if (SDL_ReadIO(stream, compressedImage.data(), imageDeflatedSize) != imageDeflatedSize)
            {
                return false;
            }

            imageData.resize(imageInflatedSize);
            uint32_t imageDeflatedSizeCopy = imageDeflatedSize;
            uint32_t imageInflatedSizeCopy = imageInflatedSize;
            if (DecodeUtils::Inflate(imageData.data(), &imageInflatedSizeCopy,
                                     compressedImage.data(), &imageDeflatedSizeCopy) != Z_OK ||
                imageInflatedSizeCopy != imageInflatedSize)
            {
                return false;
            }

            if (!SDL_ReadS32LE(stream, &atlasWidth) || !SDL_ReadS32LE(stream, &atlasHeight))
            {
                return false;
            }

            return true;
        }

        uint32_t imageSize = 0;
        if (!SDL_ReadU32LE(stream, &imageSize))
        {
            return false;
        }
        imageData.resize(imageSize);
        if (SDL_ReadIO(stream, imageData.data(), imageSize) != imageSize)
        {
            return false;
        }

        // 1-byte padding.
        uint8_t padding = 0;
        SDL_ReadU8(stream, &padding);

        uint32_t maskInflatedSize = 0;
        if (!SDL_ReadU32LE(stream, &maskInflatedSize))
        {
            return false;
        }

        const int64_t remaining = SDL_GetIOSize(stream) - SDL_TellIO(stream);
        if (remaining < 0)
        {
            return false;
        }

        std::vector<uint8_t> compressedMask(static_cast<size_t>(remaining));
        if (remaining > 0 &&
            SDL_ReadIO(stream, compressedMask.data(), static_cast<size_t>(remaining)) !=
                static_cast<size_t>(remaining))
        {
            return false;
        }

        alphaChannelData.resize(maskInflatedSize);
        uint32_t compressedMaskSize = static_cast<uint32_t>(compressedMask.size());
        uint32_t maskInflatedSizeCopy = maskInflatedSize;
        if (maskInflatedSize > 0 &&
            (DecodeUtils::Inflate(alphaChannelData.data(), &maskInflatedSizeCopy,
                                  compressedMask.data(), &compressedMaskSize) != Z_OK ||
             maskInflatedSizeCopy != maskInflatedSize))
        {
            return false;
        }

        return true;
    }

    bool SequenceLoaderMode3::DecodeSequenceHDHeader(SDL_IOStream* stream,
                                                     std::vector<uint8_t>& listData,
                                                     std::vector<uint8_t>& imageData,
                                                     bool& isEmpty, int& atlasWidth,
                                                     int& atlasHeight)
    {
        uint32_t embeddedListsSize = 0;
        if (!SDL_ReadU32LE(stream, &embeddedListsSize))
        {
            return false;
        }

        listData.resize(embeddedListsSize);
        if (embeddedListsSize > 0 &&
            SDL_ReadIO(stream, listData.data(), embeddedListsSize) != embeddedListsSize)
        {
            return false;
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
            return false;
        }

        uint8_t ddsSignature[4] = {0, 0, 0, 0};
        if ((imageEnd - imageStart) >= 4)
        {
            if (SDL_ReadIO(stream, ddsSignature, 4) != 4)
            {
                return false;
            }
            SDL_SeekIO(stream, imageStart, SDL_IO_SEEK_SET);

            if (ddsSignature[0] == 'D' && ddsSignature[1] == 'D' && ddsSignature[2] == 'S' &&
                ddsSignature[3] == ' ')
            {
                const size_t ddsSize = static_cast<size_t>(imageEnd - imageStart);
                imageData.resize(ddsSize);
                if (SDL_ReadIO(stream, imageData.data(), ddsSize) != ddsSize)
                {
                    return false;
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
            return false;
        }

        imageData.resize(imageSize);
        if (imageSize > 0 && SDL_ReadIO(stream, imageData.data(), imageSize) != imageSize)
        {
            return false;
        }

        if (!SDL_ReadS32LE(stream, &atlasWidth) || !SDL_ReadS32LE(stream, &atlasHeight))
        {
            return false;
        }

        return true;
    }

    SDL_Surface* SequenceLoaderMode3::BuildSequenceAtlasSurface(
        bool isHD, bool isCompressed, int atlasWidth, int atlasHeight,
        const std::vector<uint8_t>& imageData, const std::vector<uint8_t>& alphaChannelData)
    {
        if (imageData.empty())
        {
            return nullptr;
        }

        if (isHD)
        {
            if (SDL_Surface* ddsSurface = TryLoadDdsSurface(imageData))
            {
                return ddsSurface;
            }
            return BuildInterleavedRgbaAtlasSurface(atlasWidth, atlasHeight, imageData);
        }

        if (isCompressed)
        {
            return BuildPlanarRgbaAtlasSurface(atlasWidth, atlasHeight, imageData);
        }

        return BuildJpegAtlasSurface(imageData, alphaChannelData);
    }

    SDL_Surface* SequenceLoaderMode3::BuildInterleavedRgbaAtlasSurface(
        int width, int height, const std::vector<uint8_t>& imageData)
    {
        if (width <= 0 || height <= 0)
        {
            return nullptr;
        }

        const size_t expectedSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4U;
        if (imageData.size() < expectedSize)
        {
            return nullptr;
        }

        SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        if (surface == nullptr)
        {
            return nullptr;
        }

        SDL_memcpy(surface->pixels, imageData.data(), expectedSize);
        return surface;
    }

    SDL_Surface* SequenceLoaderMode3::TryLoadDdsSurface(const std::vector<uint8_t>& imageData)
    {
        if (imageData.size() < 4)
        {
            return nullptr;
        }

        if (imageData[0] != 'D' || imageData[1] != 'D' || imageData[2] != 'S' ||
            imageData[3] != ' ')
        {
            return nullptr;
        }

        SDL_IOStream* ddsStream =
            SDL_IOFromConstMem(imageData.data(), static_cast<int>(imageData.size()));
        if (ddsStream == nullptr)
        {
            return nullptr;
        }

        SDL_Surface* decoded = IMG_Load_IO(ddsStream, false);
        SDL_CloseIO(ddsStream);
        return decoded;
    }

    SDL_Surface* SequenceLoaderMode3::BuildPlanarRgbaAtlasSurface(
        int width, int height, const std::vector<uint8_t>& imageData)
    {
        if (width <= 0 || height <= 0)
        {
            return nullptr;
        }

        const size_t planeSize = static_cast<size_t>(width) * static_cast<size_t>(height);
        const size_t expectedSize = planeSize * 4U;
        if (imageData.size() < expectedSize)
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
            DecodeUtils::MergeBitPlane(
                plane, plane, static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                const_cast<uint8_t*>(imageData.data()), surface);
        }

        return surface;
    }

    SDL_Surface* SequenceLoaderMode3::BuildJpegAtlasSurface(
        const std::vector<uint8_t>& imageData, const std::vector<uint8_t>& alphaChannelData)
    {
        if (imageData.empty())
        {
            return nullptr;
        }

        SDL_IOStream* imageStream =
            SDL_IOFromConstMem(imageData.data(), static_cast<int>(imageData.size()));
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

        if (alphaChannelData.empty())
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

        DecodeUtils::MergeBitPlane(0, 3, static_cast<uint32_t>(output->w),
                       static_cast<uint32_t>(output->h),
                       const_cast<uint8_t*>(alphaChannelData.data()), output);
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

        SDL_Rect srcRect{info->GetLeft(), info->GetTop(), frameWidth, frameHeight};
        SDL_Rect dstRect{0, 0, frameWidth, frameHeight};
        SDL_BlitSurface(atlas, &srcRect, frame, &dstRect);
        return frame;
    }

    SDL_Surface* SequenceLoaderMode3::BuildOffsetSurface(SDL_Surface* source, int offsetX,
                                                         int offsetY)
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

        SDL_Rect dstRect{offsetX > 0 ? offsetX : 0, offsetY > 0 ? offsetY : 0, source->w,
                         source->h};
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
