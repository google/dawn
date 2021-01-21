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

#include "tests/DawnTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class TextureSubresourceTest : public DawnTest {
  public:
    static constexpr uint32_t kSize = 4u;
    static constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture CreateTexture(uint32_t mipLevelCount,
                                uint32_t arrayLayerCount,
                                wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor texDesc;
        texDesc.dimension = wgpu::TextureDimension::e2D;
        texDesc.size = {kSize, kSize, arrayLayerCount};
        texDesc.sampleCount = 1;
        texDesc.mipLevelCount = mipLevelCount;
        texDesc.usage = usage;
        texDesc.format = kFormat;
        return device.CreateTexture(&texDesc);
    }

    wgpu::TextureView CreateTextureView(wgpu::Texture texture,
                                        uint32_t baseMipLevel,
                                        uint32_t baseArrayLayer) {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.format = kFormat;
        viewDesc.baseArrayLayer = baseArrayLayer;
        viewDesc.arrayLayerCount = 1;
        viewDesc.baseMipLevel = baseMipLevel;
        viewDesc.mipLevelCount = 1;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        return texture.CreateView(&viewDesc);
    }

    void DrawTriangle(const wgpu::TextureView& view) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(vertex_index)]] var<in> VertexIndex : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;

            [[stage(vertex)]] fn main() -> void {
                const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                    vec2<f32>(-1.0,  1.0),
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0, -1.0));

                Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<out> fragColor : vec4<f32>;
            [[stage(fragment)]] fn main() -> void {
                fragColor = vec4<f32>(1.0, 0.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cColorStates[0].format = kFormat;

        wgpu::RenderPipeline rp = device.CreateRenderPipeline(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor renderPassDesc({view});
        renderPassDesc.cColorAttachments[0].clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.SetPipeline(rp);
        pass.Draw(3);
        pass.EndPass();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    void SampleAndDraw(const wgpu::TextureView& samplerView, const wgpu::TextureView& renderView) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(vertex_index)]] var<in> VertexIndex : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;

            [[stage(vertex)]] fn main() -> void {
                const pos : array<vec2<f32>, 6> = array<vec2<f32>, 6>(
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0,  1.0),
                    vec2<f32>(-1.0,  1.0),
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0, -1.0),
                    vec2<f32>( 1.0,  1.0));

                Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[group(0), binding(0)]] var<uniform_constant> samp : sampler;
            [[group(0), binding(1)]] var<uniform_constant> tex : texture_2d<f32>;

            [[builtin(frag_coord)]] var<in> FragCoord : vec4<f32>;

            [[location(0)]] var<out> fragColor : vec4<f32>;

            [[stage(fragment)]] fn main() -> void {
                fragColor = textureSample(tex, samp, FragCoord.xy / vec2<f32>(4.0, 4.0));
            })");

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cColorStates[0].format = kFormat;

        wgpu::SamplerDescriptor samplerDescriptor = {};
        wgpu::Sampler sampler = device.CreateSampler(&samplerDescriptor);

        wgpu::RenderPipeline rp = device.CreateRenderPipeline(&descriptor);
        wgpu::BindGroupLayout bgl = rp.GetBindGroupLayout(0);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, bgl, {{0, sampler}, {1, samplerView}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor renderPassDesc({renderView});
        renderPassDesc.cColorAttachments[0].clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.SetPipeline(rp);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.EndPass();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }
};

// Test different mipmap levels
TEST_P(TextureSubresourceTest, MipmapLevelsTest) {
    // TODO(crbug.com/dawn/593): This test requires glTextureView, which is unsupported on GLES.
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    // Create a texture with 2 mipmap levels and 1 layer
    wgpu::Texture texture =
        CreateTexture(2, 1,
                      wgpu::TextureUsage::Sampled | wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::CopySrc);

    // Create two views on different mipmap levels.
    wgpu::TextureView samplerView = CreateTextureView(texture, 0, 0);
    wgpu::TextureView renderView = CreateTextureView(texture, 1, 0);

    // Draw a red triangle at the bottom-left half
    DrawTriangle(samplerView);

    // Sample from one subresource and draw into another subresource in the same texture
    SampleAndDraw(samplerView, renderView);

    // Verify that pixel at bottom-left corner is red, while pixel at top-right corner is background
    // black in render view (mip level 1).
    RGBA8 topRight = RGBA8::kBlack;
    RGBA8 bottomLeft = RGBA8::kRed;
    EXPECT_TEXTURE_RGBA8_EQ(&topRight, texture, kSize / 2 - 1, 0, 1, 1, 1, 0);
    EXPECT_TEXTURE_RGBA8_EQ(&bottomLeft, texture, 0, kSize / 2 - 1, 1, 1, 1, 0);
}

// Test different array layers
TEST_P(TextureSubresourceTest, ArrayLayersTest) {
    // TODO(crbug.com/dawn/593): This test requires glTextureView, which is unsupported on GLES.
    DAWN_SKIP_TEST_IF(IsOpenGLES());
    // Create a texture with 1 mipmap level and 2 layers
    wgpu::Texture texture =
        CreateTexture(1, 2,
                      wgpu::TextureUsage::Sampled | wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::CopySrc);

    // Create two views on different layers
    wgpu::TextureView samplerView = CreateTextureView(texture, 0, 0);
    wgpu::TextureView renderView = CreateTextureView(texture, 0, 1);

    // Draw a red triangle at the bottom-left half
    DrawTriangle(samplerView);

    // Sample from one subresource and draw into another subresource in the same texture
    SampleAndDraw(samplerView, renderView);

    // Verify that pixel at bottom-left corner is red, while pixel at top-right corner is background
    // black in render view (array layer 1).
    RGBA8 topRight = RGBA8::kBlack;
    RGBA8 bottomLeft = RGBA8::kRed;
    EXPECT_TEXTURE_RGBA8_EQ(&topRight, texture, kSize - 1, 0, 1, 1, 0, 1);
    EXPECT_TEXTURE_RGBA8_EQ(&bottomLeft, texture, 0, kSize - 1, 1, 1, 0, 1);
}

// TODO (yunchao.he@intel.com):
// * add tests for storage texture and sampler across miplevel or
// arraylayer dimensions in the same texture
//
// * add tests for copy operation upon texture subresource if needed
//
// * add tests for clear operation upon texture subresource if needed

DAWN_INSTANTIATE_TEST(TextureSubresourceTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
