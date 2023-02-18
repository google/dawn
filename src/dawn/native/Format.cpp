// Copyright 2019 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/native/Format.h"

#include <bitset>

#include "dawn/native/Device.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Features.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

// Format

SampleTypeBit SampleTypeToSampleTypeBit(wgpu::TextureSampleType sampleType) {
    switch (sampleType) {
        case wgpu::TextureSampleType::Float:
        case wgpu::TextureSampleType::UnfilterableFloat:
        case wgpu::TextureSampleType::Sint:
        case wgpu::TextureSampleType::Uint:
        case wgpu::TextureSampleType::Depth:
        case wgpu::TextureSampleType::Undefined:
            // When the compiler complains that you need to add a case statement here, please
            // also add a corresponding static assert below!
            break;
    }

    static_assert(static_cast<uint32_t>(wgpu::TextureSampleType::Undefined) == 0);
    if (sampleType == wgpu::TextureSampleType::Undefined) {
        return SampleTypeBit::None;
    }

    // Check that SampleTypeBit bits are in the same position / order as the respective
    // wgpu::TextureSampleType value.
    static_assert(SampleTypeBit::Float ==
                  static_cast<SampleTypeBit>(
                      1 << (static_cast<uint32_t>(wgpu::TextureSampleType::Float) - 1)));
    static_assert(
        SampleTypeBit::UnfilterableFloat ==
        static_cast<SampleTypeBit>(
            1 << (static_cast<uint32_t>(wgpu::TextureSampleType::UnfilterableFloat) - 1)));
    static_assert(SampleTypeBit::Uint ==
                  static_cast<SampleTypeBit>(
                      1 << (static_cast<uint32_t>(wgpu::TextureSampleType::Uint) - 1)));
    static_assert(SampleTypeBit::Sint ==
                  static_cast<SampleTypeBit>(
                      1 << (static_cast<uint32_t>(wgpu::TextureSampleType::Sint) - 1)));
    static_assert(SampleTypeBit::Depth ==
                  static_cast<SampleTypeBit>(
                      1 << (static_cast<uint32_t>(wgpu::TextureSampleType::Depth) - 1)));
    return static_cast<SampleTypeBit>(1 << (static_cast<uint32_t>(sampleType) - 1));
}

bool Format::IsColor() const {
    return aspects == Aspect::Color;
}

bool Format::HasDepth() const {
    return aspects & Aspect::Depth;
}

bool Format::HasStencil() const {
    return aspects & Aspect::Stencil;
}

bool Format::HasDepthOrStencil() const {
    return aspects & (Aspect::Depth | Aspect::Stencil);
}

bool Format::IsMultiPlanar() const {
    return aspects & (Aspect::Plane0 | Aspect::Plane1);
}

bool Format::CopyCompatibleWith(const Format& format) const {
    // TODO(crbug.com/dawn/1332): Add a Format compatibility matrix.
    return baseFormat == format.baseFormat;
}

bool Format::ViewCompatibleWith(const Format& format) const {
    // TODO(crbug.com/dawn/1332): Add a Format compatibility matrix.
    return baseFormat == format.baseFormat;
}

const AspectInfo& Format::GetAspectInfo(wgpu::TextureAspect aspect) const {
    return GetAspectInfo(SelectFormatAspects(*this, aspect));
}

const AspectInfo& Format::GetAspectInfo(Aspect aspect) const {
    ASSERT(HasOneBit(aspect));
    ASSERT(aspects & aspect);
    const size_t aspectIndex = GetAspectIndex(aspect);
    ASSERT(aspectIndex < GetAspectCount(aspects));
    return aspectInfo[aspectIndex];
}

FormatIndex Format::GetIndex() const {
    return ComputeFormatIndex(format);
}

// FormatSet implementation

bool FormatSet::operator[](const Format& format) const {
    return Base::operator[](format.GetIndex());
}

typename std::bitset<kKnownFormatCount>::reference FormatSet::operator[](const Format& format) {
    return Base::operator[](format.GetIndex());
}

// Implementation details of the format table of the DeviceBase

// For the enum for formats are packed but this might change when we have a broader feature
// mechanism for webgpu.h. Formats start at 1 because 0 is the undefined format.
FormatIndex ComputeFormatIndex(wgpu::TextureFormat format) {
    // This takes advantage of overflows to make the index of TextureFormat::Undefined outside
    // of the range of the FormatTable.
    static_assert(static_cast<uint32_t>(wgpu::TextureFormat::Undefined) - 1 > kKnownFormatCount);
    return static_cast<FormatIndex>(static_cast<uint32_t>(format) - 1);
}

FormatTable BuildFormatTable(const DeviceBase* device) {
    FormatTable table;
    FormatSet formatsSet;

    static constexpr SampleTypeBit kAnyFloat =
        SampleTypeBit::Float | SampleTypeBit::UnfilterableFloat;

    auto AddFormat = [&table, &formatsSet](Format format) {
        FormatIndex index = ComputeFormatIndex(format.format);
        ASSERT(index < table.size());

        // This checks that each format is set at most once, the first part of checking that all
        // formats are set exactly once.
        ASSERT(!formatsSet[index]);

        // Vulkan describes bytesPerRow in units of texels. If there's any format for which this
        // ASSERT isn't true, then additional validation on bytesPerRow must be added.
        const bool hasMultipleAspects = !HasOneBit(format.aspects);
        ASSERT(hasMultipleAspects ||
               (kTextureBytesPerRowAlignment % format.aspectInfo[0].block.byteSize) == 0);

        table[index] = format;
        formatsSet.set(index);
    };

    auto AddColorFormat =
        [&AddFormat](wgpu::TextureFormat format, bool renderable, bool supportsStorageUsage,
                     bool supportsMultisample, bool supportsResolveTarget, uint32_t byteSize,
                     SampleTypeBit sampleTypes, uint8_t componentCount,
                     uint8_t renderTargetPixelByteCost = 0,
                     uint8_t renderTargetComponentAlignment = 0,
                     wgpu::TextureFormat baseFormat = wgpu::TextureFormat::Undefined) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.isRenderable = renderable;
            internalFormat.isCompressed = false;
            internalFormat.isSupported = true;
            internalFormat.supportsStorageUsage = supportsStorageUsage;

            if (supportsMultisample) {
                ASSERT(renderable);
            }
            internalFormat.supportsMultisample = supportsMultisample;
            internalFormat.supportsResolveTarget = supportsResolveTarget;
            internalFormat.aspects = Aspect::Color;
            internalFormat.componentCount = componentCount;
            if (renderable) {
                // If the color format is renderable, it must have a pixel byte size and component
                // alignment specified.
                ASSERT(renderTargetPixelByteCost != 0 && renderTargetComponentAlignment != 0);
                internalFormat.renderTargetPixelByteCost = renderTargetPixelByteCost;
                internalFormat.renderTargetComponentAlignment = renderTargetComponentAlignment;
            }

            // Default baseFormat of each color formats should be themselves.
            if (baseFormat == wgpu::TextureFormat::Undefined) {
                internalFormat.baseFormat = format;
            } else {
                internalFormat.baseFormat = baseFormat;
            }

            AspectInfo* firstAspect = internalFormat.aspectInfo.data();
            firstAspect->block.byteSize = byteSize;
            firstAspect->block.width = 1;
            firstAspect->block.height = 1;
            if (HasOneBit(sampleTypes)) {
                switch (sampleTypes) {
                    case SampleTypeBit::Float:
                    case SampleTypeBit::UnfilterableFloat:
                        firstAspect->baseType = wgpu::TextureComponentType::Float;
                        break;
                    case SampleTypeBit::Sint:
                        firstAspect->baseType = wgpu::TextureComponentType::Sint;
                        break;
                    case SampleTypeBit::Uint:
                        firstAspect->baseType = wgpu::TextureComponentType::Uint;
                        break;
                    default:
                        UNREACHABLE();
                }
            } else {
                ASSERT(sampleTypes & SampleTypeBit::Float);
                firstAspect->baseType = wgpu::TextureComponentType::Float;
            }
            firstAspect->supportedSampleTypes = sampleTypes;
            firstAspect->format = format;
            AddFormat(internalFormat);
        };

    auto AddDepthFormat = [&AddFormat](wgpu::TextureFormat format, uint32_t byteSize,
                                       bool isSupported) {
        Format internalFormat;
        internalFormat.format = format;
        internalFormat.baseFormat = format;
        internalFormat.isRenderable = true;
        internalFormat.isCompressed = false;
        internalFormat.isSupported = isSupported;
        internalFormat.supportsStorageUsage = false;
        internalFormat.supportsMultisample = true;
        internalFormat.supportsResolveTarget = false;
        internalFormat.aspects = Aspect::Depth;
        internalFormat.componentCount = 1;

        AspectInfo* firstAspect = internalFormat.aspectInfo.data();
        firstAspect->block.byteSize = byteSize;
        firstAspect->block.width = 1;
        firstAspect->block.height = 1;
        firstAspect->baseType = wgpu::TextureComponentType::Float;
        firstAspect->supportedSampleTypes = SampleTypeBit::Depth | SampleTypeBit::UnfilterableFloat;
        firstAspect->format = format;
        AddFormat(internalFormat);
    };

    auto AddStencilFormat = [&AddFormat](wgpu::TextureFormat format, bool isSupported) {
        Format internalFormat;
        internalFormat.format = format;
        internalFormat.baseFormat = format;
        internalFormat.isRenderable = true;
        internalFormat.isCompressed = false;
        internalFormat.isSupported = isSupported;
        internalFormat.supportsStorageUsage = false;
        internalFormat.supportsMultisample = true;
        internalFormat.supportsResolveTarget = false;
        internalFormat.aspects = Aspect::Stencil;
        internalFormat.componentCount = 1;

        // Duplicate the data for the stencil aspect in both the first and second aspect info.
        //  - aspectInfo[0] is used by AddMultiAspectFormat to copy the info for the whole
        //    stencil8 aspect of depth-stencil8 formats.
        //  - aspectInfo[1] is the actual info used in the rest of Dawn since
        //    GetAspectIndex(Aspect::Stencil) is 1.
        ASSERT(GetAspectIndex(Aspect::Stencil) == 1);

        internalFormat.aspectInfo[0].block.byteSize = 1;
        internalFormat.aspectInfo[0].block.width = 1;
        internalFormat.aspectInfo[0].block.height = 1;
        internalFormat.aspectInfo[0].baseType = wgpu::TextureComponentType::Uint;
        internalFormat.aspectInfo[0].supportedSampleTypes = SampleTypeBit::Uint;
        internalFormat.aspectInfo[0].format = format;

        internalFormat.aspectInfo[1] = internalFormat.aspectInfo[0];

        AddFormat(internalFormat);
    };

    auto AddCompressedFormat =
        [&AddFormat](wgpu::TextureFormat format, uint32_t byteSize, uint32_t width, uint32_t height,
                     bool isSupported, uint8_t componentCount,
                     wgpu::TextureFormat baseFormat = wgpu::TextureFormat::Undefined) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.isRenderable = false;
            internalFormat.isCompressed = true;
            internalFormat.isSupported = isSupported;
            internalFormat.supportsStorageUsage = false;
            internalFormat.supportsMultisample = false;
            internalFormat.supportsResolveTarget = false;
            internalFormat.aspects = Aspect::Color;
            internalFormat.componentCount = componentCount;

            // Default baseFormat of each compressed formats should be themselves.
            if (baseFormat == wgpu::TextureFormat::Undefined) {
                internalFormat.baseFormat = format;
            } else {
                internalFormat.baseFormat = baseFormat;
            }

            AspectInfo* firstAspect = internalFormat.aspectInfo.data();
            firstAspect->block.byteSize = byteSize;
            firstAspect->block.width = width;
            firstAspect->block.height = height;
            firstAspect->baseType = wgpu::TextureComponentType::Float;
            firstAspect->supportedSampleTypes = kAnyFloat;
            firstAspect->format = format;
            AddFormat(internalFormat);
        };

    auto AddMultiAspectFormat =
        [&AddFormat, &table](wgpu::TextureFormat format, Aspect aspects,
                             wgpu::TextureFormat firstFormat, wgpu::TextureFormat secondFormat,
                             bool isRenderable, bool isSupported, bool supportsMultisample,
                             uint8_t componentCount) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.baseFormat = format;
            internalFormat.isRenderable = isRenderable;
            internalFormat.isCompressed = false;
            internalFormat.isSupported = isSupported;
            internalFormat.supportsStorageUsage = false;
            internalFormat.supportsMultisample = supportsMultisample;
            internalFormat.supportsResolveTarget = false;
            internalFormat.aspects = aspects;
            internalFormat.componentCount = componentCount;

            // Multi aspect formats just copy information about single-aspect formats. This
            // means that the single-plane formats must have been added before multi-aspect
            // ones. (it is ASSERTed below).
            const FormatIndex firstFormatIndex = ComputeFormatIndex(firstFormat);
            const FormatIndex secondFormatIndex = ComputeFormatIndex(secondFormat);

            ASSERT(table[firstFormatIndex].aspectInfo[0].format != wgpu::TextureFormat::Undefined);
            ASSERT(table[secondFormatIndex].aspectInfo[0].format != wgpu::TextureFormat::Undefined);

            internalFormat.aspectInfo[0] = table[firstFormatIndex].aspectInfo[0];
            internalFormat.aspectInfo[1] = table[secondFormatIndex].aspectInfo[0];

            AddFormat(internalFormat);
        };

    // clang-format off
        // 1 byte color formats
        AddColorFormat(wgpu::TextureFormat::R8Unorm, true, false, true, true, 1, kAnyFloat, 1, 1, 1);
        AddColorFormat(wgpu::TextureFormat::R8Snorm, false, false, false, false, 1, kAnyFloat, 1);
        AddColorFormat(wgpu::TextureFormat::R8Uint, true, false, true, false, 1, SampleTypeBit::Uint, 1, 1, 1);
        AddColorFormat(wgpu::TextureFormat::R8Sint, true, false, true, false, 1, SampleTypeBit::Sint, 1, 1, 1);

        // 2 bytes color formats
        AddColorFormat(wgpu::TextureFormat::R16Uint, true, false, true, false, 2, SampleTypeBit::Uint, 1, 2, 2);
        AddColorFormat(wgpu::TextureFormat::R16Sint, true, false, true, false, 2, SampleTypeBit::Sint, 1, 2, 2);
        AddColorFormat(wgpu::TextureFormat::R16Float, true, false, true, true, 2, kAnyFloat, 1, 2, 2);
        AddColorFormat(wgpu::TextureFormat::RG8Unorm, true, false, true, true, 2, kAnyFloat, 2, 2, 1);
        AddColorFormat(wgpu::TextureFormat::RG8Snorm, false, false, false, false, 2, kAnyFloat, 2);
        AddColorFormat(wgpu::TextureFormat::RG8Uint, true, false, true, false, 2, SampleTypeBit::Uint, 2, 2, 1);
        AddColorFormat(wgpu::TextureFormat::RG8Sint, true, false, true, false, 2, SampleTypeBit::Sint, 2, 2, 1);

        // 4 bytes color formats
        AddColorFormat(wgpu::TextureFormat::R32Uint, true, true, false, false, 4, SampleTypeBit::Uint, 1, 4, 4);
        AddColorFormat(wgpu::TextureFormat::R32Sint, true, true, false, false, 4, SampleTypeBit::Sint, 1, 4, 4);
        AddColorFormat(wgpu::TextureFormat::R32Float, true, true, true, false, 4, SampleTypeBit::UnfilterableFloat, 1, 4, 4);
        AddColorFormat(wgpu::TextureFormat::RG16Uint, true, false, true, false, 4, SampleTypeBit::Uint, 2, 4, 2);
        AddColorFormat(wgpu::TextureFormat::RG16Sint, true, false, true, false, 4, SampleTypeBit::Sint, 2, 4, 2);
        AddColorFormat(wgpu::TextureFormat::RG16Float, true, false, true, true, 4, kAnyFloat, 2, 4, 2);
        AddColorFormat(wgpu::TextureFormat::RGBA8Unorm, true, true, true, true, 4, kAnyFloat, 4, 8, 1);
        AddColorFormat(wgpu::TextureFormat::RGBA8UnormSrgb, true, false, true, true, 4, kAnyFloat, 4, 8, 1, wgpu::TextureFormat::RGBA8Unorm);
        AddColorFormat(wgpu::TextureFormat::RGBA8Snorm, false, true, false, false, 4, kAnyFloat, 4);
        AddColorFormat(wgpu::TextureFormat::RGBA8Uint, true, true, true, false, 4, SampleTypeBit::Uint, 4, 4, 1);
        AddColorFormat(wgpu::TextureFormat::RGBA8Sint, true, true, true, false, 4, SampleTypeBit::Sint, 4, 4, 1);

        bool BGRA8UnormSupportsStorageUsage = device->HasFeature(Feature::BGRA8UnormStorage);
        AddColorFormat(wgpu::TextureFormat::BGRA8Unorm, true, BGRA8UnormSupportsStorageUsage, true, true, 4, kAnyFloat, 4, 8, 1);
        AddColorFormat(wgpu::TextureFormat::BGRA8UnormSrgb, true, false, true, true, 4, kAnyFloat, 4, 8, 1, wgpu::TextureFormat::BGRA8Unorm);
        AddColorFormat(wgpu::TextureFormat::RGB10A2Unorm, true, false, true, true, 4, kAnyFloat, 4, 8, 4);

        bool isRG11B10UfloatRenderable = device->HasFeature(Feature::RG11B10UfloatRenderable);
        AddColorFormat(wgpu::TextureFormat::RG11B10Ufloat, isRG11B10UfloatRenderable, false, isRG11B10UfloatRenderable, isRG11B10UfloatRenderable, 4, kAnyFloat, 3, 8, 4);
        AddColorFormat(wgpu::TextureFormat::RGB9E5Ufloat, false, false, false, false, 4, kAnyFloat, 3);

        // 8 bytes color formats
        AddColorFormat(wgpu::TextureFormat::RG32Uint, true, true, false, false, 8, SampleTypeBit::Uint, 2, 8, 4);
        AddColorFormat(wgpu::TextureFormat::RG32Sint, true, true, false, false, 8, SampleTypeBit::Sint, 2, 8, 4);
        AddColorFormat(wgpu::TextureFormat::RG32Float, true, true, false, false, 8, SampleTypeBit::UnfilterableFloat, 2, 8, 4);
        AddColorFormat(wgpu::TextureFormat::RGBA16Uint, true, true, true, false, 8, SampleTypeBit::Uint, 4, 8, 2);
        AddColorFormat(wgpu::TextureFormat::RGBA16Sint, true, true, true, false, 8, SampleTypeBit::Sint, 4, 8, 2);
        AddColorFormat(wgpu::TextureFormat::RGBA16Float, true, true, true, true, 8, kAnyFloat, 4, 8, 2);

        // 16 bytes color formats
        AddColorFormat(wgpu::TextureFormat::RGBA32Uint, true, true, false, false, 16, SampleTypeBit::Uint, 4, 16, 4);
        AddColorFormat(wgpu::TextureFormat::RGBA32Sint, true, true, false, false, 16, SampleTypeBit::Sint, 4, 16, 4);
        AddColorFormat(wgpu::TextureFormat::RGBA32Float, true, true, false, false, 16, SampleTypeBit::UnfilterableFloat, 4, 16, 4);

        // Depth-stencil formats
        AddStencilFormat(wgpu::TextureFormat::Stencil8, true);
        AddDepthFormat(wgpu::TextureFormat::Depth16Unorm, 2, true);
        // TODO(crbug.com/dawn/843): This is 4 because we read this to perform zero initialization,
        // and textures are always use depth32float. We should improve this to be more robust. Perhaps,
        // using 0 here to mean "unsized" and adding a backend-specific query for the block size.
        AddDepthFormat(wgpu::TextureFormat::Depth24Plus, 4, true);
        AddMultiAspectFormat(wgpu::TextureFormat::Depth24PlusStencil8,
                              Aspect::Depth | Aspect::Stencil, wgpu::TextureFormat::Depth24Plus, wgpu::TextureFormat::Stencil8, true, true, true, 2);
        AddDepthFormat(wgpu::TextureFormat::Depth32Float, 4, true);
        bool isD32S8Supported = device->HasFeature(Feature::Depth32FloatStencil8);
        AddMultiAspectFormat(wgpu::TextureFormat::Depth32FloatStencil8,
                              Aspect::Depth | Aspect::Stencil, wgpu::TextureFormat::Depth32Float, wgpu::TextureFormat::Stencil8, true, isD32S8Supported, true, 2);

        // BC compressed formats
        bool isBCFormatSupported = device->HasFeature(Feature::TextureCompressionBC);
        AddCompressedFormat(wgpu::TextureFormat::BC1RGBAUnorm, 8, 4, 4, isBCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::BC1RGBAUnormSrgb, 8, 4, 4, isBCFormatSupported, 4, wgpu::TextureFormat::BC1RGBAUnorm);
        AddCompressedFormat(wgpu::TextureFormat::BC4RSnorm, 8, 4, 4, isBCFormatSupported, 1);
        AddCompressedFormat(wgpu::TextureFormat::BC4RUnorm, 8, 4, 4, isBCFormatSupported, 1);
        AddCompressedFormat(wgpu::TextureFormat::BC2RGBAUnorm, 16, 4, 4, isBCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::BC2RGBAUnormSrgb, 16, 4, 4, isBCFormatSupported, 4, wgpu::TextureFormat::BC2RGBAUnorm);
        AddCompressedFormat(wgpu::TextureFormat::BC3RGBAUnorm, 16, 4, 4, isBCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::BC3RGBAUnormSrgb, 16, 4, 4, isBCFormatSupported, 4, wgpu::TextureFormat::BC3RGBAUnorm);
        AddCompressedFormat(wgpu::TextureFormat::BC5RGSnorm, 16, 4, 4, isBCFormatSupported, 2);
        AddCompressedFormat(wgpu::TextureFormat::BC5RGUnorm, 16, 4, 4, isBCFormatSupported, 2);
        AddCompressedFormat(wgpu::TextureFormat::BC6HRGBFloat, 16, 4, 4, isBCFormatSupported, 3);
        AddCompressedFormat(wgpu::TextureFormat::BC6HRGBUfloat, 16, 4, 4, isBCFormatSupported, 3);
        AddCompressedFormat(wgpu::TextureFormat::BC7RGBAUnorm, 16, 4, 4, isBCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::BC7RGBAUnormSrgb, 16, 4, 4, isBCFormatSupported, 4, wgpu::TextureFormat::BC7RGBAUnorm);

        // ETC2/EAC compressed formats
        bool isETC2FormatSupported = device->HasFeature(Feature::TextureCompressionETC2);
        AddCompressedFormat(wgpu::TextureFormat::ETC2RGB8Unorm, 8, 4, 4, isETC2FormatSupported, 3);
        AddCompressedFormat(wgpu::TextureFormat::ETC2RGB8UnormSrgb, 8, 4, 4, isETC2FormatSupported, 3, wgpu::TextureFormat::ETC2RGB8Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ETC2RGB8A1Unorm, 8, 4, 4, isETC2FormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ETC2RGB8A1UnormSrgb, 8, 4, 4, isETC2FormatSupported, 4, wgpu::TextureFormat::ETC2RGB8A1Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ETC2RGBA8Unorm, 16, 4, 4, isETC2FormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ETC2RGBA8UnormSrgb, 16, 4, 4, isETC2FormatSupported, 4, wgpu::TextureFormat::ETC2RGBA8Unorm);
        AddCompressedFormat(wgpu::TextureFormat::EACR11Unorm, 8, 4, 4, isETC2FormatSupported, 1);
        AddCompressedFormat(wgpu::TextureFormat::EACR11Snorm, 8, 4, 4, isETC2FormatSupported, 1);
        AddCompressedFormat(wgpu::TextureFormat::EACRG11Unorm, 16, 4, 4, isETC2FormatSupported, 2);
        AddCompressedFormat(wgpu::TextureFormat::EACRG11Snorm, 16, 4, 4, isETC2FormatSupported, 2);

        // ASTC compressed formats
        bool isASTCFormatSupported = device->HasFeature(Feature::TextureCompressionASTC);
        AddCompressedFormat(wgpu::TextureFormat::ASTC4x4Unorm, 16, 4, 4, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC4x4UnormSrgb, 16, 4, 4, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC4x4Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC5x4Unorm, 16, 5, 4, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC5x4UnormSrgb, 16, 5, 4, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC5x4Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC5x5Unorm, 16, 5, 5, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC5x5UnormSrgb, 16, 5, 5, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC5x5Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC6x5Unorm, 16, 6, 5, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC6x5UnormSrgb, 16, 6, 5, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC6x5Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC6x6Unorm, 16, 6, 6, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC6x6UnormSrgb, 16, 6, 6, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC6x6Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC8x5Unorm, 16, 8, 5, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC8x5UnormSrgb, 16, 8, 5, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC8x5Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC8x6Unorm, 16, 8, 6, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC8x6UnormSrgb, 16, 8, 6, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC8x6Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC8x8Unorm, 16, 8, 8, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC8x8UnormSrgb, 16, 8, 8, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC8x8Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC10x5Unorm, 16, 10, 5, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC10x5UnormSrgb, 16, 10, 5, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC10x5Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC10x6Unorm, 16, 10, 6, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC10x6UnormSrgb, 16, 10, 6, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC10x6Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC10x8Unorm, 16, 10, 8, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC10x8UnormSrgb, 16, 10, 8, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC10x8Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC10x10Unorm, 16, 10, 10, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC10x10UnormSrgb, 16, 10, 10, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC10x10Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC12x10Unorm, 16, 12, 10, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC12x10UnormSrgb, 16, 12, 10, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC12x10Unorm);
        AddCompressedFormat(wgpu::TextureFormat::ASTC12x12Unorm, 16, 12, 12, isASTCFormatSupported, 4);
        AddCompressedFormat(wgpu::TextureFormat::ASTC12x12UnormSrgb, 16, 12, 12, isASTCFormatSupported, 4, wgpu::TextureFormat::ASTC12x12Unorm);

        // multi-planar formats
        const bool isMultiPlanarFormatSupported = device->HasFeature(Feature::MultiPlanarFormats);
        AddMultiAspectFormat(wgpu::TextureFormat::R8BG8Biplanar420Unorm, Aspect::Plane0 | Aspect::Plane1,
            wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm, false, isMultiPlanarFormatSupported, false, 3);

    // clang-format on

    // This checks that each format is set at least once, the second part of checking that all
    // formats are checked exactly once. If this assertion is failing and texture formats have been
    // added or removed recently, check that kKnownFormatCount has been updated.
    ASSERT(formatsSet.all());

    return table;
}

}  // namespace dawn::native
