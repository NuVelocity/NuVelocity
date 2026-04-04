#include <SDL3_image/SDL_image.h>
#include <array>
#include <string>
#include <utility>
#include <vector>

#include "SequenceLoaderMode4.h"

#include "BlitType.h"
#include "SequenceFrameInfoList.h"
#include "SequenceLoaderShared.h"

namespace nuvelocity
{
    static bool ReadBlob(SDL_IOStream* stream, size_t size, uint8_t*& data)
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

    Sequence* SequenceLoaderMode4::Load(SDL_IOStream* stream)
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
        bool isEmpty = false;
        int atlasWidth = 0;
        int atlasHeight = 0;

        if (!DecodeSequenceHDHeader(stream,
                                    listData,
                                    listDataSize,
                                    imageData,
                                    imageDataSize,
                                    isEmpty,
                                    atlasWidth,
                                    atlasHeight))
        {
            SequenceLoaderShared::FreeDecodedBuffers(listData,
                                                     listDataSize,
                                                     imageData,
                                                     imageDataSize,
                                                     alphaChannelData,
                                                     alphaChannelDataSize);
            return nullptr;
        }

        std::string listText(reinterpret_cast<const char*>(listData), listDataSize);

        SequenceFrameInfoList* frameInfoList = nullptr;
        Sequence* sequence = nullptr;
        bool hasFrameInfoList = false;

        if (!SequenceLoaderShared::DeserializeSequenceRoots(listText, sequence, frameInfoList))
        {
            SequenceLoaderShared::FreeDecodedBuffers(listData,
                                                     listDataSize,
                                                     imageData,
                                                     imageDataSize,
                                                     alphaChannelData,
                                                     alphaChannelDataSize);
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
                emptyFrames.push_back(SequenceLoaderShared::BuildTransparentSurface(1, 1));
                sequence->SetFrames(std::move(emptyFrames));
            }

            SequenceLoaderShared::FreeDecodedBuffers(listData,
                                                     listDataSize,
                                                     imageData,
                                                     imageDataSize,
                                                     alphaChannelData,
                                                     alphaChannelDataSize);
            delete frameInfoList;
            return sequence;
        }

        SDL_Surface* spriteAtlas =
            BuildSequenceAtlasSurface(atlasWidth, atlasHeight, imageData, imageDataSize);
        SequenceLoaderShared::FreeDecodedBuffers(listData,
                                                 listDataSize,
                                                 imageData,
                                                 imageDataSize,
                                                 alphaChannelData,
                                                 alphaChannelDataSize);
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

        if (!SequenceLoaderShared::BuildFramesFromAtlas(sequence, frameInfoList, spriteAtlas))
        {
            delete frameInfoList;
            delete sequence;
            return nullptr;
        }

        delete frameInfoList;
        return sequence;
    }

    bool SequenceLoaderMode4::DecodeSequenceHDHeader(SDL_IOStream* stream,
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
        isEmpty = false;
        atlasWidth = 0;
        atlasHeight = 0;

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
        if (!ReadBlob(stream, listDataSize, listData))
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
                imageDataSize = static_cast<size_t>(imageEnd - imageStart);
                if (!ReadBlob(stream, imageDataSize, imageData))
                {
                    return fail();
                }
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
        if (!ReadBlob(stream, imageDataSize, imageData))
        {
            return fail();
        }

        if (!SDL_ReadS32LE(stream, &atlasWidth) || !SDL_ReadS32LE(stream, &atlasHeight))
        {
            return fail();
        }

        return true;
    }

    SDL_Surface* SequenceLoaderMode4::BuildSequenceAtlasSurface(int atlasWidth,
                                                                int atlasHeight,
                                                                const uint8_t* imageData,
                                                                size_t imageDataSize)
    {
        if (imageData == nullptr || imageDataSize == 0)
        {
            return nullptr;
        }

        if (SDL_Surface* ddsSurface = TryLoadDdsSurface(imageData, imageDataSize))
        {
            return ddsSurface;
        }

        return BuildInterleavedRgbaAtlasSurface(atlasWidth, atlasHeight, imageData, imageDataSize);
    }

    SDL_Surface* SequenceLoaderMode4::BuildInterleavedRgbaAtlasSurface(int width,
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

    SDL_Surface* SequenceLoaderMode4::TryLoadDdsSurface(const uint8_t* imageData,
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
} // namespace nuvelocity
