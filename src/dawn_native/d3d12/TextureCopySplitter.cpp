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
        Origin3D ComputeTexelOffsets(const Format& format,
                                     uint32_t offset,
                                     uint32_t rowPitch,
                                     uint32_t slicePitch) {
            uint32_t byteOffsetX = offset % rowPitch;
            offset -= byteOffsetX;
            uint32_t byteOffsetY = offset % slicePitch;
            uint32_t byteOffsetZ = offset - byteOffsetY;

            return {byteOffsetX / format.blockByteSize * format.blockWidth,
                    byteOffsetY / rowPitch * format.blockHeight, byteOffsetZ / slicePitch};
        }
    }  // namespace

    TextureCopySplit ComputeTextureCopySplit(Origin3D origin,
                                             Extent3D copySize,
                                             const Format& format,
                                             uint64_t offset,
                                             uint32_t rowPitch,
                                             uint32_t imageHeight) {
        TextureCopySplit copy;

        ASSERT(rowPitch % format.blockByteSize == 0);

        uint64_t alignedOffset =
            offset & ~static_cast<uint64_t>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);

        copy.offset = alignedOffset;
        if (offset == alignedOffset) {
            copy.count = 1;

            copy.copies[0].textureOffset = origin;

            copy.copies[0].copySize = copySize;

            copy.copies[0].bufferOffset.x = 0;
            copy.copies[0].bufferOffset.y = 0;
            copy.copies[0].bufferOffset.z = 0;
            copy.copies[0].bufferSize = copySize;

            // Return early. There is only one copy needed because the offset is already 512-byte
            // aligned
            return copy;
        }

        ASSERT(alignedOffset < offset);
        ASSERT(offset - alignedOffset < D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

        uint32_t slicePitch = rowPitch * (imageHeight / format.blockHeight);
        Origin3D texelOffset = ComputeTexelOffsets(
            format, static_cast<uint32_t>(offset - alignedOffset), rowPitch, slicePitch);

        uint32_t copyBytesPerRowPitch = copySize.width / format.blockWidth * format.blockByteSize;
        uint32_t byteOffsetInRowPitch = texelOffset.x / format.blockWidth * format.blockByteSize;
        if (copyBytesPerRowPitch + byteOffsetInRowPitch <= rowPitch) {
            // The region's rows fit inside the row pitch. In this case, extend the width of the
            // PlacedFootprint and copy the buffer with an offset location
            //  |<--------------- row pitch --------------->|
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

            copy.copies[0].textureOffset = origin;

            copy.copies[0].copySize = copySize;

            copy.copies[0].bufferOffset = texelOffset;
            copy.copies[0].bufferSize.width = copySize.width + texelOffset.x;
            copy.copies[0].bufferSize.height = imageHeight + texelOffset.y;
            copy.copies[0].bufferSize.depth = copySize.depth + texelOffset.z;

            return copy;
        }

        // The region's rows straddle the row pitch. Split the copy into two copies
        //  |<--------------- row pitch --------------->|
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

        copy.copies[0].textureOffset = origin;

        ASSERT(rowPitch > byteOffsetInRowPitch);
        uint32_t texelsPerRow = rowPitch / format.blockByteSize * format.blockWidth;
        copy.copies[0].copySize.width = texelsPerRow - texelOffset.x;
        copy.copies[0].copySize.height = copySize.height;
        copy.copies[0].copySize.depth = copySize.depth;

        copy.copies[0].bufferOffset = texelOffset;
        copy.copies[0].bufferSize.width = texelsPerRow;
        copy.copies[0].bufferSize.height = imageHeight + texelOffset.y;
        copy.copies[0].bufferSize.depth = copySize.depth + texelOffset.z;

        copy.copies[1].textureOffset.x = origin.x + copy.copies[0].copySize.width;
        copy.copies[1].textureOffset.y = origin.y;
        copy.copies[1].textureOffset.z = origin.z;

        ASSERT(copySize.width > copy.copies[0].copySize.width);
        copy.copies[1].copySize.width = copySize.width - copy.copies[0].copySize.width;
        copy.copies[1].copySize.height = copySize.height;
        copy.copies[1].copySize.depth = copySize.depth;

        copy.copies[1].bufferOffset.x = 0;
        copy.copies[1].bufferOffset.y = texelOffset.y + format.blockHeight;
        copy.copies[1].bufferOffset.z = texelOffset.z;
        copy.copies[1].bufferSize.width = copy.copies[1].copySize.width;
        copy.copies[1].bufferSize.height = imageHeight + texelOffset.y + format.blockHeight;
        copy.copies[1].bufferSize.depth = copySize.depth + texelOffset.z;

        return copy;
    }

}}  // namespace dawn_native::d3d12
