// Copyright 2017 The NXT Authors
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

#include "backend/d3d12/TextureCopySplitter.h"

#include "backend/d3d12/d3d12_platform.h"
#include "common/Assert.h"

namespace backend { namespace d3d12 {

    namespace {
        void ComputeTexelOffsets(uint32_t offset,
                                 uint32_t rowPitch,
                                 uint32_t slicePitch,
                                 uint32_t texelSize,
                                 uint32_t* texelOffsetX,
                                 uint32_t* texelOffsetY,
                                 uint32_t* texelOffsetZ) {
            uint32_t byteOffsetX = offset % rowPitch;
            offset -= byteOffsetX;
            uint32_t byteOffsetY = offset % slicePitch;
            uint32_t byteOffsetZ = offset - byteOffsetY;

            *texelOffsetX = byteOffsetX / texelSize;
            *texelOffsetY = byteOffsetY / rowPitch;
            *texelOffsetZ = byteOffsetZ / slicePitch;
        }
    }  // namespace

    TextureCopySplit ComputeTextureCopySplit(uint32_t x,
                                             uint32_t y,
                                             uint32_t z,
                                             uint32_t width,
                                             uint32_t height,
                                             uint32_t depth,
                                             uint32_t texelSize,
                                             uint32_t offset,
                                             uint32_t rowPitch) {
        TextureCopySplit copy;

        if (z != 0 || depth > 1) {
            // TODO(enga@google.com): Handle 3D / 2D arrays
            ASSERT(false);
            return copy;
        }

        ASSERT(rowPitch % texelSize == 0);

        uint32_t alignedOffset = offset & ~(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);

        copy.offset = alignedOffset;
        if (offset == alignedOffset) {
            copy.count = 1;

            copy.copies[0].textureOffset.x = x;
            copy.copies[0].textureOffset.y = y;
            copy.copies[0].textureOffset.z = z;

            copy.copies[0].copySize.width = width;
            copy.copies[0].copySize.height = height;
            copy.copies[0].copySize.depth = depth;

            copy.copies[0].bufferOffset.x = 0;
            copy.copies[0].bufferOffset.y = 0;
            copy.copies[0].bufferOffset.z = 0;
            copy.copies[0].bufferSize.width = width;
            copy.copies[0].bufferSize.height = height;
            copy.copies[0].bufferSize.depth = depth;

            // Return early. There is only one copy needed because the offset is already 512-byte
            // aligned
            return copy;
        }

        ASSERT(alignedOffset < offset);

        uint32_t texelOffsetX, texelOffsetY, texelOffsetZ;
        ComputeTexelOffsets(offset - alignedOffset, rowPitch, rowPitch * height, texelSize,
                            &texelOffsetX, &texelOffsetY, &texelOffsetZ);

        uint32_t rowPitchInTexels = rowPitch / texelSize;

        if (width + texelOffsetX <= rowPitchInTexels) {
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

            copy.copies[0].textureOffset.x = x;
            copy.copies[0].textureOffset.y = y;
            copy.copies[0].textureOffset.z = z;

            copy.copies[0].copySize.width = width;
            copy.copies[0].copySize.height = height;
            copy.copies[0].copySize.depth = depth;

            copy.copies[0].bufferOffset.x = texelOffsetX;
            copy.copies[0].bufferOffset.y = texelOffsetY;
            copy.copies[0].bufferOffset.z = texelOffsetZ;
            copy.copies[0].bufferSize.width = width + texelOffsetX;
            copy.copies[0].bufferSize.height = height + texelOffsetY;
            copy.copies[0].bufferSize.depth = depth + texelOffsetZ;

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

        copy.copies[0].textureOffset.x = x;
        copy.copies[0].textureOffset.y = y;
        copy.copies[0].textureOffset.z = z;

        ASSERT(rowPitchInTexels > texelOffsetX);
        copy.copies[0].copySize.width = rowPitchInTexels - texelOffsetX;
        copy.copies[0].copySize.height = height;
        copy.copies[0].copySize.depth = depth;

        copy.copies[0].bufferOffset.x = texelOffsetX;
        copy.copies[0].bufferOffset.y = texelOffsetY;
        copy.copies[0].bufferOffset.z = texelOffsetZ;
        copy.copies[0].bufferSize.width = rowPitchInTexels;
        copy.copies[0].bufferSize.height = height + texelOffsetY;
        copy.copies[0].bufferSize.depth = depth + texelOffsetZ;

        copy.copies[1].textureOffset.x = x + copy.copies[0].copySize.width;
        copy.copies[1].textureOffset.y = y;
        copy.copies[1].textureOffset.z = z;

        ASSERT(width > copy.copies[0].copySize.width);
        copy.copies[1].copySize.width = width - copy.copies[0].copySize.width;
        copy.copies[1].copySize.height = height;
        copy.copies[1].copySize.depth = depth;

        copy.copies[1].bufferOffset.x = 0;
        copy.copies[1].bufferOffset.y = texelOffsetY + 1;
        copy.copies[1].bufferOffset.z = texelOffsetZ;
        copy.copies[1].bufferSize.width = copy.copies[1].copySize.width;
        copy.copies[1].bufferSize.height = height + texelOffsetY + 1;
        copy.copies[1].bufferSize.depth = depth + texelOffsetZ;

        return copy;
    }

}}  // namespace backend::d3d12
