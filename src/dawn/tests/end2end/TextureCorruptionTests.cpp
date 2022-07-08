// Copyright 2022 The Dawn Authors
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

#include <vector>

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {
enum class WriteType { WriteTexture, B2TCopy };

std::ostream& operator<<(std::ostream& o, WriteType writeType) {
    switch (writeType) {
        case WriteType::WriteTexture:
            o << "WriteTexture";
            break;
        case WriteType::B2TCopy:
            o << "B2TCopy";
            break;
    }
    return o;
}

using TextureFormat = wgpu::TextureFormat;
using TextureWidth = uint32_t;
using TextureHeight = uint32_t;

DAWN_TEST_PARAM_STRUCT(TextureCorruptionTestsParams,
                       TextureFormat,
                       TextureWidth,
                       TextureHeight,
                       WriteType);

}  // namespace

class TextureCorruptionTests : public DawnTestWithParams<TextureCorruptionTestsParams> {
  protected:
    std::ostringstream& DoTest(wgpu::Texture texture,
                               const wgpu::Extent3D textureSize,
                               uint32_t depthOrArrayLayer,
                               uint32_t copyValue,
                               uint32_t bytesPerTexel) {
        uint32_t bytesPerRow = Align(textureSize.width * bytesPerTexel, 256);
        uint64_t bufferSize = bytesPerRow * textureSize.height;
        wgpu::BufferDescriptor descriptor;
        descriptor.size = bufferSize;
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        wgpu::Buffer resultBuffer = device.CreateBuffer(&descriptor);

        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(texture, 0, {0, 0, depthOrArrayLayer});
        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(buffer, 0, bytesPerRow);
        wgpu::ImageCopyBuffer imageCopyResult =
            utils::CreateImageCopyBuffer(resultBuffer, 0, bytesPerRow);

        // Initialize the data to be copied
        wgpu::Extent3D copySize = {textureSize.width, textureSize.height, 1};

        // Data is stored in a uint32_t vector, so a single texel may require multiple vector
        // elements for some formats
        uint32_t elementNumPerTexel = bytesPerTexel / sizeof(uint32_t);
        uint32_t elementNumPerRow = bytesPerRow / sizeof(uint32_t);
        uint32_t elementNumInTotal = bufferSize / sizeof(uint32_t);
        std::vector<uint32_t> data(elementNumInTotal, 0);
        for (uint32_t i = 0; i < copySize.height; ++i) {
            for (uint32_t j = 0; j < copySize.width; ++j) {
                for (uint32_t k = 0; k < elementNumPerTexel; ++k) {
                    data[i * elementNumPerRow + j * elementNumPerTexel + k] = copyValue;
                    copyValue++;
                }
            }
        }

        // Copy data into the texture via B2T copy or WriteTexture
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        switch (GetParam().mWriteType) {
            case WriteType::B2TCopy: {
                queue.WriteBuffer(buffer, 0, data.data(), bufferSize);
                encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &copySize);
                break;
            }
            case WriteType::WriteTexture: {
                wgpu::TextureDataLayout textureDataLayout =
                    utils::CreateTextureDataLayout(0, bytesPerRow);
                queue.WriteTexture(&imageCopyTexture, data.data(), bufferSize, &textureDataLayout,
                                   &copySize);
                break;
            }
            default:
                break;
        }

        // Verify the data in copied texture via a T2B copy and comparison
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyResult, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        return EXPECT_BUFFER_U32_RANGE_EQ(data.data(), resultBuffer, 0, elementNumInTotal);
    }
};

TEST_P(TextureCorruptionTests, CopyTests) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());
    uint32_t width = GetParam().mTextureWidth;
    uint32_t height = GetParam().mTextureHeight;
    uint32_t depthOrArrayLayerCount = 2;
    wgpu::TextureFormat format = GetParam().mTextureFormat;
    wgpu::Extent3D textureSize = {width, height, depthOrArrayLayerCount};

    // Pre-allocate textures. The incorrect copy may corrupt neighboring textures or layers.
    wgpu::TextureDescriptor texDesc = {};
    texDesc.dimension = wgpu::TextureDimension::e2D;
    texDesc.size = textureSize;
    texDesc.mipLevelCount = 1;
    texDesc.format = format;
    texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    std::vector<wgpu::Texture> textures;
    uint32_t texNum = 2;
    for (uint32_t i = 0; i < texNum; ++i) {
        textures.push_back(device.CreateTexture(&texDesc));
    }

    // Copy data and verify the result one by one for every layer of every texture
    uint32_t copyValue = 100000000;
    for (uint32_t i = 0; i < texNum; ++i) {
        for (uint32_t j = 0; j < depthOrArrayLayerCount; ++j) {
            DoTest(textures[i], textureSize, j, copyValue, utils::GetTexelBlockSizeInBytes(format))
                << "texNum: " << i << ", layer: " << j;
            copyValue += 100000000;
        }
    }
}

DAWN_INSTANTIATE_TEST_P(TextureCorruptionTests,
                        {D3D12Backend()},
                        {wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA16Uint,
                         wgpu::TextureFormat::RGBA32Uint},
                        {100u, 200u, 300u, 400u, 500u, 600u, 700u, 800u, 900u, 1000u, 1200u},
                        {100u, 200u},
                        {WriteType::WriteTexture, WriteType::B2TCopy});
