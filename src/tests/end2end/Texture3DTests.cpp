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

#include "tests/DawnTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/TestUtils.h"
#include "utils/WGPUHelpers.h"

constexpr static uint32_t kRTSize = 4;
constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

class Texture3DTests : public DawnTest {};

TEST_P(Texture3DTests, Sampling) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Set up pipeline. Two triangles will be drawn via the pipeline. They will fill the entire
    // color attachment with data sampled from 3D texture.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[stage(vertex)]]
        fn main([[builtin(vertex_index)]] VertexIndex : u32) -> [[builtin(position)]] vec4<f32> {
            var pos = array<vec2<f32>, 6>(
                vec2<f32>(-1.0, 1.0),
                vec2<f32>( -1.0, -1.0),
                vec2<f32>(1.0, 1.0),
                vec2<f32>(1.0, 1.0),
                vec2<f32>(-1.0, -1.0),
                vec2<f32>(1.0, -1.0));

            return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var samp : sampler;
        [[group(0), binding(1)]] var tex : texture_3d<f32>;

        [[stage(fragment)]]
        fn main([[builtin(position)]] FragCoord : vec4<f32>) -> [[location(0)]] vec4<f32> {
            return textureSample(tex, samp, vec3<f32>(FragCoord.xy / 4.0, 1.5 / 4.0));
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
    std::vector<RGBA8> data = std::vector<RGBA8>(size);
    for (uint32_t z = 0; z < copySize.depthOrArrayLayers; ++z) {
        for (uint32_t y = 0; y < copySize.height; ++y) {
            for (uint32_t x = 0; x < copySize.width; ++x) {
                uint32_t i = (z * copySize.height + y) * bytesPerRow / bytesPerTexel + x;
                data[i] = RGBA8(x, y, z, 255);
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
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // We sample data from the 3D texture at depth slice 1: 1.5 / 4.0 for z axis in textureSampler()
    // in shader, so the expected color at coordinate(x, y) should be (x, y, 1, 255).
    for (uint32_t i = 0; i < kRTSize; ++i) {
        for (uint32_t j = 0; j < kRTSize; ++j) {
            EXPECT_PIXEL_RGBA8_EQ(RGBA8(i, j, 1, 255), renderPass.color, i, j);
        }
    }
}

DAWN_INSTANTIATE_TEST(Texture3DTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
