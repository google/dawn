// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d12/TextureCopySplitter.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/webgpu_cpp_print.h"
#include "gtest/gtest.h"

namespace dawn::native::d3d12 {
namespace {

// Suffix operator for TexelCount
constexpr TexelCount operator""_tc(uint64_t v) {
    return TexelCount{v};
}
// Suffix operator for BlockCount
constexpr BlockCount operator""_bc(uint64_t v) {
    return BlockCount{v};
}

struct TextureSpec {
    TexelCount x;                    // origin.x in texels
    TexelCount y;                    // origin.y in texels
    TexelCount z;                    // origin.z in texels
    TexelCount width;                // copySize.width in texels
    TexelCount height;               // copySize.height in texels
    TexelCount depthOrArrayLayers;   // copy size depth or array layers
    uint32_t texelBlockSizeInBytes;  // bytes per block
    TexelCount blockWidth{1};        // texel width per block
    TexelCount blockHeight{1};       // texel height per block
};

struct BufferSpec {
    uint64_t offset;        // byte offset into buffer to copy to/from
    uint32_t bytesPerRow;   // bytes per block row (multiples of 256), aka row pitch
    BlockCount rowsPerImage;  // bock rows per image slice (user-defined)
};

// TODO(425944899): Store TypedTexelBlockInfo in TextureSpec
TypedTexelBlockInfo ToTypedTexelBlockInfo(const TextureSpec& textureSpec) {
    TypedTexelBlockInfo blockInfo;
    blockInfo.byteSize = textureSpec.texelBlockSizeInBytes;
    blockInfo.width = TexelCount{textureSpec.blockWidth};
    blockInfo.height = TexelCount{textureSpec.blockHeight};
    return blockInfo;
}

// TODO(425944899): Store TexelOrigin3D in TextureSpec
TexelOrigin3D ToTexelOrigin3D(const TextureSpec& textureSpec) {
    return TexelOrigin3D{textureSpec.x, textureSpec.y, textureSpec.z};
}

// TODO(425944899): Store TexelExtent3D in TextureSpec
TexelExtent3D ToTexelExtent3D(const TextureSpec& textureSpec) {
    return TexelExtent3D{textureSpec.width, textureSpec.height, textureSpec.depthOrArrayLayers};
}

// Check that each copy region fits inside the buffer footprint
void ValidateFootprints(const TextureSpec& textureSpec,
                        const BufferSpec& bufferSpec,
                        const TextureCopySubresource& copySplit,
                        wgpu::TextureDimension dimension) {
    TypedTexelBlockInfo blockInfo = ToTypedTexelBlockInfo(textureSpec);
    for (uint32_t i = 0; i < copySplit.count; ++i) {
        const auto& copy = copySplit.copies[i];
        // TODO(425944899): Rework this function to work in blocks, not texels
        const TexelExtent3D& copySize = blockInfo.ToTexel(copy.copySize);
        const TexelOrigin3D& bufferOffset = blockInfo.ToTexel(copy.bufferOffset);
        const TexelExtent3D& bufferSize = blockInfo.ToTexel(copy.bufferSize);
        ASSERT_LE(bufferOffset.x + copySize.width, bufferSize.width);
        ASSERT_LE(bufferOffset.y + copySize.height, bufferSize.height);
        ASSERT_LE(bufferOffset.z + copySize.depthOrArrayLayers, bufferSize.depthOrArrayLayers);

        // If there are multiple layers, 2D texture splitter actually splits each layer
        // independently. See the details in Compute2DTextureCopySplits(). As a result,
        // if we simply expand a copy region generated by 2D texture splitter to all
        // layers, the copy region might be OOB. But that is not the approach that the
        // current 2D texture splitter is doing, although Compute2DTextureCopySubresource
        // forwards "copySize.depthOrArrayLayers" to the copy region it generated. So skip
        // the test below for 2D textures with multiple layers.
        if (textureSpec.depthOrArrayLayers <= 1_tc || dimension == wgpu::TextureDimension::e3D) {
            BlockCount widthInBlocks = blockInfo.ToBlockWidth(textureSpec.width);
            BlockCount heightInBlocks = blockInfo.ToBlockHeight(textureSpec.height);
            uint64_t minimumRequiredBufferSize =
                bufferSpec.offset +
                // TOOD(425944899): add overload of RequiredBytesInCopy that accepts strong types
                utils::RequiredBytesInCopy(
                    bufferSpec.bytesPerRow, static_cast<uint32_t>(bufferSpec.rowsPerImage),
                    static_cast<uint32_t>(widthInBlocks), static_cast<uint32_t>(heightInBlocks),
                    static_cast<uint32_t>(textureSpec.depthOrArrayLayers),
                    textureSpec.texelBlockSizeInBytes);

            // The last pixel (buffer footprint) of each copy region depends on its
            // bufferOffset and copySize. It is not the last pixel where the bufferSize
            // ends.
            ASSERT_EQ(bufferOffset.x % textureSpec.blockWidth, 0_tc);
            ASSERT_EQ(copySize.width % textureSpec.blockWidth, 0_tc);
            TexelCount footprintWidth = bufferOffset.x + copySize.width;
            ASSERT_EQ(footprintWidth % textureSpec.blockWidth, 0_tc);
            BlockCount footprintWidthInBlocks = blockInfo.ToBlockWidth(footprintWidth);

            ASSERT_EQ(bufferOffset.y % textureSpec.blockHeight, 0_tc);
            ASSERT_EQ(copySize.height % textureSpec.blockHeight, 0_tc);
            TexelCount footprintHeight = bufferOffset.y + copySize.height;
            ASSERT_EQ(footprintHeight % textureSpec.blockHeight, 0_tc);
            BlockCount footprintHeightInBlocks = blockInfo.ToBlockHeight(footprintHeight);

            uint64_t bufferSizeForFootprint =
                copy.alignedOffset +
                utils::RequiredBytesInCopy(
                    bufferSpec.bytesPerRow,
                    static_cast<uint32_t>(blockInfo.ToBlockHeight(bufferSize.height)),
                    static_cast<uint32_t>(footprintWidthInBlocks),
                    static_cast<uint32_t>(footprintHeightInBlocks),
                    static_cast<uint32_t>(blockInfo.ToBlockDepth(bufferSize.depthOrArrayLayers)),
                    textureSpec.texelBlockSizeInBytes);

            // The buffer footprint of each copy region should not exceed the minimum
            // required buffer size. Otherwise, pixels accessed by copy may be OOB.
            ASSERT_LE(bufferSizeForFootprint, minimumRequiredBufferSize);
        }
    }
}

// Check that the offset is aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512)
void ValidateOffset(const TextureCopySubresource& copySplit, bool relaxed) {
    for (uint32_t i = 0; i < copySplit.count; ++i) {
        if (!relaxed) {
            ASSERT_TRUE(
                Align(copySplit.copies[i].alignedOffset, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT) ==
                copySplit.copies[i].alignedOffset);
        }
    }
}

template <typename T>
bool InclusiveRangesOverlap(T minA, T maxA, T minB, T maxB) {
    return (minA <= minB && minB <= maxA) || (minB <= minA && minA <= maxB);
}

// Check that no pair of copy regions intersect each other
void ValidateDisjoint(const TextureSpec& textureSpec, const TextureCopySubresource& copySplit) {
    TypedTexelBlockInfo blockInfo = ToTypedTexelBlockInfo(textureSpec);
    for (uint32_t i = 0; i < copySplit.count; ++i) {
        const auto& a = copySplit.copies[i];
        // TODO(425944899): Rework this function to work in blocks, not texels
        const TexelExtent3D& copySizeA = blockInfo.ToTexel(a.copySize);
        const TexelOrigin3D& textureOffsetA = blockInfo.ToTexel(a.textureOffset);
        for (uint32_t j = i + 1; j < copySplit.count; ++j) {
            const auto& b = copySplit.copies[j];
            // If textureOffset.x is 0, and copySize.width is 2, we are copying pixel 0 and
            // 1. We never touch pixel 2 on x-axis. So the copied range on x-axis should be
            // [textureOffset.x, textureOffset.x + copySize.width - 1] and both ends are
            // included.
            const TexelExtent3D& copySizeB = blockInfo.ToTexel(b.copySize);
            const TexelOrigin3D& textureOffsetB = blockInfo.ToTexel(b.textureOffset);
            bool overlapX =
                InclusiveRangesOverlap(textureOffsetA.x, textureOffsetA.x + copySizeA.width - 1_tc,
                                       textureOffsetB.x, textureOffsetB.x + copySizeB.width - 1_tc);
            bool overlapY = InclusiveRangesOverlap(
                textureOffsetA.y, textureOffsetA.y + copySizeA.height - 1_tc, textureOffsetB.y,
                textureOffsetB.y + copySizeB.height - 1_tc);
            bool overlapZ = InclusiveRangesOverlap(
                textureOffsetA.z, textureOffsetA.z + copySizeA.depthOrArrayLayers - 1_tc,
                textureOffsetB.z, textureOffsetB.z + copySizeB.depthOrArrayLayers - 1_tc);
            ASSERT_TRUE(!overlapX || !overlapY || !overlapZ);
        }
    }
}

// Check that the union of the copy regions exactly covers the texture region
void ValidateTextureBounds(const TextureSpec& textureSpec,
                           const TextureCopySubresource& copySplit) {
    ASSERT_GT(copySplit.count, 0u);
    TypedTexelBlockInfo blockInfo = ToTypedTexelBlockInfo(textureSpec);

    // TODO(425944899): Rework this function to work in blocks, not texels
    const TexelExtent3D& copySize0 = blockInfo.ToTexel(copySplit.copies[0].copySize);
    const TexelOrigin3D& textureOffset0 = blockInfo.ToTexel(copySplit.copies[0].textureOffset);
    TexelCount minX = textureOffset0.x;
    TexelCount minY = textureOffset0.y;
    TexelCount minZ = textureOffset0.z;
    TexelCount maxX = textureOffset0.x + copySize0.width;
    TexelCount maxY = textureOffset0.y + copySize0.height;
    TexelCount maxZ = textureOffset0.z + copySize0.depthOrArrayLayers;

    for (uint32_t i = 1; i < copySplit.count; ++i) {
        const auto& copy = copySplit.copies[i];
        const TexelOrigin3D& textureOffset = blockInfo.ToTexel(copy.textureOffset);
        minX = std::min(minX, textureOffset.x);
        minY = std::min(minY, textureOffset.y);
        minZ = std::min(minZ, textureOffset.z);
        const TexelExtent3D& copySize = blockInfo.ToTexel(copy.copySize);
        maxX = std::max(maxX, textureOffset.x + copySize.width);
        maxY = std::max(maxY, textureOffset.y + copySize.height);
        maxZ = std::max(maxZ, textureOffset.z + copySize.depthOrArrayLayers);
    }

    ASSERT_EQ(minX, textureSpec.x);
    ASSERT_EQ(minY, textureSpec.y);
    ASSERT_EQ(minZ, textureSpec.z);
    ASSERT_EQ(maxX, textureSpec.x + textureSpec.width);
    ASSERT_EQ(maxY, textureSpec.y + textureSpec.height);
    ASSERT_EQ(maxZ, textureSpec.z + textureSpec.depthOrArrayLayers);
}

// Validate that the number of pixels copied is exactly equal to the number of pixels in the
// texture region
void ValidatePixelCount(const TextureSpec& textureSpec, const TextureCopySubresource& copySplit) {
    TypedTexelBlockInfo blockInfo = ToTypedTexelBlockInfo(textureSpec);
    TexelCount totalCopiedTexels{0};
    for (uint32_t i = 0; i < copySplit.count; ++i) {
        const auto& copy = copySplit.copies[i];
        // TODO(425944899): Rework this function to work in blocks, not texels
        const TexelExtent3D& copySize = blockInfo.ToTexel(copy.copySize);
        TexelCount copiedTexels = copySize.width * copySize.height * copySize.depthOrArrayLayers;
        ASSERT_GT(copiedTexels, 0_tc);
        totalCopiedTexels += copiedTexels;
    }
    ASSERT_EQ(totalCopiedTexels,
              textureSpec.width * textureSpec.height * textureSpec.depthOrArrayLayers);
}

// Check that every buffer offset is at the correct pixel location
void ValidateBufferOffset(const TextureSpec& textureSpec,
                          const BufferSpec& bufferSpec,
                          const TextureCopySubresource& copySplit,
                          wgpu::TextureDimension dimension,
                          bool relaxed) {
    ASSERT_GT(copySplit.count, 0u);
    TypedTexelBlockInfo blockInfo = ToTypedTexelBlockInfo(textureSpec);

    for (uint32_t i = 0; i < copySplit.count; ++i) {
        const auto& copy = copySplit.copies[i];
        const BlockOrigin3D& bufferOffset = copy.bufferOffset;
        const BlockOrigin3D& textureOffset = copy.textureOffset;
        // Note that for relaxed, the row pitch (bytesPerRow) is not required to be 256 bytes,
        // but Dawn currently doesn't do anything about this.
        BlockCount rowPitchInBlocks = blockInfo.BytesToBlocks(bufferSpec.bytesPerRow);
        BlockCount slicePitchInBlocks = rowPitchInBlocks * bufferSpec.rowsPerImage;
        BlockCount absoluteOffsetInBlocks = blockInfo.BytesToBlocks(copy.alignedOffset) +
                                            bufferOffset.x + bufferOffset.y * rowPitchInBlocks;

        // There is one empty row at most in a 2D copy region. However, it is not true for
        // a 3D texture copy region when we are copying the last row of each slice. We may
        // need to offset a lot rows and copy.bufferOffset.y may be big.
        if (dimension == wgpu::TextureDimension::e2D) {
            ASSERT_LE(bufferOffset.y, BlockCount{1});
        }
        ASSERT_EQ(bufferOffset.z, 0_bc);

        ASSERT_GE(absoluteOffsetInBlocks, blockInfo.BytesToBlocks(bufferSpec.offset));

        BlockCount relativeOffsetInBlocks =
            absoluteOffsetInBlocks - blockInfo.BytesToBlocks(bufferSpec.offset);

        BlockCount z = relativeOffsetInBlocks / slicePitchInBlocks;
        BlockCount yBlocks = (relativeOffsetInBlocks % slicePitchInBlocks) / rowPitchInBlocks;
        BlockCount xBlocks = relativeOffsetInBlocks % rowPitchInBlocks;

        ASSERT_EQ(textureOffset.x - blockInfo.ToBlockWidth(textureSpec.x), xBlocks);
        ASSERT_EQ(textureOffset.y - blockInfo.ToBlockHeight(textureSpec.y), yBlocks);
        ASSERT_EQ(textureOffset.z - blockInfo.ToBlockDepth(textureSpec.z), z);
    }
}

void ValidateCopySplit(const TextureSpec& textureSpec,
                       const BufferSpec& bufferSpec,
                       const TextureCopySubresource& copySplit,
                       wgpu::TextureDimension dimension,
                       bool relaxed) {
    ValidateFootprints(textureSpec, bufferSpec, copySplit, dimension);
    ValidateOffset(copySplit, relaxed);
    ValidateDisjoint(textureSpec, copySplit);
    ValidateTextureBounds(textureSpec, copySplit);
    ValidatePixelCount(textureSpec, copySplit);
    ValidateBufferOffset(textureSpec, bufferSpec, copySplit, dimension, relaxed);
}

std::ostream& operator<<(std::ostream& os, const TextureSpec& textureSpec) {
    os << "TextureSpec(" << "[origin=(" << textureSpec.x << ", " << textureSpec.y << ", "
       << textureSpec.z << "), copySize=(" << textureSpec.width << ", " << textureSpec.height
       << ", " << textureSpec.depthOrArrayLayers
       << ")], blockBytes=" << textureSpec.texelBlockSizeInBytes
       << ", blockWidth=" << textureSpec.blockWidth << ", blockHeight=" << textureSpec.blockHeight
       << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const BufferSpec& bufferSpec) {
    os << "BufferSpec(offset=" << bufferSpec.offset << ", bytesPerRow=" << bufferSpec.bytesPerRow
       << ", rowsPerImage=" << bufferSpec.rowsPerImage << ")";
    return os;
}
std::ostream& operator<<(std::ostream& os, const TextureCopySubresource& copySplit) {
    os << "CopySplit\n";
    for (uint32_t i = 0; i < copySplit.count; ++i) {
        const auto& copy = copySplit.copies[i];
        auto& textureOffset = copy.textureOffset;
        auto& bufferOffset = copy.bufferOffset;
        auto& copySize = copy.copySize;
        os << "  " << i << ": Texture at (" << textureOffset.x << ", " << textureOffset.y << ", "
           << textureOffset.z << "), size (" << copySize.width << ", " << copySize.height << ", "
           << copySize.depthOrArrayLayers << ")\n";
        os << "  " << i << ": Buffer at (" << bufferOffset.x << ", " << bufferOffset.y << ", "
           << bufferOffset.z << "), footprint (" << copySize.width << ", " << copySize.height
           << ", " << copySize.depthOrArrayLayers << ")\n";
    }
    return os;
}

// Define base texture sizes and offsets to test with: some aligned, some unaligned
constexpr TextureSpec kBaseTextureSpecs[] = {
    // 1x1 2D copies
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 1_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 64_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 128_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 192_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 31_tc,
     .y = 16_tc,
     .z = 0_tc,
     .width = 1_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 64_tc,
     .y = 16_tc,
     .z = 0_tc,
     .width = 1_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 64_tc,
     .y = 16_tc,
     .z = 8_tc,
     .width = 1_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    // 2x1, 1x2, and 2x2
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 64_tc,
     .height = 2_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 64_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 2_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 64_tc,
     .height = 2_tc,
     .depthOrArrayLayers = 2_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 128_tc,
     .height = 2_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 128_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 2_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 128_tc,
     .height = 2_tc,
     .depthOrArrayLayers = 2_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 192_tc,
     .height = 2_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 192_tc,
     .height = 1_tc,
     .depthOrArrayLayers = 2_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 192_tc,
     .height = 2_tc,
     .depthOrArrayLayers = 2_tc,
     .texelBlockSizeInBytes = 4},
    // 1024x1024 2D and 3D
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 256_tc,
     .y = 512_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 64_tc,
     .y = 48_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 64_tc,
     .y = 48_tc,
     .z = 16_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1024_tc,
     .texelBlockSizeInBytes = 4},
    // Non-power of two texture dims
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 257_tc,
     .height = 31_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 17_tc,
     .height = 93_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 59_tc,
     .y = 13_tc,
     .z = 0_tc,
     .width = 257_tc,
     .height = 31_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 17_tc,
     .y = 73_tc,
     .z = 0_tc,
     .width = 17_tc,
     .height = 93_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4},
    {.x = 17_tc,
     .y = 73_tc,
     .z = 59_tc,
     .width = 17_tc,
     .height = 93_tc,
     .depthOrArrayLayers = 99_tc,
     .texelBlockSizeInBytes = 4},
    // 4x4 block size 2D copies
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 4_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 64_tc,
     .y = 16_tc,
     .z = 0_tc,
     .width = 4_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 64_tc,
     .y = 16_tc,
     .z = 8_tc,
     .width = 4_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 4_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 16,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 64_tc,
     .y = 16_tc,
     .z = 0_tc,
     .width = 4_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 16,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 64_tc,
     .y = 16_tc,
     .z = 8_tc,
     .width = 4_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 16,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    // 4x4 block size 2D copies of 1024x1024 textures
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 256_tc,
     .y = 512_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 64_tc,
     .y = 48_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 64_tc,
     .y = 48_tc,
     .z = 16_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 16,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 256_tc,
     .y = 512_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 16,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 64_tc,
     .y = 48_tc,
     .z = 0_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 4,
     .blockWidth = 16_tc,
     .blockHeight = 4_tc},
    {.x = 64_tc,
     .y = 48_tc,
     .z = 16_tc,
     .width = 1024_tc,
     .height = 1024_tc,
     .depthOrArrayLayers = 1_tc,
     .texelBlockSizeInBytes = 16,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    // 4x4 block size 3D copies
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 64_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 2_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 64_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 2_tc,
     .texelBlockSizeInBytes = 16,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 64_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 8_tc,
     .texelBlockSizeInBytes = 16,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
    {.x = 0_tc,
     .y = 0_tc,
     .z = 0_tc,
     .width = 128_tc,
     .height = 4_tc,
     .depthOrArrayLayers = 8_tc,
     .texelBlockSizeInBytes = 8,
     .blockWidth = 4_tc,
     .blockHeight = 4_tc},
};

// Define base buffer sizes to work with: some offsets aligned, some unaligned. bytesPerRow
// is the minimum required
std::array<BufferSpec, 15> BaseBufferSpecs(const TextureSpec& textureSpec) {
    TypedTexelBlockInfo blockInfo = ToTypedTexelBlockInfo(textureSpec);
    uint32_t bytesPerRow = Align(blockInfo.ToBytes(blockInfo.ToBlockWidth(textureSpec.width)),
                                 kTextureBytesPerRowAlignment);

    auto alignNonPow2 = [](uint32_t value, uint32_t size) -> uint32_t {
        return value == 0 ? 0 : ((value - 1) / size + 1) * size;
    };

    return {
        BufferSpec{alignNonPow2(0, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(256, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(512, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(1024, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(1024, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height) * 2_bc},

        BufferSpec{alignNonPow2(32, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(64, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(64, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height) * 2_bc},

        BufferSpec{alignNonPow2(31, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(257, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(384, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(511, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(513, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(1023, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height)},
        BufferSpec{alignNonPow2(1023, textureSpec.texelBlockSizeInBytes), bytesPerRow,
                   blockInfo.ToBlockHeight(textureSpec.height) * 2_bc},
    };
}

// Define a list of values to set properties in the spec structs
constexpr uint32_t kCheckValues[] = {1,  2,  3,  4,   5,   6,   7,    8,     // small values
                                     16, 32, 64, 128, 256, 512, 1024, 2048,  // powers of 2
                                     15, 31, 63, 127, 257, 511, 1023, 2047,  // misalignments
                                     17, 33, 65, 129, 257, 513, 1025, 2049};

struct CopySplitTestParam {
    wgpu::TextureDimension dimension;
    bool relaxed;
};

class CopySplitTest : public testing::TestWithParam<CopySplitTestParam> {
  protected:
    void DoTest(const TextureSpec& textureSpec,
                const BufferSpec& bufferSpec,
                wgpu::TextureDimension dimension,
                bool relaxed) {
        DAWN_ASSERT(textureSpec.width % textureSpec.blockWidth == 0_tc &&
                    textureSpec.height % textureSpec.blockHeight == 0_tc);
        // Add trace so that failures emit the input test specs
        std::stringstream trace;
        trace << textureSpec << ", " << bufferSpec;
        SCOPED_TRACE(trace.str());

        TextureCopySubresource copySplit;
        switch (dimension) {
            case wgpu::TextureDimension::e1D:
            case wgpu::TextureDimension::e2D: {
                // Skip test cases that are clearly for 3D. Validation would catch
                // these cases before reaching the TextureCopySplitter.
                if (textureSpec.z > 0_tc || textureSpec.depthOrArrayLayers > 1_tc) {
                    return;
                }
                if (relaxed) {
                    copySplit = Compute2DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
                        ToTexelOrigin3D(textureSpec).ToOrigin3D(),
                        ToTexelExtent3D(textureSpec).ToExtent3D(),
                        ToTypedTexelBlockInfo(textureSpec).ToTexelBlockInfo(), bufferSpec.offset,
                        bufferSpec.bytesPerRow);
                } else {
                    copySplit = Compute2DTextureCopySubresource(
                        ToTexelOrigin3D(textureSpec).ToOrigin3D(),
                        ToTexelExtent3D(textureSpec).ToExtent3D(),
                        ToTypedTexelBlockInfo(textureSpec).ToTexelBlockInfo(), bufferSpec.offset,
                        bufferSpec.bytesPerRow);
                }
                break;
            }
            case wgpu::TextureDimension::e3D: {
                if (relaxed) {
                    copySplit = Compute3DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
                        ToTexelOrigin3D(textureSpec).ToOrigin3D(),
                        ToTexelExtent3D(textureSpec).ToExtent3D(),
                        ToTypedTexelBlockInfo(textureSpec).ToTexelBlockInfo(), bufferSpec.offset,
                        bufferSpec.bytesPerRow, static_cast<uint32_t>(bufferSpec.rowsPerImage));
                } else {
                    copySplit = Compute3DTextureCopySplits(
                        ToTexelOrigin3D(textureSpec).ToOrigin3D(),
                        ToTexelExtent3D(textureSpec).ToExtent3D(),
                        ToTypedTexelBlockInfo(textureSpec).ToTexelBlockInfo(), bufferSpec.offset,
                        bufferSpec.bytesPerRow, static_cast<uint32_t>(bufferSpec.rowsPerImage));
                }
                break;
            }
            default:
                DAWN_UNREACHABLE();
                break;
        }

        ValidateCopySplit(textureSpec, bufferSpec, copySplit, dimension, relaxed);

        if (HasFatalFailure()) {
            std::ostringstream message;
            message << "Failed generating splits: " << textureSpec << ", " << bufferSpec << "\n"
                    << dimension << " " << copySplit << "\n";
            FAIL() << message.str();
        }
    }

    // Call from parameterized tests
    void DoTest(const TextureSpec& textureSpec, const BufferSpec& bufferSpec) {
        wgpu::TextureDimension dimension = GetParam().dimension;
        bool relaxed = GetParam().relaxed;
        DoTest(textureSpec, bufferSpec, dimension, relaxed);
    }
};

TEST_P(CopySplitTest, General) {
    for (const TextureSpec& textureSpec : kBaseTextureSpecs) {
        for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
            DoTest(textureSpec, bufferSpec);
        }
    }
}

TEST_P(CopySplitTest, TextureWidth) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t val : kCheckValues) {
            if (TexelCount{val} % textureSpec.blockWidth != 0_tc) {
                continue;
            }
            textureSpec.width = TexelCount{val};
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, TextureHeight) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t val : kCheckValues) {
            if (TexelCount{val} % textureSpec.blockHeight != 0_tc) {
                continue;
            }
            textureSpec.height = TexelCount{val};
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, TextureX) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t val : kCheckValues) {
            if (TexelCount{val} % textureSpec.blockWidth != 0_tc) {
                continue;
            }
            textureSpec.x = TexelCount{val};
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, TextureY) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t val : kCheckValues) {
            if (TexelCount{val} % textureSpec.blockHeight != 0_tc) {
                continue;
            }
            textureSpec.y = TexelCount{val};
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, TexelSize) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t texelSize : {4, 8, 16, 32, 64}) {
            textureSpec.texelBlockSizeInBytes = texelSize;
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, BufferOffset) {
    for (const TextureSpec& textureSpec : kBaseTextureSpecs) {
        for (BufferSpec bufferSpec : BaseBufferSpecs(textureSpec)) {
            for (uint32_t val : kCheckValues) {
                bufferSpec.offset = textureSpec.texelBlockSizeInBytes * val;

                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, RowPitch) {
    for (const TextureSpec& textureSpec : kBaseTextureSpecs) {
        for (BufferSpec bufferSpec : BaseBufferSpecs(textureSpec)) {
            uint32_t baseRowPitch = bufferSpec.bytesPerRow;
            for (uint32_t i = 0; i < 5; ++i) {
                bufferSpec.bytesPerRow = baseRowPitch + i * 256;

                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, ImageHeight) {
    for (const TextureSpec& textureSpec : kBaseTextureSpecs) {
        for (BufferSpec bufferSpec : BaseBufferSpecs(textureSpec)) {
            BlockCount baseImageHeight = bufferSpec.rowsPerImage;
            for (uint32_t i = 0; i < 5; ++i) {
                bufferSpec.rowsPerImage = baseImageHeight + BlockCount{i * 256};

                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    CopySplitTest,
    testing::Values(CopySplitTestParam{.dimension = wgpu::TextureDimension::e1D, .relaxed = false},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e1D, .relaxed = true},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e2D, .relaxed = false},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e2D, .relaxed = true},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e3D, .relaxed = false},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e3D, .relaxed = true}));

// Test for specific case that failed CTS for BCSliced3D formats (4x4 block) when the copy height
// is 1 block row, and we have a buffer offset that results in the copy region straddling
// bytesPerRow.
TEST_F(CopySplitTest, Block4x4_3D_CopyOneRow_StraddleBytesPerRowOffset) {
    constexpr TexelCount blockDim = 4_tc;
    constexpr uint32_t bytesPerBlock = 8;
    TextureSpec textureSpec = {.x = 0_tc,
                               .y = 0_tc,
                               .z = 0_tc,
                               .width = 128_tc,
                               .height = 1_tc * blockDim,
                               .depthOrArrayLayers = 8_tc,
                               .texelBlockSizeInBytes = bytesPerBlock,
                               .blockWidth = blockDim,
                               .blockHeight = blockDim};
    BufferSpec bufferSpec = {.offset = 3592, .bytesPerRow = 256, .rowsPerImage = 1_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e3D, false);
}
// Test similar failure to above for 2x2 blocks
TEST_F(CopySplitTest, Block2x2_3D_CopyOneRow_StraddleBytesPerRowOffset) {
    constexpr TexelCount blockDim = 2_tc;
    constexpr uint32_t bytesPerBlock = 4;
    TextureSpec textureSpec = {.x = 0_tc,
                               .y = 0_tc,
                               .z = 0_tc,
                               .width = 128_tc,
                               .height = 1_tc * blockDim,
                               .depthOrArrayLayers = 8_tc,
                               .texelBlockSizeInBytes = bytesPerBlock,
                               .blockWidth = blockDim,
                               .blockHeight = blockDim};
    BufferSpec bufferSpec = {.offset = 3592, .bytesPerRow = 256, .rowsPerImage = 1_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e3D, false);
}
// Also test 1x1, although this always passed as this one engages the "copySize.height is odd" path.
TEST_F(CopySplitTest, Block1x1_3D_CopyOneRow_StraddleBytesPerRowOffset) {
    constexpr TexelCount blockDim = 1_tc;
    constexpr uint32_t bytesPerBlock = 4;
    TextureSpec textureSpec = {.x = 0_tc,
                               .y = 0_tc,
                               .z = 0_tc,
                               .width = 128_tc,
                               .height = 1_tc * blockDim,
                               .depthOrArrayLayers = 8_tc,
                               .texelBlockSizeInBytes = bytesPerBlock,
                               .blockWidth = blockDim,
                               .blockHeight = blockDim};
    BufferSpec bufferSpec = {.offset = 3592, .bytesPerRow = 256, .rowsPerImage = 1_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e3D, false);
}

}  // anonymous namespace
}  // namespace dawn::native::d3d12
