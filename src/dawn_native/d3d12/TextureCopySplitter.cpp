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

#include "dawn_native/d3d12/TextureCopySplitter.h"

#include "common/Assert.h"
#include "dawn_native/Format.h"
#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

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
    }  // namespace

    TextureCopySubresource Compute2DTextureCopySubresource(Origin3D origin,
                                                           Extent3D copySize,
                                                           const TexelBlockInfo& blockInfo,
                                                           uint64_t offset,
                                                           uint32_t bytesPerRow) {
        TextureCopySubresource copy;

        ASSERT(bytesPerRow % blockInfo.byteSize == 0);

        // The copies must be 512-aligned. To do this, we calculate the first 512-aligned address
        // preceding our data.
        uint64_t alignedOffset =
            offset & ~static_cast<uint64_t>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);

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
        Origin3D texelOffset = ComputeTexelOffsets(
            blockInfo, static_cast<uint32_t>(offset - alignedOffset), bytesPerRow);

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
        uint64_t alignedOffsetForCopy1 =
            offsetForCopy1 & ~static_cast<uint64_t>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
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
            copies.copySubresources[1] =
                Compute2DTextureCopySubresource(copyFirstLayerOrigin, copyOneLayerSize, blockInfo,
                                                bufferOffsetNextLayer, bytesPerRow);
        }

        return copies;
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
        // will be incorrectly offset for each depth slice except the first one. This situation
        // can be introduced by 2 special cases:
        //   - If there is an empty row at the beginning of the copy region due to alignment.
        //   - If copySize.height is 1 and one row of data straddle two rows.
        //
        // For these special cases, we need to recompute the copy regions for 3D textures like
        // 1) split and modify the incorrect copy region to a few more copy regions, or 2) abandon
        // the old copy region and regenerate the copy regions in different approach.

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

        bool needRecompute = false;
        uint32_t rowsPerImageInTexels = rowsPerImage * blockInfo.height;
        for (uint32_t i = 0; i < copySubresource.count; ++i) {
            // There can be one empty row at most in a copy region.
            ASSERT(copySubresource.copies[i].bufferSize.height <= rowsPerImage + blockInfo.height);
            Extent3D& bufferSize = copySubresource.copies[i].bufferSize;
            if (bufferSize.height > rowsPerImageInTexels) {
                needRecompute = true;
            } else if (bufferSize.height < rowsPerImageInTexels) {
                // If we are copying multiple depth slices, we should skip rowsPerImageInTexels rows
                // at least for each slice even though we only copy partial rows in each slice
                // sometimes.
                bufferSize.height = rowsPerImageInTexels;
            }
        }

        if (!needRecompute) {
            return copySubresource;
        }

        // TODO(yunchao.he@intel.com): recompute copy regions for special cases for 3D textures,
        // and return the revised copy regions.
        ASSERT(bytesPerRow == D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        return copySubresource;
    }
}}  // namespace dawn_native::d3d12
