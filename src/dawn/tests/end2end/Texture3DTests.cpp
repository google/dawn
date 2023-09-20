// Copyright 2021 The Dawn Authors
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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static uint32_t kRTSize = 4;
constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

class Texture3DTests : public DawnTest {};

TEST_P(Texture3DTests, Sampling) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Set up pipeline. Two triangles will be drawn via the pipeline. They will fill the entire
    // color attachment with data sampled from 3D texture.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, 1.0),
                vec2f( -1.0, -1.0),
                vec2f(1.0, 1.0),
                vec2f(1.0, 1.0),
                vec2f(-1.0, -1.0),
                vec2f(1.0, -1.0));

            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var samp : sampler;
        @group(0) @binding(1) var tex : texture_3d<f32>;

        @fragment
        fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
            return textureSample(tex, samp, vec3f(FragCoord.xy / 4.0, 1.5 / 4.0));
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::Extent3D copySize = {kRTSize, kRTSize, kRTSize};

    // Create a 3D texture, fill the texture via a B2T copy with well-designed data.
    // The 3D texture will be used as the data source of a sampler in shader.
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e3D;
    descriptor.size = copySize;
    descriptor.format = kFormat;
    descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    wgpu::TextureView textureView = texture.CreateView();

    uint32_t bytesPerRow = utils::GetMinimumBytesPerRow(kFormat, copySize.width);
    uint32_t sizeInBytes =
        utils::RequiredBytesInCopy(bytesPerRow, copySize.height, copySize, kFormat);
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(kFormat);
    uint32_t size = sizeInBytes / bytesPerTexel;
    std::vector<utils::RGBA8> data = std::vector<utils::RGBA8>(size);
    for (uint32_t z = 0; z < copySize.depthOrArrayLayers; ++z) {
        for (uint32_t y = 0; y < copySize.height; ++y) {
            for (uint32_t x = 0; x < copySize.width; ++x) {
                uint32_t i = (z * copySize.height + y) * bytesPerRow / bytesPerTexel + x;
                data[i] = utils::RGBA8(x, y, z, 255);
            }
        }
    }
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, data.data(), sizeInBytes, wgpu::BufferUsage::CopySrc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::ImageCopyBuffer imageCopyBuffer =
        utils::CreateImageCopyBuffer(buffer, 0, bytesPerRow, copySize.height);
    wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(texture, 0, {0, 0, 0});
    encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &copySize);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampler}, {1, textureView}});

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(6);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // We sample data from the 3D texture at depth slice 1: 1.5 / 4.0 for z axis in textureSampler()
    // in shader, so the expected color at coordinate(x, y) should be (x, y, 1, 255).
    for (uint32_t i = 0; i < kRTSize; ++i) {
        for (uint32_t j = 0; j < kRTSize; ++j) {
            EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(i, j, 1, 255), renderPass.color, i, j);
        }
    }
}

// Regression test for crbug.com/dawn/2072 where the WSize of D3D UAV descriptor ends up being 0.
// (which is invalid as noted by the debug layers)
TEST_P(Texture3DTests, LatestMipClampsDepthSizeForStorageTextures) {
    wgpu::TextureDescriptor tDesc;
    tDesc.dimension = wgpu::TextureDimension::e3D;
    tDesc.size = {2, 2, 1};
    tDesc.mipLevelCount = 2;
    tDesc.usage = wgpu::TextureUsage::StorageBinding;
    tDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::Texture t = device.CreateTexture(&tDesc);

    wgpu::TextureViewDescriptor vDesc;
    vDesc.baseMipLevel = 1;
    vDesc.mipLevelCount = 1;
    wgpu::TextureView v = t.CreateView(&vDesc);

    wgpu::ComputePipelineDescriptor pDesc;
    pDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var t : texture_storage_3d<r32uint, write>;
        @compute @workgroup_size(1) fn main() {
            _ = t;
        }
    )");
    pDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, v}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetBindGroup(0, bg);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

DAWN_INSTANTIATE_TEST(Texture3DTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
