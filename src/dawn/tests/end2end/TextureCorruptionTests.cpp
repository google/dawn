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
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

// 2D array textures with particular dimensions may corrupt on some devices. This test creates some
// 2d-array textures with different dimensions, and test them one by one. For each sub-test, the
// tested texture is written via different methods, then read back from the texture and verify the
// data.

constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

namespace {
enum class WriteType {
    ClearTexture,
    WriteTexture,    // Write the tested texture via writeTexture API
    B2TCopy,         // Write the tested texture via B2T copy
    RenderConstant,  // Write the tested texture via rendering the whole rectangle with solid color
                     // (0xFFFFFFFF)
    RenderFromTextureSample,  // Write the tested texture via sampling from a temp texture and
                              // writing the sampled data
    RenderFromTextureLoad     // Write the tested texture via textureLoad() from a temp texture and
                              // writing the loaded data
};

std::ostream& operator<<(std::ostream& o, WriteType writeType) {
    switch (writeType) {
        case WriteType::ClearTexture:
            o << "ClearTexture";
            break;
        case WriteType::WriteTexture:
            o << "WriteTexture";
            break;
        case WriteType::B2TCopy:
            o << "B2TCopy";
            break;
        case WriteType::RenderConstant:
            o << "RenderConstant";
            break;
        case WriteType::RenderFromTextureSample:
            o << "RenderFromTextureSample";
            break;
        case WriteType::RenderFromTextureLoad:
            o << "RenderFromTextureLoad";
            break;
    }
    return o;
}

using TextureFormat = wgpu::TextureFormat;
using TextureWidth = uint32_t;
using TextureHeight = uint32_t;

DAWN_TEST_PARAM_STRUCT(TextureCorruptionTestsParams, TextureWidth, TextureHeight, WriteType);

}  // namespace

class TextureCorruptionTests : public DawnTestWithParams<TextureCorruptionTestsParams> {
  protected:
    std::ostringstream& DoTest(wgpu::Texture texture,
                               const wgpu::Extent3D textureSize,
                               uint32_t depthOrArrayLayer,
                               uint32_t srcValue) {
        uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(kFormat);
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

        WriteType type = GetParam().mWriteType;

        // Fill data into a buffer
        wgpu::Extent3D copySize = {textureSize.width, textureSize.height, 1};

        // Data is stored in a uint32_t vector, so a single texel may require multiple vector
        // elements for some formats
        ASSERT(bytesPerTexel = sizeof(uint32_t));
        uint32_t elementNumPerRow = bytesPerRow / sizeof(uint32_t);
        uint32_t elementNumInTotal = bufferSize / sizeof(uint32_t);
        std::vector<uint32_t> data(elementNumInTotal, 0);
        for (uint32_t i = 0; i < copySize.height; ++i) {
            for (uint32_t j = 0; j < copySize.width; ++j) {
                if (type == WriteType::RenderFromTextureSample ||
                    type == WriteType::RenderConstant) {
                    // Fill a simple and constant value (0xFFFFFFFF) in the whole buffer for
                    // texture sampling and rendering because either sampling operation will
                    // lead to precision loss or rendering a solid color is easier to implement and
                    // compare.
                    data[i * elementNumPerRow + j] = 0xFFFFFFFF;
                } else if (type != WriteType::ClearTexture) {
                    data[i * elementNumPerRow + j] = srcValue;
                    srcValue++;
                }
            }
        }

        // Write data into the given layer via various write types
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        switch (type) {
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
            case WriteType::RenderConstant:
            case WriteType::RenderFromTextureSample:
            case WriteType::RenderFromTextureLoad: {
                // Write data into a single layer temp texture and read from this texture if needed
                wgpu::TextureView tempView;
                if (type != WriteType::RenderConstant) {
                    wgpu::Texture tempTexture = Create2DTexture(copySize);
                    wgpu::ImageCopyTexture imageCopyTempTexture =
                        utils::CreateImageCopyTexture(tempTexture, 0, {0, 0, 0});
                    wgpu::TextureDataLayout textureDataLayout =
                        utils::CreateTextureDataLayout(0, bytesPerRow);
                    queue.WriteTexture(&imageCopyTempTexture, data.data(), bufferSize,
                                       &textureDataLayout, &copySize);
                    tempView = tempTexture.CreateView();
                }

                // Write into the specified layer of a 2D array texture
                wgpu::TextureViewDescriptor viewDesc;
                viewDesc.format = kFormat;
                viewDesc.dimension = wgpu::TextureViewDimension::e2D;
                viewDesc.baseMipLevel = 0;
                viewDesc.mipLevelCount = 1;
                viewDesc.baseArrayLayer = depthOrArrayLayer;
                viewDesc.arrayLayerCount = 1;
                CreatePipelineAndRender(texture.CreateView(&viewDesc), tempView, encoder, type);
                break;
            }
            default:
                break;
        }

        // Verify the data in texture via a T2B copy and comparison
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyResult, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        return EXPECT_BUFFER_U32_RANGE_EQ(data.data(), resultBuffer, 0, elementNumInTotal);
    }

    void CreatePipelineAndRender(wgpu::TextureView renderView,
                                 wgpu::TextureView samplerView,
                                 wgpu::CommandEncoder encoder,
                                 WriteType type) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.cTargets[0].format = kFormat;

        // Draw the whole texture (a rectangle) via two triangles
        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                var pos = array<vec2<f32>, 6>(
                    vec2<f32>(-1.0,  1.0),
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0,  1.0),
                    vec2<f32>( 1.0,  1.0),
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0, -1.0));
                return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        if (type == WriteType::RenderConstant) {
            pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment
            fn main(@builtin(position) FragCoord : vec4<f32>) -> @location(0) vec4<f32> {
                return vec4<f32>(1.0, 1.0, 1.0, 1.0);
            })");
        } else if (type == WriteType::RenderFromTextureSample) {
            pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var samp : sampler;
            @group(0) @binding(1) var tex : texture_2d<f32>;

            @fragment
            fn main(@builtin(position) FragCoord : vec4<f32>) -> @location(0) vec4<f32> {
                return textureSample(tex, samp, FragCoord.xy);
            })");
        } else {
            pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var tex : texture_2d<f32>;

            @fragment
            fn main(@builtin(position) Fragcoord: vec4<f32>) -> @location(0) vec4<f32> {
                return textureLoad(tex, vec2<i32>(Fragcoord.xy), 0);
            })");
        }

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

        utils::ComboRenderPassDescriptor renderPassDescriptor({renderView});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.SetPipeline(pipeline);
        if (type != WriteType::RenderConstant) {
            wgpu::BindGroup bindGroup;
            if (type == WriteType::RenderFromTextureLoad) {
                bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                 {{0, samplerView}});
            } else {
                bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                 {{0, device.CreateSampler()}, {1, samplerView}});
            }
            pass.SetBindGroup(0, bindGroup);
        }
        pass.Draw(6);
        pass.End();
    }

    wgpu::Texture Create2DTexture(const wgpu::Extent3D size) {
        wgpu::TextureDescriptor texDesc = {};
        texDesc.dimension = wgpu::TextureDimension::e2D;
        texDesc.size = size;
        texDesc.mipLevelCount = 1;
        texDesc.format = kFormat;
        texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc |
                        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
        return device.CreateTexture(&texDesc);
    }
};

TEST_P(TextureCorruptionTests, Tests) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());
    uint32_t width = GetParam().mTextureWidth;
    uint32_t height = GetParam().mTextureHeight;
    uint32_t depthOrArrayLayerCount = 2;
    wgpu::Extent3D textureSize = {width, height, depthOrArrayLayerCount};

    // Pre-allocate textures. The incorrect write type may corrupt neighboring textures or layers.
    std::vector<wgpu::Texture> textures;
    uint32_t texNum = 2;
    for (uint32_t i = 0; i < texNum; ++i) {
        textures.push_back(Create2DTexture(textureSize));
    }

    // Write data and verify the result one by one for every layer of every texture
    uint32_t srcValue = 100000000;
    for (uint32_t i = 0; i < texNum; ++i) {
        for (uint32_t j = 0; j < depthOrArrayLayerCount; ++j) {
            DoTest(textures[i], textureSize, j, srcValue) << "texNum: " << i << ", layer: " << j;
            srcValue += 100000000;
        }
    }
}

DAWN_INSTANTIATE_TEST_P(TextureCorruptionTests,
                        {D3D12Backend()},
                        {100u, 200u, 300u, 400u, 500u, 600u, 700u, 800u, 900u, 1000u, 1200u},
                        {100u, 200u},
                        {WriteType::ClearTexture, WriteType::WriteTexture, WriteType::B2TCopy,
                         WriteType::RenderConstant, WriteType::RenderFromTextureSample,
                         WriteType::RenderFromTextureLoad});
