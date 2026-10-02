// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/native/BufferTextureCopyOversizedRow.h"

#include <cstdint>

#include "src/dawn/common/Constants.h"
#include "src/dawn/common/Math.h"
#include "src/dawn/native/Buffer.h"
#include "src/dawn/native/CommandAllocator.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/Texture.h"
#include "src/dawn/native/Toggles.h"

namespace dawn::native {

namespace {

// Intel GPUs store a buffer<->texture copy's bytes per row and texels per row in two separate
// hardware register fields: bytes per row is 18 bits and texels per row is 14 bits. The two must
// be checked independently: bytesPerRow == texelsPerRow * bytesPerBlock, so a format with a small
// bytesPerBlock (e.g. R8Unorm, 1 byte/texel) can push texelsPerRow past 2^14 while bytesPerRow is
// still nowhere near 2^18. See https://crbug.com/481934465.
constexpr uint64_t kMaxBytesPerRow = 1u << 18;
constexpr uint64_t kMaxTexelsPerRow = 1u << 14;

// Splits {bufferCopy, textureCopy, copySize} into one sub-copy per unpadded row, in row-major
// order, invoking `func(bufferOffset, origin)` for each one. `bufferOffset` is the real
// (unclamped) offset of the row in the buffer; `origin` is the texture origin of this row (the
// rest of the TextureCopy is unchanged from the original, unsplit copy).
template <typename Func>
void SplitBufferTextureCopyByRow(const BufferCopy& bufferCopy,
                                 const TextureCopy& textureCopy,
                                 const BlockExtent3D& copySize,
                                 const TypedTexelBlockInfo& blockInfo,
                                 Func&& func) {
    const BlockOrigin3D origin = blockInfo.ToBlock(textureCopy.origin);
    const uint64_t bytesPerRow = blockInfo.ToBytes(bufferCopy.blocksPerRow);

    for (BlockCount z{0u}; z < copySize.depthOrArrayLayers; z += BlockCount{1u}) {
        // Buffer offset of the first row of this layer.
        const uint64_t layerOffset =
            bufferCopy.offset +
            blockInfo.ToBytes(z * bufferCopy.rowsPerImage * bufferCopy.blocksPerRow);
        for (BlockCount y{0u}; y < copySize.height; y += BlockCount{1u}) {
            // Advance from the layer's first row by y rows of (possibly padded) bytes per row.
            func(layerOffset + static_cast<uint64_t>(y) * bytesPerRow,
                 blockInfo.ToTexel(BlockOrigin3D{origin.x, origin.y + y, origin.z + z}));
        }
    }
}

}  // namespace

bool NeedsBufferTextureCopySplitForOversizedRow(const DeviceBase* device,
                                                const BufferCopy& bufferCopy,
                                                const TypedTexelBlockInfo& blockInfo) {
    if (!device->IsToggleEnabled(Toggle::SplitBufferTextureCopyForOversizedRow)) {
        return false;
    }
    const bool exceedsMaxBytesPerRow =
        blockInfo.ToBytes(bufferCopy.blocksPerRow) >= kMaxBytesPerRow;
    const bool exceedsMaxTexelsPerRow =
        static_cast<uint64_t>(blockInfo.ToTexelWidth(bufferCopy.blocksPerRow)) >= kMaxTexelsPerRow;
    return exceedsMaxBytesPerRow || exceedsMaxTexelsPerRow;
}

void SplitCopyTextureToBufferForOversizedRow(CommandAllocator* allocator,
                                             const BufferCopy& bufferCopy,
                                             const TextureCopy& textureCopy,
                                             const BlockExtent3D& copySize,
                                             const TypedTexelBlockInfo& blockInfo) {
    const BlockCount blocksPerRow = blockInfo.BytesToBlocks(
        Align(blockInfo.ToBytes(copySize.width), uint64_t{kTextureBytesPerRowAlignment}));
    SplitBufferTextureCopyByRow(
        bufferCopy, textureCopy, copySize, blockInfo,
        [&](uint64_t bufferOffset, const TexelOrigin3D& origin) {
            CopyTextureToBufferCmd* cmd =
                allocator->Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
            cmd->source = textureCopy;
            cmd->source.origin = origin;
            cmd->destination.buffer = bufferCopy.buffer;
            cmd->destination.offset = bufferOffset;
            cmd->destination.blocksPerRow = blocksPerRow;
            cmd->destination.rowsPerImage = BlockCount{1u};
            cmd->copySize =
                blockInfo.ToTexel(BlockExtent3D{copySize.width, BlockCount{1u}, BlockCount{1u}});
        });
}

void SplitCopyBufferToTextureForOversizedRow(CommandAllocator* allocator,
                                             const BufferCopy& bufferCopy,
                                             const TextureCopy& textureCopy,
                                             const BlockExtent3D& copySize,
                                             const TypedTexelBlockInfo& blockInfo) {
    const BlockCount blocksPerRow = blockInfo.BytesToBlocks(
        Align(blockInfo.ToBytes(copySize.width), uint64_t{kTextureBytesPerRowAlignment}));
    SplitBufferTextureCopyByRow(
        bufferCopy, textureCopy, copySize, blockInfo,
        [&](uint64_t bufferOffset, const TexelOrigin3D& origin) {
            CopyBufferToTextureCmd* cmd =
                allocator->Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
            cmd->source.buffer = bufferCopy.buffer;
            cmd->source.offset = bufferOffset;
            cmd->source.blocksPerRow = blocksPerRow;
            cmd->source.rowsPerImage = BlockCount{1u};
            cmd->destination = textureCopy;
            cmd->destination.origin = origin;
            cmd->copySize =
                blockInfo.ToTexel(BlockExtent3D{copySize.width, BlockCount{1u}, BlockCount{1u}});
        });
}

}  // namespace dawn::native
