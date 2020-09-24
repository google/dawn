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

#include "dawn_native/Format.h"

#include "dawn_native/Device.h"
#include "dawn_native/Extensions.h"
#include "dawn_native/Texture.h"

#include <bitset>

namespace dawn_native {

    // Format

    // static
    Format::Type Format::TextureComponentTypeToFormatType(
        wgpu::TextureComponentType componentType) {
        switch (componentType) {
            case wgpu::TextureComponentType::Float:
            case wgpu::TextureComponentType::Sint:
            case wgpu::TextureComponentType::Uint:
                break;
        }
        // Check that Type correctly mirrors TextureComponentType except for "Other".
        static_assert(static_cast<Type>(wgpu::TextureComponentType::Float) == Type::Float, "");
        static_assert(static_cast<Type>(wgpu::TextureComponentType::Sint) == Type::Sint, "");
        static_assert(static_cast<Type>(wgpu::TextureComponentType::Uint) == Type::Uint, "");
        return static_cast<Type>(componentType);
    }

    // static
    wgpu::TextureComponentType Format::FormatTypeToTextureComponentType(Type type) {
        switch (type) {
            case Type::Float:
            case Type::Sint:
            case Type::Uint:
                break;

            case Type::Other:
                UNREACHABLE();
        }
        // Check that Type correctly mirrors TextureComponentType except for "Other".
        static_assert(static_cast<Type>(wgpu::TextureComponentType::Float) == Type::Float, "");
        static_assert(static_cast<Type>(wgpu::TextureComponentType::Sint) == Type::Sint, "");
        static_assert(static_cast<Type>(wgpu::TextureComponentType::Uint) == Type::Uint, "");
        return static_cast<wgpu::TextureComponentType>(type);
    }

    bool Format::IsColor() const {
        return aspects == Aspect::Color;
    }

    bool Format::HasDepth() const {
        return (aspects & Aspect::Depth) != 0;
    }

    bool Format::HasStencil() const {
        return (aspects & Aspect::Stencil) != 0;
    }

    bool Format::HasDepthOrStencil() const {
        return (aspects & (Aspect::Depth | Aspect::Stencil)) != 0;
    }

    bool Format::HasComponentType(Type componentType) const {
        return componentType == type;
    }

    TexelBlockInfo Format::GetTexelBlockInfo(wgpu::TextureAspect aspect) const {
        switch (aspect) {
            case wgpu::TextureAspect::All:
                return blockInfo;

            case wgpu::TextureAspect::DepthOnly:
                ASSERT(HasDepth());
                switch (format) {
                    case wgpu::TextureFormat::Depth32Float:
                        return blockInfo;
                    default:
                        UNREACHABLE();
                        break;
                }
                break;

            case wgpu::TextureAspect::StencilOnly:
                ASSERT(HasStencil());
                switch (format) {
                    case wgpu::TextureFormat::Depth24PlusStencil8:
                        return {1, 1, 1};
                    default:
                        UNREACHABLE();
                        break;
                }
                break;
        }
    }

    TexelBlockInfo Format::GetTexelBlockInfo(Aspect aspect) const {
        ASSERT(HasOneBit(aspect));
        ASSERT(aspects & aspect);
        switch (aspect) {
            case Aspect::Color:
                ASSERT(aspects == aspect);
                return blockInfo;
            case Aspect::Depth:
                switch (format) {
                    case wgpu::TextureFormat::Depth32Float:
                        return blockInfo;
                    default:
                        UNREACHABLE();
                        break;
                }
            case Aspect::Stencil:
                switch (format) {
                    case wgpu::TextureFormat::Depth24PlusStencil8:
                        return {1, 1, 1};
                    default:
                        UNREACHABLE();
                        break;
                }
                break;

            case Aspect::None:
                UNREACHABLE();
        }
    }

    size_t Format::GetIndex() const {
        return ComputeFormatIndex(format);
    }

    // Implementation details of the format table of the DeviceBase

    // For the enum for formats are packed but this might change when we have a broader extension
    // mechanism for webgpu.h. Formats start at 1 because 0 is the undefined format.
    size_t ComputeFormatIndex(wgpu::TextureFormat format) {
        // This takes advantage of overflows to make the index of TextureFormat::Undefined outside
        // of the range of the FormatTable.
        static_assert(static_cast<uint32_t>(wgpu::TextureFormat::Undefined) - 1 > kKnownFormatCount,
                      "");
        return static_cast<size_t>(static_cast<uint32_t>(format) - 1);
    }

    FormatTable BuildFormatTable(const DeviceBase* device) {
        FormatTable table;
        std::bitset<kKnownFormatCount> formatsSet;

        using Type = Format::Type;

        auto AddFormat = [&table, &formatsSet](Format format) {
            size_t index = ComputeFormatIndex(format.format);
            ASSERT(index < table.size());

            // This checks that each format is set at most once, the first part of checking that all
            // formats are set exactly once.
            ASSERT(!formatsSet[index]);

            table[index] = format;
            formatsSet.set(index);
        };

        auto AddColorFormat = [&AddFormat](wgpu::TextureFormat format, bool renderable,
                                           bool supportsStorageUsage, uint32_t byteSize,
                                           Type type) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.isRenderable = renderable;
            internalFormat.isCompressed = false;
            internalFormat.isSupported = true;
            internalFormat.supportsStorageUsage = supportsStorageUsage;
            internalFormat.aspects = Aspect::Color;
            internalFormat.type = type;
            internalFormat.blockInfo.blockByteSize = byteSize;
            internalFormat.blockInfo.blockWidth = 1;
            internalFormat.blockInfo.blockHeight = 1;
            AddFormat(internalFormat);
        };

        auto AddDepthStencilFormat = [&AddFormat](wgpu::TextureFormat format, Aspect aspects,
                                                  uint32_t byteSize) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.isRenderable = true;
            internalFormat.isCompressed = false;
            internalFormat.isSupported = true;
            internalFormat.supportsStorageUsage = false;
            internalFormat.aspects = aspects;
            internalFormat.type = Type::Other;
            internalFormat.blockInfo.blockByteSize = byteSize;
            internalFormat.blockInfo.blockWidth = 1;
            internalFormat.blockInfo.blockHeight = 1;
            AddFormat(internalFormat);
        };

        auto AddDepthFormat = [&AddFormat](wgpu::TextureFormat format, uint32_t byteSize,
                                           Type type) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.isRenderable = true;
            internalFormat.isCompressed = false;
            internalFormat.isSupported = true;
            internalFormat.supportsStorageUsage = false;
            internalFormat.aspects = Aspect::Depth;
            internalFormat.type = type;
            internalFormat.blockInfo.blockByteSize = byteSize;
            internalFormat.blockInfo.blockWidth = 1;
            internalFormat.blockInfo.blockHeight = 1;
            AddFormat(internalFormat);
        };

        auto AddCompressedFormat = [&AddFormat](wgpu::TextureFormat format, uint32_t byteSize,
                                                uint32_t width, uint32_t height, bool isSupported) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.isRenderable = false;
            internalFormat.isCompressed = true;
            internalFormat.isSupported = isSupported;
            internalFormat.supportsStorageUsage = false;
            internalFormat.aspects = Aspect::Color;
            internalFormat.type = Type::Float;
            internalFormat.blockInfo.blockByteSize = byteSize;
            internalFormat.blockInfo.blockWidth = width;
            internalFormat.blockInfo.blockHeight = height;
            AddFormat(internalFormat);
        };

        // clang-format off

        // 1 byte color formats
        AddColorFormat(wgpu::TextureFormat::R8Unorm, true, false, 1, Type::Float);
        AddColorFormat(wgpu::TextureFormat::R8Snorm, false, false, 1, Type::Float);
        AddColorFormat(wgpu::TextureFormat::R8Uint, true, false, 1, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::R8Sint, true, false, 1, Type::Sint);

        // 2 bytes color formats
        AddColorFormat(wgpu::TextureFormat::R16Uint, true, false, 2, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::R16Sint, true, false, 2, Type::Sint);
        AddColorFormat(wgpu::TextureFormat::R16Float, true, false, 2, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RG8Unorm, true, false, 2, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RG8Snorm, false, false, 2, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RG8Uint, true, false, 2, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::RG8Sint, true, false, 2, Type::Sint);

        // 4 bytes color formats
        AddColorFormat(wgpu::TextureFormat::R32Uint, true, true, 4, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::R32Sint, true, true, 4, Type::Sint);
        AddColorFormat(wgpu::TextureFormat::R32Float, true, true, 4, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RG16Uint, true, false, 4, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::RG16Sint, true, false, 4, Type::Sint);
        AddColorFormat(wgpu::TextureFormat::RG16Float, true, false, 4, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RGBA8Unorm, true, true, 4, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RGBA8UnormSrgb, true, false, 4, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RGBA8Snorm, false, true, 4, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RGBA8Uint, true, true, 4, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::RGBA8Sint, true, true, 4, Type::Sint);
        AddColorFormat(wgpu::TextureFormat::BGRA8Unorm, true, false, 4, Type::Float);
        AddColorFormat(wgpu::TextureFormat::BGRA8UnormSrgb, true, false, 4, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RGB10A2Unorm, true, false, 4, Type::Float);

        AddColorFormat(wgpu::TextureFormat::RG11B10Ufloat, false, false, 4, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RGB9E5Ufloat, false, false, 4, Type::Float);

        // 8 bytes color formats
        AddColorFormat(wgpu::TextureFormat::RG32Uint, true, true, 8, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::RG32Sint, true, true, 8, Type::Sint);
        AddColorFormat(wgpu::TextureFormat::RG32Float, true, true, 8, Type::Float);
        AddColorFormat(wgpu::TextureFormat::RGBA16Uint, true, true, 8, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::RGBA16Sint, true, true, 8, Type::Sint);
        AddColorFormat(wgpu::TextureFormat::RGBA16Float, true, true, 8, Type::Float);

        // 16 bytes color formats
        AddColorFormat(wgpu::TextureFormat::RGBA32Uint, true, true, 16, Type::Uint);
        AddColorFormat(wgpu::TextureFormat::RGBA32Sint, true, true, 16, Type::Sint);
        AddColorFormat(wgpu::TextureFormat::RGBA32Float, true, true, 16, Type::Float);

        // Depth only formats
        AddDepthFormat(wgpu::TextureFormat::Depth32Float, 4, Type::Float);

        // Packed depth/depth-stencil formats
        AddDepthStencilFormat(wgpu::TextureFormat::Depth24Plus, Aspect::Depth, 4);
        // TODO(cwallez@chromium.org): It isn't clear if this format should be copyable
        // because its size isn't well defined, is it 4, 5 or 8?
        AddDepthStencilFormat(wgpu::TextureFormat::Depth24PlusStencil8,
                              Aspect::Depth | Aspect::Stencil, 4);

        // BC compressed formats
        bool isBCFormatSupported = device->IsExtensionEnabled(Extension::TextureCompressionBC);
        AddCompressedFormat(wgpu::TextureFormat::BC1RGBAUnorm, 8, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC1RGBAUnormSrgb, 8, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC4RSnorm, 8, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC4RUnorm, 8, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC2RGBAUnorm, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC2RGBAUnormSrgb, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC3RGBAUnorm, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC3RGBAUnormSrgb, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC5RGSnorm, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC5RGUnorm, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC6HRGBFloat, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC6HRGBUfloat, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC7RGBAUnorm, 16, 4, 4, isBCFormatSupported);
        AddCompressedFormat(wgpu::TextureFormat::BC7RGBAUnormSrgb, 16, 4, 4, isBCFormatSupported);

        // clang-format on

        // This checks that each format is set at least once, the second part of checking that all
        // formats are checked exactly once.
        ASSERT(formatsSet.all());

        return table;
    }

}  // namespace dawn_native
