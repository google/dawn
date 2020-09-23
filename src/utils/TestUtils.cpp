// Copyright 2020 The Dawn Authors
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

#include "utils/TestUtils.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "utils/TextureFormatUtils.h"
#include "utils/WGPUHelpers.h"

#include <vector>

namespace utils {

    uint32_t GetMinimumBytesPerRow(wgpu::TextureFormat format, uint32_t width) {
        const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(format);
        return Align(bytesPerTexel * width, kTextureBytesPerRowAlignment);
    }

    uint32_t GetBytesInBufferTextureCopy(wgpu::TextureFormat format,
                                         uint32_t width,
                                         uint32_t bytesPerRow,
                                         uint32_t rowsPerImage,
                                         uint32_t copyArrayLayerCount) {
        ASSERT(rowsPerImage > 0);
        const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(format);
        const uint32_t bytesAtLastImage = bytesPerRow * (rowsPerImage - 1) + bytesPerTexel * width;
        return bytesPerRow * rowsPerImage * (copyArrayLayerCount - 1) + bytesAtLastImage;
    }

    // TODO(jiawei.shao@intel.com): support compressed texture formats
    TextureDataCopyLayout GetTextureDataCopyLayoutForTexture2DAtLevel(
        wgpu::TextureFormat format,
        wgpu::Extent3D textureSizeAtLevel0,
        uint32_t mipmapLevel,
        uint32_t rowsPerImage) {
        TextureDataCopyLayout layout;

        layout.mipSize = {textureSizeAtLevel0.width >> mipmapLevel,
                          textureSizeAtLevel0.height >> mipmapLevel, textureSizeAtLevel0.depth};

        layout.bytesPerRow = GetMinimumBytesPerRow(format, layout.mipSize.width);

        uint32_t appliedRowsPerImage = rowsPerImage > 0 ? rowsPerImage : layout.mipSize.height;
        layout.bytesPerImage = layout.bytesPerRow * appliedRowsPerImage;

        layout.byteLength =
            GetBytesInBufferTextureCopy(format, layout.mipSize.width, layout.bytesPerRow,
                                        appliedRowsPerImage, textureSizeAtLevel0.depth);

        const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(format);
        layout.texelBlocksPerRow = layout.bytesPerRow / bytesPerTexel;
        layout.texelBlocksPerImage = layout.bytesPerImage / bytesPerTexel;
        layout.texelBlockCount = layout.byteLength / bytesPerTexel;

        return layout;
    }

    uint64_t RequiredBytesInCopy(uint64_t bytesPerRow,
                                 uint64_t rowsPerImage,
                                 wgpu::Extent3D copyExtent,
                                 wgpu::TextureFormat textureFormat) {
        if (copyExtent.width == 0 || copyExtent.height == 0 || copyExtent.depth == 0) {
            return 0;
        } else {
            uint32_t blockSize = utils::GetTexelBlockSizeInBytes(textureFormat);
            uint32_t blockWidth = utils::GetTextureFormatBlockWidth(textureFormat);
            uint32_t blockHeight = utils::GetTextureFormatBlockHeight(textureFormat);

            uint64_t texelBlockRowsPerImage = rowsPerImage / blockHeight;
            uint64_t bytesPerImage = bytesPerRow * texelBlockRowsPerImage;
            uint64_t bytesInLastSlice = bytesPerRow * (copyExtent.height / blockHeight - 1) +
                                        (copyExtent.width / blockWidth * blockSize);
            return bytesPerImage * (copyExtent.depth - 1) + bytesInLastSlice;
        }
    }

    uint64_t GetTexelCountInCopyRegion(uint64_t bytesPerRow,
                                       uint64_t rowsPerImage,
                                       wgpu::Extent3D copyExtent,
                                       wgpu::TextureFormat textureFormat) {
        return RequiredBytesInCopy(bytesPerRow, rowsPerImage, copyExtent, textureFormat) /
               utils::GetTexelBlockSizeInBytes(textureFormat);
    }

    void UnalignDynamicUploader(wgpu::Device device) {
        std::vector<uint8_t> data = {1};

        wgpu::TextureDescriptor descriptor = {};
        descriptor.size = {1, 1, 1};
        descriptor.format = wgpu::TextureFormat::R8Unorm;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        wgpu::TextureCopyView textureCopyView = utils::CreateTextureCopyView(texture, 0, {0, 0, 0});
        wgpu::TextureDataLayout textureDataLayout = utils::CreateTextureDataLayout(0, 0, 0);
        wgpu::Extent3D copyExtent = {1, 1, 1};

        // WriteTexture with exactly 1 byte of data.
        device.GetDefaultQueue().WriteTexture(&textureCopyView, data.data(), 1, &textureDataLayout,
                                              &copyExtent);
    }
}  // namespace utils