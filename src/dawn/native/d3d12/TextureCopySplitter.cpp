// Copyright 2017 The Dawn Authors
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

#include "dawn/native/d3d12/TextureCopySplitter.h"

#include "dawn/common/Assert.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

namespace {
Origin3D ComputeTexelOffsets(const TexelBlockInfo& blockInfo,
                             uint32_t offset,
                             uint32_t bytesPerRow) {
    ASSERT(bytesPerRow != 0);
    uint32_t byteOffsetX = offset % bytesPerRow;
    uint32_t byteOffsetY = offset - byteOffsetX;

    return {byteOffsetX / blockInfo.byteSize * blockInfo.width,
            byteOffsetY / bytesPerRow * blockInfo.height, 0};
}

uint64_t OffsetToFirstCopiedTexel(const TexelBlockInfo& blockInfo,
                                  uint32_t bytesPerRow,
                                  uint64_t alignedOffset,
                                  Origin3D bufferOffset) {
    ASSERT(bufferOffset.z == 0);
    return alignedOffset + bufferOffset.x * blockInfo.byteSize / blockInfo.width +
           bufferOffset.y * bytesPerRow / blockInfo.height;
}

uint64_t AlignDownForDataPlacement(uint32_t offset) {
    return offset & ~static_cast<uint64_t>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
}
}  // namespace

TextureCopySubresource::CopyInfo* TextureCopySubresource::AddCopy() {
    ASSERT(this->count < kMaxTextureCopyRegions);
    return &this->copies[this->count++];
}

TextureCopySubresource Compute2DTextureCopySubresource(Origin3D origin,
                                                       Extent3D copySize,
                                                       const TexelBlockInfo& blockInfo,
                                                       uint64_t offset,
                                                       uint32_t bytesPerRow) {
    TextureCopySubresource copy;

    ASSERT(bytesPerRow % blockInfo.byteSize == 0);

    // The copies must be 512-aligned. To do this, we calculate the first 512-aligned address
    // preceding our data.
    uint64_t alignedOffset = AlignDownForDataPlacement(offset);

    // If the provided offset to the data was already 512-aligned, we can simply copy the data
    // without further translation.
    if (offset == alignedOffset) {
        copy.count = 1;

        copy.copies[0].alignedOffset = alignedOffset;
        copy.copies[0].textureOffset = origin;
        copy.copies[0].copySize = copySize;
        copy.copies[0].bufferOffset = {0, 0, 0};
        copy.copies[0].bufferSize = copySize;

        return copy;
    }

    ASSERT(alignedOffset < offset);
    ASSERT(offset - alignedOffset < D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

    // We must reinterpret our aligned offset into X and Y offsets with respect to the row
    // pitch.
    //
    // You can visualize the data in the buffer like this:
    // |-----------------------++++++++++++++++++++++++++++++++|
    // ^ 512-aligned address   ^ Aligned offset               ^ End of copy data
    //
    // Now when you consider the row pitch, you can visualize the data like this:
    // |~~~~~~~~~~~~~~~~|
    // |~~~~~+++++++++++|
    // |++++++++++++++++|
    // |+++++~~~~~~~~~~~|
    // |<---row pitch-->|
    //
    // The X and Y offsets calculated in ComputeTexelOffsets can be visualized like this:
    // |YYYYYYYYYYYYYYYY|
    // |XXXXXX++++++++++|
    // |++++++++++++++++|
    // |++++++~~~~~~~~~~|
    // |<---row pitch-->|
    Origin3D texelOffset =
        ComputeTexelOffsets(blockInfo, static_cast<uint32_t>(offset - alignedOffset), bytesPerRow);

    ASSERT(texelOffset.y <= blockInfo.height);
    ASSERT(texelOffset.z == 0);

    uint32_t copyBytesPerRowPitch = copySize.width / blockInfo.width * blockInfo.byteSize;
    uint32_t byteOffsetInRowPitch = texelOffset.x / blockInfo.width * blockInfo.byteSize;
    if (copyBytesPerRowPitch + byteOffsetInRowPitch <= bytesPerRow) {
        // The region's rows fit inside the bytes per row. In this case, extend the width of the
        // PlacedFootprint and copy the buffer with an offset location
        //  |<------------- bytes per row ------------->|
        //
        //  |-------------------------------------------|
        //  |                                           |
        //  |                 +++++++++++++++++~~~~~~~~~|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++~~~~~~~~~|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++~~~~~~~~~|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++~~~~~~~~~|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++         |
        //  |-------------------------------------------|

        // Copy 0:
        //  |----------------------------------|
        //  |                                  |
        //  |                 +++++++++++++++++|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++|
        //  |----------------------------------|

        copy.count = 1;

        copy.copies[0].alignedOffset = alignedOffset;
        copy.copies[0].textureOffset = origin;
        copy.copies[0].copySize = copySize;
        copy.copies[0].bufferOffset = texelOffset;

        copy.copies[0].bufferSize.width = copySize.width + texelOffset.x;
        copy.copies[0].bufferSize.height = copySize.height + texelOffset.y;
        copy.copies[0].bufferSize.depthOrArrayLayers = copySize.depthOrArrayLayers;

        return copy;
    }

    // The region's rows straddle the bytes per row. Split the copy into two copies
    //  |<------------- bytes per row ------------->|
    //
    //  |-------------------------------------------|
    //  |                                           |
    //  |                                   ++++++++|
    //  |+++++++++~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |+++++++++~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |+++++++++~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |+++++++++~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |+++++++++                                  |
    //  |-------------------------------------------|

    //  Copy 0:
    //  |-------------------------------------------|
    //  |                                           |
    //  |                                   ++++++++|
    //  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |-------------------------------------------|

    //  Copy 1:
    //  |---------|
    //  |         |
    //  |         |
    //  |+++++++++|
    //  |+++++++++|
    //  |+++++++++|
    //  |+++++++++|
    //  |+++++++++|
    //  |---------|

    copy.count = 2;

    copy.copies[0].alignedOffset = alignedOffset;
    copy.copies[0].textureOffset = origin;

    ASSERT(bytesPerRow > byteOffsetInRowPitch);
    uint32_t texelsPerRow = bytesPerRow / blockInfo.byteSize * blockInfo.width;
    copy.copies[0].copySize.width = texelsPerRow - texelOffset.x;
    copy.copies[0].copySize.height = copySize.height;
    copy.copies[0].copySize.depthOrArrayLayers = copySize.depthOrArrayLayers;

    copy.copies[0].bufferOffset = texelOffset;
    copy.copies[0].bufferSize.width = texelsPerRow;
    copy.copies[0].bufferSize.height = copySize.height + texelOffset.y;
    copy.copies[0].bufferSize.depthOrArrayLayers = copySize.depthOrArrayLayers;

    uint64_t offsetForCopy1 =
        offset + copy.copies[0].copySize.width / blockInfo.width * blockInfo.byteSize;
    uint64_t alignedOffsetForCopy1 = AlignDownForDataPlacement(offsetForCopy1);
    Origin3D texelOffsetForCopy1 = ComputeTexelOffsets(
        blockInfo, static_cast<uint32_t>(offsetForCopy1 - alignedOffsetForCopy1), bytesPerRow);

    ASSERT(texelOffsetForCopy1.y <= blockInfo.height);
    ASSERT(texelOffsetForCopy1.z == 0);

    copy.copies[1].alignedOffset = alignedOffsetForCopy1;
    copy.copies[1].textureOffset.x = origin.x + copy.copies[0].copySize.width;
    copy.copies[1].textureOffset.y = origin.y;
    copy.copies[1].textureOffset.z = origin.z;

    ASSERT(copySize.width > copy.copies[0].copySize.width);
    copy.copies[1].copySize.width = copySize.width - copy.copies[0].copySize.width;
    copy.copies[1].copySize.height = copySize.height;
    copy.copies[1].copySize.depthOrArrayLayers = copySize.depthOrArrayLayers;

    copy.copies[1].bufferOffset = texelOffsetForCopy1;
    copy.copies[1].bufferSize.width = copy.copies[1].copySize.width + texelOffsetForCopy1.x;
    copy.copies[1].bufferSize.height = copySize.height + texelOffsetForCopy1.y;
    copy.copies[1].bufferSize.depthOrArrayLayers = copySize.depthOrArrayLayers;

    return copy;
}

TextureCopySplits Compute2DTextureCopySplits(Origin3D origin,
                                             Extent3D copySize,
                                             const TexelBlockInfo& blockInfo,
                                             uint64_t offset,
                                             uint32_t bytesPerRow,
                                             uint32_t rowsPerImage) {
    TextureCopySplits copies;

    const uint64_t bytesPerLayer = bytesPerRow * rowsPerImage;

    // The function Compute2DTextureCopySubresource() decides how to split the copy based on:
    // - the alignment of the buffer offset with D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512)
    // - the alignment of the buffer offset with D3D12_TEXTURE_DATA_PITCH_ALIGNMENT (256)
    // Each layer of a 2D array might need to be split, but because of the WebGPU
    // constraint that "bytesPerRow" must be a multiple of 256, all odd (resp. all even) layers
    // will be at an offset multiple of 512 of each other, which means they will all result in
    // the same 2D split. Thus we can just compute the copy splits for the first and second
    // layers, and reuse them for the remaining layers by adding the related offset of each
    // layer. Moreover, if "rowsPerImage" is even, both the first and second copy layers can
    // share the same copy split, so in this situation we just need to compute copy split once
    // and reuse it for all the layers.
    Extent3D copyOneLayerSize = copySize;
    Origin3D copyFirstLayerOrigin = origin;
    copyOneLayerSize.depthOrArrayLayers = 1;
    copyFirstLayerOrigin.z = 0;

    copies.copySubresources[0] = Compute2DTextureCopySubresource(
        copyFirstLayerOrigin, copyOneLayerSize, blockInfo, offset, bytesPerRow);

    // When the copy only refers one texture 2D array layer,
    // copies.copySubresources[1] will never be used so we can safely early return here.
    if (copySize.depthOrArrayLayers == 1) {
        return copies;
    }

    if (bytesPerLayer % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0) {
        copies.copySubresources[1] = copies.copySubresources[0];
        copies.copySubresources[1].copies[0].alignedOffset += bytesPerLayer;
        copies.copySubresources[1].copies[1].alignedOffset += bytesPerLayer;
    } else {
        const uint64_t bufferOffsetNextLayer = offset + bytesPerLayer;
        copies.copySubresources[1] = Compute2DTextureCopySubresource(
            copyFirstLayerOrigin, copyOneLayerSize, blockInfo, bufferOffsetNextLayer, bytesPerRow);
    }

    return copies;
}

void Recompute3DTextureCopyRegionWithEmptyFirstRowAndEvenCopyHeight(Origin3D origin,
                                                                    Extent3D copySize,
                                                                    const TexelBlockInfo& blockInfo,
                                                                    uint32_t bytesPerRow,
                                                                    uint32_t rowsPerImage,
                                                                    TextureCopySubresource& copy,
                                                                    uint32_t i) {
    // Let's assign data and show why copy region generated by ComputeTextureCopySubresource
    // is incorrect if there is an empty row at the beginning of the copy block.
    // Assuming that bytesPerRow is 256 and we are doing a B2T copy, and copy size is {width: 2,
    // height: 4, depthOrArrayLayers: 3}. Then the data layout in buffer is demonstrated
    // as below:
    //
    //               |<----- bytes per row ------>|
    //
    //               |----------------------------|
    //  row (N - 1)  |                            |
    //  row N        |                 ++~~~~~~~~~|
    //  row (N + 1)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 2)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 3)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 4)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 5)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 6)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 7)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 8)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 9)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 10) |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 11) |~~~~~~~~~~~~~~~~~++         |
    //               |----------------------------|

    // The copy we mean to do is the following:
    //
    //   - image 0: row N to row (N + 3),
    //   - image 1: row (N + 4) to row (N + 7),
    //   - image 2: row (N + 8) to row (N + 11).
    //
    // Note that alignedOffset is at the beginning of row (N - 1), while buffer offset makes
    // the copy start at row N. Row (N - 1) is the empty row between alignedOffset and offset.
    //
    // The 2D copy region of image 0 we received from Compute2DTextureCopySubresource() is
    // the following:
    //
    //              |-------------------|
    //  row (N - 1) |                   |
    //  row N       |                 ++|
    //  row (N + 1) |~~~~~~~~~~~~~~~~~++|
    //  row (N + 2) |~~~~~~~~~~~~~~~~~++|
    //  row (N + 3) |~~~~~~~~~~~~~~~~~++|
    //              |-------------------|
    //
    // However, if we simply expand the copy region of image 0 to all depth ranges of a 3D
    // texture, we will copy 5 rows every time, and every first row of each slice will be
    // skipped. As a result, the copied data will be:
    //
    //   - image 0: row N to row (N + 3), which is correct. Row (N - 1) is skipped.
    //   - image 1: row (N + 5) to row (N + 8) because row (N + 4) is skipped. It is incorrect.
    //
    // Likewise, all other image followed will be incorrect because we wrongly keep skipping
    // one row for each depth slice.
    //
    // Solution: split the copy region to two copies: copy 3 (rowsPerImage - 1) rows in and
    // expand to all depth slices in the first copy. 3 rows + one skipped rows = 4 rows, which
    // equals to rowsPerImage. Then copy the last row in the second copy. However, the copy
    // block of the last row of the last image may out-of-bound (see the details below), so
    // we need an extra copy for the very last row.

    // Copy 0: copy 3 rows, not 4 rows.
    //                _____________________
    //               /                    /|
    //              /                    / |
    //              |-------------------|  |
    //  row (N - 1) |                   |  |
    //  row N       |                 ++|  |
    //  row (N + 1) |~~~~~~~~~~~~~~~~~++| /
    //  row (N + 2) |~~~~~~~~~~~~~~~~~++|/
    //              |-------------------|

    // Copy 1: move down two rows and copy the last row on image 0, and expand to
    // copySize.depthOrArrayLayers - 1 depth slices. Note that if we expand it to all depth
    // slices, the last copy block will be row (N + 9) to row (N + 12). Row (N + 11) might
    // be the last row of the entire buffer. Then row (N + 12) will be out-of-bound.
    //                _____________________
    //               /                    /|
    //              /                    / |
    //              |-------------------|  |
    //  row (N + 1) |                   |  |
    //  row (N + 2) |                   |  |
    //  row (N + 3) |                 ++| /
    //  row (N + 4) |~~~~~~~~~~~~~~~~~~~|/
    //              |-------------------|
    //
    //  copy 2: copy the last row of the last image.
    //              |-------------------|
    //  row (N + 11)|                 ++|
    //              |-------------------|

    // Copy 0: copy copySize.height - 1 rows
    TextureCopySubresource::CopyInfo& copy0 = copy.copies[i];
    copy0.copySize.height = copySize.height - blockInfo.height;
    copy0.bufferSize.height = rowsPerImage * blockInfo.height;  // rowsPerImageInTexels

    // Copy 1: move down 2 rows and copy the last row on image 0, and expand to all depth slices
    // but the last one.
    TextureCopySubresource::CopyInfo* copy1 = copy.AddCopy();
    *copy1 = copy0;
    copy1->alignedOffset += 2 * bytesPerRow;
    copy1->textureOffset.y += copySize.height - blockInfo.height;
    // Offset two rows from the copy height for the bufferOffset (See the figure above):
    //   - one for the row we advanced in the buffer: row (N + 4).
    //   - one for the last row we want to copy: row (N + 3) itself.
    copy1->bufferOffset.y = copySize.height - 2 * blockInfo.height;
    copy1->copySize.height = blockInfo.height;
    copy1->copySize.depthOrArrayLayers--;
    copy1->bufferSize.depthOrArrayLayers--;

    // Copy 2: copy the last row of the last image.
    uint64_t offsetForCopy0 =
        OffsetToFirstCopiedTexel(blockInfo, bytesPerRow, copy0.alignedOffset, copy0.bufferOffset);
    uint64_t offsetForLastRowOfLastImage =
        offsetForCopy0 +
        bytesPerRow * (copy0.copySize.height + rowsPerImage * (copySize.depthOrArrayLayers - 1));
    uint64_t alignedOffsetForLastRowOfLastImage =
        AlignDownForDataPlacement(offsetForLastRowOfLastImage);
    Origin3D texelOffsetForLastRowOfLastImage = ComputeTexelOffsets(
        blockInfo,
        static_cast<uint32_t>(offsetForLastRowOfLastImage - alignedOffsetForLastRowOfLastImage),
        bytesPerRow);

    TextureCopySubresource::CopyInfo* copy2 = copy.AddCopy();
    copy2->alignedOffset = alignedOffsetForLastRowOfLastImage;
    copy2->textureOffset = copy1->textureOffset;
    copy2->textureOffset.z = origin.z + copySize.depthOrArrayLayers - 1;
    copy2->copySize = copy1->copySize;
    copy2->copySize.depthOrArrayLayers = 1;
    copy2->bufferOffset = texelOffsetForLastRowOfLastImage;
    copy2->bufferSize.width = copy1->bufferSize.width;
    ASSERT(copy2->copySize.height == 1);
    copy2->bufferSize.height = copy2->bufferOffset.y + copy2->copySize.height;
    copy2->bufferSize.depthOrArrayLayers = 1;
}

void Recompute3DTextureCopyRegionWithEmptyFirstRowAndOddCopyHeight(Extent3D copySize,
                                                                   uint32_t bytesPerRow,
                                                                   TextureCopySubresource& copy,
                                                                   uint32_t i) {
    // Read the comments of Recompute3DTextureCopyRegionWithEmptyFirstRowAndEvenCopyHeight() for
    // the reason why it is incorrect if we simply extend the copy region to all depth slices
    // when there is an empty first row at the copy region.
    //
    // If the copy height is odd, we can use two copies to make it correct:
    //   - copy 0: only copy the first depth slice. Keep other arguments the same.
    //   - copy 1: copy all rest depth slices because it will start without an empty row if
    //     copy height is odd. Odd height + one (empty row) is even. An even row number times
    //     bytesPerRow (256) will be aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512)

    // Copy 0: copy the first depth slice (image 0)
    TextureCopySubresource::CopyInfo& copy0 = copy.copies[i];
    copy0.copySize.depthOrArrayLayers = 1;
    copy0.bufferSize.depthOrArrayLayers = 1;

    // Copy 1: copy the rest depth slices in one shot
    TextureCopySubresource::CopyInfo* copy1 = copy.AddCopy();
    *copy1 = copy0;
    ASSERT(copySize.height % 2 == 1);
    copy1->alignedOffset += (copySize.height + 1) * bytesPerRow;
    ASSERT(copy1->alignedOffset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0);
    // textureOffset.z should add one because the first slice has already been copied in copy0.
    copy1->textureOffset.z++;
    // bufferOffset.y should be 0 because we skipped the first depth slice and there is no empty
    // row in this copy region.
    copy1->bufferOffset.y = 0;
    copy1->copySize.height = copySize.height;
    copy1->copySize.depthOrArrayLayers = copySize.depthOrArrayLayers - 1;
    copy1->bufferSize.height = copySize.height;
    copy1->bufferSize.depthOrArrayLayers = copySize.depthOrArrayLayers - 1;
}

TextureCopySubresource Compute3DTextureCopySplits(Origin3D origin,
                                                  Extent3D copySize,
                                                  const TexelBlockInfo& blockInfo,
                                                  uint64_t offset,
                                                  uint32_t bytesPerRow,
                                                  uint32_t rowsPerImage) {
    // To compute the copy region(s) for 3D textures, we call Compute2DTextureCopySubresource
    // and get copy region(s) for the first slice of the copy, then extend to all depth slices
    // and become a 3D copy. However, this doesn't work as easily as that due to some corner
    // cases.
    //
    // For example, if bufferSize.height is greater than rowsPerImage in the generated copy
    // region and we simply extend the 2D copy region to all copied depth slices, copied data
    // will be incorrectly offset for each depth slice except the first one.
    //
    // For these special cases, we need to recompute the copy regions for 3D textures via
    // split the incorrect copy region to a couple more copy regions.

    // Call Compute2DTextureCopySubresource and get copy regions. This function has already
    // forwarded "copySize.depthOrArrayLayers" to all depth slices.
    TextureCopySubresource copySubresource =
        Compute2DTextureCopySubresource(origin, copySize, blockInfo, offset, bytesPerRow);

    ASSERT(copySubresource.count <= 2);
    // If copySize.depth is 1, we can return copySubresource. Because we don't need to extend
    // the copy region(s) to other depth slice(s).
    if (copySize.depthOrArrayLayers == 1) {
        return copySubresource;
    }

    uint32_t rowsPerImageInTexels = rowsPerImage * blockInfo.height;
    // The copy region(s) generated by Compute2DTextureCopySubresource might be incorrect.
    // However, we may append a couple more copy regions in the for loop below. We don't need
    // to revise these new added copy regions.
    uint32_t originalCopyCount = copySubresource.count;
    for (uint32_t i = 0; i < originalCopyCount; ++i) {
        // There can be one empty row at most in a copy region.
        ASSERT(copySubresource.copies[i].bufferSize.height <=
               rowsPerImageInTexels + blockInfo.height);
        Extent3D& bufferSize = copySubresource.copies[i].bufferSize;

        if (bufferSize.height == rowsPerImageInTexels) {
            // If the copy region's bufferSize.height equals to rowsPerImageInTexels, we can use
            // this copy region without any modification.
            continue;
        }

        if (bufferSize.height < rowsPerImageInTexels) {
            // If we are copying multiple depth slices, we should skip rowsPerImageInTexels rows
            // for each slice even though we only copy partial rows in each slice sometimes.
            bufferSize.height = rowsPerImageInTexels;
        } else {
            // bufferSize.height > rowsPerImageInTexels. There is an empty row in this copy
            // region due to alignment adjustment.

            // bytesPerRow is definitely 256, and it is definitely a full copy on height.
            // Otherwise, bufferSize.height wount be greater than rowsPerImageInTexels and
            // there won't be an empty row at the beginning of this copy region.
            ASSERT(bytesPerRow == D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
            ASSERT(copySize.height == rowsPerImageInTexels);

            if (copySize.height % 2 == 0) {
                // If copySize.height is even and there is an empty row at the beginning of the
                // first slice of the copy region, the offset of all depth slices will never be
                // aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512) and there is always
                // an empty row at each depth slice. We need a totally different approach to
                // split the copy region.
                Recompute3DTextureCopyRegionWithEmptyFirstRowAndEvenCopyHeight(
                    origin, copySize, blockInfo, bytesPerRow, rowsPerImage, copySubresource, i);
            } else {
                // If copySize.height is odd and there is an empty row at the beginning of the
                // first slice of the copy region, we can split the copy region into two copies:
                // copy0 to copy the first slice, copy1 to copy the rest slices because the
                // offset of slice 1 is aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512)
                // without an empty row. This is an easier case relative to cases with even copy
                // height.
                Recompute3DTextureCopyRegionWithEmptyFirstRowAndOddCopyHeight(copySize, bytesPerRow,
                                                                              copySubresource, i);
            }
        }
    }

    return copySubresource;
}
}  // namespace dawn::native::d3d12
