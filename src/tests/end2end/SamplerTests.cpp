// Copyright 2018 The Dawn Authors
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

#include <cmath>

#include "tests/DawnTest.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

constexpr static unsigned int kRTSize = 64;

namespace {
    struct AddressModeTestCase {
        wgpu::AddressMode mMode;
        uint8_t mExpected2;
        uint8_t mExpected3;
    };
    AddressModeTestCase addressModes[] = {
        {
            wgpu::AddressMode::Repeat,
            0,
            255,
        },
        {
            wgpu::AddressMode::MirrorRepeat,
            255,
            0,
        },
        {
            wgpu::AddressMode::ClampToEdge,
            255,
            255,
        },
    };
}  // namespace

class SamplerTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        mRenderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        auto vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;

            [[stage(vertex)]] fn main() -> void {
                const pos : array<vec2<f32>, 6> = array<vec2<f32>, 6>(
                    vec2<f32>(-2.0, -2.0),
                    vec2<f32>(-2.0,  2.0),
                    vec2<f32>( 2.0, -2.0),
                    vec2<f32>(-2.0,  2.0),
                    vec2<f32>( 2.0, -2.0),
                    vec2<f32>( 2.0,  2.0));
                Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            }
        )");
        auto fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[group(0), binding(0)]] var<uniform_constant> sampler0 : sampler;
            [[group(0), binding(1)]] var<uniform_constant> texture0 : texture_2d<f32>;

            [[builtin(frag_coord)]] var<in> FragCoord : vec4<f32>;

            [[location(0)]] var<out> fragColor : vec4<f32>;

            [[stage(fragment)]] fn main() -> void {
                fragColor = textureSample(texture0, sampler0, FragCoord.xy / vec2<f32>(2.0, 2.0));
            })");

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.vertexStage.module = vsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        pipelineDescriptor.cColorStates[0].format = mRenderPass.colorFormat;

        mPipeline = device.CreateRenderPipeline(&pipelineDescriptor);
        mBindGroupLayout = mPipeline.GetBindGroupLayout(0);

        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = 2;
        descriptor.size.height = 2;
        descriptor.size.depth = 1;
        descriptor.sampleCount = 1;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::Sampled;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        // Create a 2x2 checkerboard texture, with black in the top left and bottom right corners.
        const uint32_t rowPixels = kTextureBytesPerRowAlignment / sizeof(RGBA8);
        RGBA8 data[rowPixels * 2];
        data[0] = data[rowPixels + 1] = RGBA8::kBlack;
        data[1] = data[rowPixels] = RGBA8::kWhite;

        wgpu::Buffer stagingBuffer =
            utils::CreateBufferFromData(device, data, sizeof(data), wgpu::BufferUsage::CopySrc);
        wgpu::BufferCopyView bufferCopyView = utils::CreateBufferCopyView(stagingBuffer, 0, 256);
        wgpu::TextureCopyView textureCopyView = utils::CreateTextureCopyView(texture, 0, {0, 0, 0});
        wgpu::Extent3D copySize = {2, 2, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copySize);

        wgpu::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);

        mTextureView = texture.CreateView();
    }

    void TestAddressModes(AddressModeTestCase u, AddressModeTestCase v, AddressModeTestCase w) {
        wgpu::Sampler sampler;
        {
            wgpu::SamplerDescriptor descriptor = {};
            descriptor.minFilter = wgpu::FilterMode::Nearest;
            descriptor.magFilter = wgpu::FilterMode::Nearest;
            descriptor.mipmapFilter = wgpu::FilterMode::Nearest;
            descriptor.addressModeU = u.mMode;
            descriptor.addressModeV = v.mMode;
            descriptor.addressModeW = w.mMode;
            sampler = device.CreateSampler(&descriptor);
        }

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, mBindGroupLayout, {{0, sampler}, {1, mTextureView}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&mRenderPass.renderPassInfo);
            pass.SetPipeline(mPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.EndPass();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        RGBA8 expectedU2(u.mExpected2, u.mExpected2, u.mExpected2, 255);
        RGBA8 expectedU3(u.mExpected3, u.mExpected3, u.mExpected3, 255);
        RGBA8 expectedV2(v.mExpected2, v.mExpected2, v.mExpected2, 255);
        RGBA8 expectedV3(v.mExpected3, v.mExpected3, v.mExpected3, 255);
        EXPECT_PIXEL_RGBA8_EQ(RGBA8::kBlack, mRenderPass.color, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(RGBA8::kWhite, mRenderPass.color, 0, 1);
        EXPECT_PIXEL_RGBA8_EQ(RGBA8::kWhite, mRenderPass.color, 1, 0);
        EXPECT_PIXEL_RGBA8_EQ(RGBA8::kBlack, mRenderPass.color, 1, 1);
        EXPECT_PIXEL_RGBA8_EQ(expectedU2, mRenderPass.color, 2, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedU3, mRenderPass.color, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedV2, mRenderPass.color, 0, 2);
        EXPECT_PIXEL_RGBA8_EQ(expectedV3, mRenderPass.color, 0, 3);
        // TODO: add tests for W address mode, once Dawn supports 3D textures
    }

    utils::BasicRenderPass mRenderPass;
    wgpu::BindGroupLayout mBindGroupLayout;
    wgpu::RenderPipeline mPipeline;
    wgpu::TextureView mTextureView;
};

// Test drawing a rect with a checkerboard texture with different address modes.
TEST_P(SamplerTest, AddressMode) {
    for (auto u : addressModes) {
        for (auto v : addressModes) {
            for (auto w : addressModes) {
                TestAddressModes(u, v, w);
            }
        }
    }
}

DAWN_INSTANTIATE_TEST(SamplerTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
