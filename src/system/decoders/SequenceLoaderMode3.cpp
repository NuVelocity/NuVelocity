#include <SDL3_image/SDL_image.h>
#include <string>
#include <vector>

#include "SequenceLoaderMode3.h"

#include "BlitType.h"
#include "DecodeUtils.h"
#include "SequenceFrameInfoList.h"

namespace nuvelocity
{
    constexpr uint8_t kSequenceSignatureStandard = 0x01;

    static bool ReadBlob(SDL_IOStream* stream, uint32_t size, uint8_t*& data)
    {
        data = nullptr;
        if (size == 0)
        {
            return true;
        }

        data = static_cast<uint8_t*>(SDL_malloc(size));
        if (data == nullptr)
        {
            return false;
        }

        if (SDL_ReadIO(stream, data, size) != size)
        {
            SDL_free(data);
            data = nullptr;
            return false;
        }

        return true;
    }

    static bool InflateBlob(const uint8_t* compressedData,
                            uint32_t compressedSize,
                            uint32_t inflatedSize,
                            uint8_t*& inflatedData,
                            size_t& inflatedDataSize)
    {
        inflatedData = nullptr;
        inflatedDataSize = static_cast<size_t>(inflatedSize);
        if (inflatedSize == 0)
        {
            return true;
        }

        inflatedData = static_cast<uint8_t*>(SDL_malloc(inflatedSize));
        if (inflatedData == nullptr)
        {
            return false;
        }

        uint32_t compressedSizeCopy = compressedSize;
        uint32_t inflatedSizeCopy = inflatedSize;
        const int inflateResult = DecodeUtils::Inflate(
            inflatedData, &inflatedSizeCopy, compressedData, &compressedSizeCopy);
        if (inflateResult != Z_OK || inflatedSizeCopy != inflatedSize)
        {
            SDL_free(inflatedData);
            inflatedData = nullptr;
            inflatedDataSize = 0;
            return false;
        }

        return true;
    }

    static bool DecodeInflatedSection(SDL_IOStream* stream,
                                      uint32_t compressedSize,
                                      uint32_t inflatedSize,
                                      uint8_t*& output,
                                      size_t& outputSize)
    {
        uint8_t* compressed = nullptr;
        if (!ReadBlob(stream, compressedSize, compressed))
        {
            return false;
        }

        const bool inflated =
            InflateBlob(compressed, compressedSize, inflatedSize, output, outputSize);
        SDL_free(compressed);
        return inflated;
    }

    static bool DecodeStandardCompressedImage(SDL_IOStream* stream,
                                              uint8_t*& imageData,
                                              size_t& imageDataSize,
                                              int& atlasWidth,
                                              int& atlasHeight)
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

        if (!DecodeInflatedSection(
                stream, imageDeflatedSize, imageInflatedSize, imageData, imageDataSize))
        {
            return false;
        }

        return SDL_ReadS32LE(stream, &atlasWidth) && SDL_ReadS32LE(stream, &atlasHeight);
    }

    static bool DecodeStandardUncompressedImage(SDL_IOStream* stream,
                                                uint8_t*& imageData,
                                                size_t& imageDataSize,
                                                uint8_t*& alphaChannelData,
                                                size_t& alphaChannelDataSize)
    {
        uint32_t imageSize = 0;
        if (!SDL_ReadU32LE(stream, &imageSize))
        {
            return false;
        }

        imageDataSize = static_cast<size_t>(imageSize);
        if (!ReadBlob(stream, imageSize, imageData))
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

        alphaChannelDataSize = static_cast<size_t>(maskInflatedSize);
        if (maskInflatedSize == 0)
        {
            return true;
        }

        uint8_t* compressedMask = nullptr;
        if (!ReadBlob(stream, static_cast<uint32_t>(remaining), compressedMask))
        {
            return false;
        }

        uint32_t compressedMaskSize = static_cast<uint32_t>(remaining);
        const bool decodedMask = InflateBlob(compressedMask,
                                             compressedMaskSize,
                                             maskInflatedSize,
                                             alphaChannelData,
                                             alphaChannelDataSize);
        SDL_free(compressedMask);
        return decodedMask;
    }

    Sequence* SequenceLoaderMode3::Load(SDL_IOStream* stream, FontHeaderData** outFontHeaderData)
    {
        if (outFontHeaderData != nullptr)
        {
            *outFontHeaderData = nullptr;
        }

        if (stream == nullptr)
        {
            return nullptr;
        }

        if (SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET) < 0)
        {
            return nullptr;
        }

        const bool hasStandardHeader = DecodeUtils::FrameHasDeflateHeader(stream);

        if (SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET) < 0)
        {
            return nullptr;
        }

        const bool isFontSequence =
            !hasStandardHeader && DecodeUtils::FontFrameHasDeflateHeader(stream);

        if (SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET) < 0)
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
        int atlasWidth = 0;
        int atlasHeight = 0;

        if (!DecodeSequenceStandardHeader(stream,
                                          isFontSequence,
                                          outFontHeaderData,
                                          listData,
                                          listDataSize,
                                          imageData,
                                          imageDataSize,
                                          alphaChannelData,
                                          alphaChannelDataSize,
                                          isCompressed,
                                          isEmpty,
                                          atlasWidth,
                                          atlasHeight))
        {
            DecodeUtils::FreeDecodedBuffers(listData,
                                            listDataSize,
                                            imageData,
                                            imageDataSize,
                                            alphaChannelData,
                                            alphaChannelDataSize);
            if (outFontHeaderData != nullptr)
            {
                delete *outFontHeaderData;
                *outFontHeaderData = nullptr;
            }
            return nullptr;
        }

        std::string listText(reinterpret_cast<const char*>(listData), listDataSize);

        SequenceFrameInfoList* frameInfoList = nullptr;
        Sequence* sequence = nullptr;
        bool hasFrameInfoList = false;

        if (!DecodeUtils::DeserializeSequenceRoots(listText, sequence, frameInfoList))
        {
            DecodeUtils::FreeDecodedBuffers(listData,
                                            listDataSize,
                                            imageData,
                                            imageDataSize,
                                            alphaChannelData,
                                            alphaChannelDataSize);
            if (outFontHeaderData != nullptr)
            {
                delete *outFontHeaderData;
                *outFontHeaderData = nullptr;
            }
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
                emptyFrames.push_back(DecodeUtils::BuildTransparentSurface(1, 1));
                sequence->SetFrames(std::move(emptyFrames));
            }

            DecodeUtils::FreeDecodedBuffers(listData,
                                            listDataSize,
                                            imageData,
                                            imageDataSize,
                                            alphaChannelData,
                                            alphaChannelDataSize);
            delete frameInfoList;
            return sequence;
        }

        SDL_Surface* spriteAtlas = BuildSequenceAtlasSurface(isCompressed,
                                                             atlasWidth,
                                                             atlasHeight,
                                                             imageData,
                                                             imageDataSize,
                                                             alphaChannelData,
                                                             alphaChannelDataSize);
        DecodeUtils::FreeDecodedBuffers(listData,
                                        listDataSize,
                                        imageData,
                                        imageDataSize,
                                        alphaChannelData,
                                        alphaChannelDataSize);
        if (spriteAtlas == nullptr)
        {
            delete frameInfoList;
            delete sequence;
            if (outFontHeaderData != nullptr)
            {
                delete *outFontHeaderData;
                *outFontHeaderData = nullptr;
            }
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
            if (outFontHeaderData != nullptr)
            {
                delete *outFontHeaderData;
                *outFontHeaderData = nullptr;
            }
            return nullptr;
        }

        delete frameInfoList;
        return sequence;
    }

    FontBitmap* SequenceLoaderMode3::LoadFontBitmap(SDL_IOStream* stream)
    {
        FontHeaderData* headerData = nullptr;
        Sequence* sequence = Load(stream, &headerData);
        if (sequence == nullptr || headerData == nullptr)
        {
            delete headerData;
            delete sequence;
            return nullptr;
        }

        auto* fontBitmap = new FontBitmap();
        fontBitmap->SetFirstAscii(headerData->firstAscii);
        fontBitmap->SetLastAscii(headerData->lastAscii);
        fontBitmap->SetXHeight(headerData->xHeight);
        fontBitmap->SetSequence(std::unique_ptr<Sequence>(sequence));
        delete headerData;
        return fontBitmap;
    }

    bool SequenceLoaderMode3::DecodeSequenceStandardHeader(SDL_IOStream* stream,
                                                           bool isFontSequence,
                                                           FontHeaderData** outFontHeaderData,
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
        isCompressed = false;
        isEmpty = false;
        atlasWidth = 0;
        atlasHeight = 0;
        if (outFontHeaderData != nullptr)
        {
            *outFontHeaderData = nullptr;
        }

        const auto fail = [&]()
        {
            DecodeUtils::FreeDecodedBuffers(listData,
                                            listDataSize,
                                            imageData,
                                            imageDataSize,
                                            alphaChannelData,
                                            alphaChannelDataSize);
            return false;
        };

        if (isFontSequence)
        {
            // Font sequences prepend three int32 values before the standard signature.
            if (outFontHeaderData != nullptr)
            {
                *outFontHeaderData =
                    new FontHeaderData{.firstAscii = 0, .lastAscii = 0, .xHeight = 0};
                if (!SDL_ReadS32LE(stream, &(*outFontHeaderData)->firstAscii) ||
                    !SDL_ReadS32LE(stream, &(*outFontHeaderData)->lastAscii) ||
                    !SDL_ReadS32LE(stream, &(*outFontHeaderData)->xHeight))
                {
                    delete *outFontHeaderData;
                    *outFontHeaderData = nullptr;
                    return false;
                }
            }
            else
            {
                int32_t firstAscii = 0;
                int32_t lastAscii = 0;
                int32_t xHeight = 0;
                if (!SDL_ReadS32LE(stream, &firstAscii) || !SDL_ReadS32LE(stream, &lastAscii) ||
                    !SDL_ReadS32LE(stream, &xHeight))
                {
                    return false;
                }
            }
        }

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

        if (!DecodeInflatedSection(
                stream, frameInfoDeflatedSize, frameInfoInflatedSize, listData, listDataSize))
        {
            return fail();
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
            if (!DecodeStandardCompressedImage(
                    stream, imageData, imageDataSize, atlasWidth, atlasHeight))
            {
                return fail();
            }
            return true;
        }

        if (!DecodeStandardUncompressedImage(
                stream, imageData, imageDataSize, alphaChannelData, alphaChannelDataSize))
        {
            return fail();
        }

        return true;
    }

    SDL_Surface* SequenceLoaderMode3::BuildSequenceAtlasSurface(bool isCompressed,
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

        if (isCompressed)
        {
            return BuildPlanarRgbaAtlasSurface(atlasWidth, atlasHeight, imageData, imageDataSize);
        }

        return BuildJpegAtlasSurface(
            imageData, imageDataSize, alphaChannelData, alphaChannelDataSize);
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
} // namespace nuvelocity
