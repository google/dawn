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

#include <cmath>

#include "tests/DawnTest.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

constexpr static unsigned int kRTSize = 16;

namespace {
    // MipLevel colors, ordering from base level to high level
    // each mipmap of the texture is having a different color
    // so we can check if the sampler anisotropic filtering is fetching
    // from the correct miplevel
    const std::array<RGBA8, 3> colors = {RGBA8::kRed, RGBA8::kGreen, RGBA8::kBlue};
}  // namespace

class SamplerFilterAnisotropicTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        mRenderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[block]] struct Uniforms {
                [[offset(0)]] matrix : mat4x4<f32>;
            };

            [[location(0)]] var<in> position : vec4<f32>;
            [[location(1)]] var<in> uv : vec2<f32>;

            [[set(0), binding(2)]] var<uniform> uniforms : Uniforms;

            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[location(0)]] var<out> fragUV : vec2<f32>;

            [[stage(vertex)]] fn main() -> void {
                fragUV = uv;
                Position = uniforms.matrix * position;
            }
        )");
        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[set(0), binding(0)]] var<uniform_constant> sampler0 : sampler;
            [[set(0), binding(1)]] var<uniform_constant> texture0 : texture_2d<f32>;

            [[builtin(frag_coord)]] var<in> FragCoord : vec4<f32>;

            [[location(0)]] var<in> fragUV: vec2<f32>;

            [[location(0)]] var<out> fragColor : vec4<f32>;

            [[stage(fragment)]] fn main() -> void {
                fragColor = textureSample(texture0, sampler0, fragUV);
            })");

        utils::ComboVertexStateDescriptor vertexState;
        vertexState.cVertexBuffers[0].attributeCount = 2;
        vertexState.cAttributes[0].format = wgpu::VertexFormat::Float4;
        vertexState.cAttributes[1].shaderLocation = 1;
        vertexState.cAttributes[1].offset = 4 * sizeof(float);
        vertexState.cAttributes[1].format = wgpu::VertexFormat::Float2;
        vertexState.vertexBufferCount = 1;
        vertexState.cVertexBuffers[0].arrayStride = 6 * sizeof(float);

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.vertexStage.module = vsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        pipelineDescriptor.vertexState = &vertexState;
        pipelineDescriptor.cColorStates[0].format = mRenderPass.colorFormat;

        mPipeline = device.CreateRenderPipeline(&pipelineDescriptor);
        mBindGroupLayout = mPipeline.GetBindGroupLayout(0);

        InitTexture();
    }

    void InitTexture() {
        const uint32_t mipLevelCount = colors.size();

        const uint32_t textureWidthLevel0 = 1 << (mipLevelCount - 1);
        const uint32_t textureHeightLevel0 = 1 << (mipLevelCount - 1);

        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = textureWidthLevel0;
        descriptor.size.height = textureHeightLevel0;
        descriptor.size.depth = 1;
        descriptor.sampleCount = 1;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::Sampled;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        const uint32_t rowPixels = kTextureBytesPerRowAlignment / sizeof(RGBA8);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Populate each mip level with a different color
        for (uint32_t level = 0; level < mipLevelCount; ++level) {
            const uint32_t texWidth = textureWidthLevel0 >> level;
            const uint32_t texHeight = textureHeightLevel0 >> level;

            const RGBA8 color = colors[level];

            std::vector<RGBA8> data(rowPixels * texHeight, color);
            wgpu::Buffer stagingBuffer = utils::CreateBufferFromData(
                device, data.data(), data.size() * sizeof(RGBA8), wgpu::BufferUsage::CopySrc);
            wgpu::BufferCopyView bufferCopyView =
                utils::CreateBufferCopyView(stagingBuffer, 0, kTextureBytesPerRowAlignment);
            wgpu::TextureCopyView textureCopyView =
                utils::CreateTextureCopyView(texture, level, {0, 0, 0});
            wgpu::Extent3D copySize = {texWidth, texHeight, 1};
            encoder.CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copySize);
        }
        wgpu::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);

        mTextureView = texture.CreateView();
    }

    // void TestFilterAnisotropic(const FilterAnisotropicTestCase& testCase) {
    void TestFilterAnisotropic(const uint16_t maxAnisotropy) {
        wgpu::Sampler sampler;
        {
            wgpu::SamplerDescriptor descriptor = {};
            descriptor.minFilter = wgpu::FilterMode::Linear;
            descriptor.magFilter = wgpu::FilterMode::Linear;
            descriptor.mipmapFilter = wgpu::FilterMode::Linear;
            descriptor.maxAnisotropy = maxAnisotropy;
            sampler = device.CreateSampler(&descriptor);
        }

        // The transform matrix gives us a slanted plane
        // Tweaking happens at: https://jsfiddle.net/t8k7c95o/5/
        // You can get an idea of what the test looks like at the url rendered by webgl
        std::array<float, 16> transform = {-1.7320507764816284,
                                           1.8322050568049563e-16,
                                           -6.176817699518044e-17,
                                           -6.170640314703498e-17,
                                           -2.1211504944260596e-16,
                                           -1.496108889579773,
                                           0.5043753981590271,
                                           0.5038710236549377,
                                           0,
                                           -43.63650894165039,
                                           -43.232173919677734,
                                           -43.18894577026367,
                                           0,
                                           21.693578720092773,
                                           21.789791107177734,
                                           21.86800193786621};
        wgpu::Buffer transformBuffer = utils::CreateBufferFromData(
            device, transform.data(), sizeof(transform), wgpu::BufferUsage::Uniform);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, mBindGroupLayout,
            {{0, sampler}, {1, mTextureView}, {2, transformBuffer, 0, sizeof(transform)}});

        // The plane is scaled on z axis in the transform matrix
        // so uv here is also scaled
        // vertex attribute layout:
        // position : vec4, uv : vec2
        const float vertexData[] = {
            -0.5, 0.5, -0.5, 1, 0, 0,  0.5, 0.5, -0.5, 1, 1, 0, -0.5, 0.5, 0.5, 1, 0, 50,
            -0.5, 0.5, 0.5,  1, 0, 50, 0.5, 0.5, -0.5, 1, 1, 0, 0.5,  0.5, 0.5, 1, 1, 50,
        };
        wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(
            device, vertexData, sizeof(vertexData), wgpu::BufferUsage::Vertex);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&mRenderPass.renderPassInfo);
            pass.SetPipeline(mPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.Draw(6);
            pass.EndPass();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // https://jsfiddle.net/t8k7c95o/5/
        // (x, y) -> (8, [0,15)) full readpixels result on Win10 Nvidia D3D12 GPU
        // maxAnisotropy: 1
        //  0 - 00 00 00
        //  1 - 00 00 ff
        //  2 - 00 00 ff
        //  3 - 00 00 ff
        //  4 - 00 f9 06
        //  5 - 00 f9 06
        //  6 - f2 0d 00
        //  7 - f2 0d 00
        //  8 - ff 00 00
        //  9 - ff 00 00
        // 10 - ff 00 00
        // 11 - ff 00 00
        // 12 - ff 00 00
        // 13 - ff 00 00
        // 14 - ff 00 00
        // 15 - ff 00 00

        // maxAnisotropy: 2
        //  0 - 00 00 00
        //  1 - 00 00 ff
        //  2 - 00 7e 81
        //  3 - 00 7e 81
        //  4 - ff 00 00
        //  5 - ff 00 00
        //  6 - ff 00 00
        //  7 - ff 00 00
        //  8 - ff 00 00
        //  9 - ff 00 00
        // 10 - ff 00 00
        // 11 - ff 00 00
        // 12 - ff 00 00
        // 13 - ff 00 00
        // 14 - ff 00 00
        // 15 - ff 00 00

        // maxAnisotropy: 16
        //  0 - 00 00 00
        //  1 - 00 00 ff
        //  2 - dd 22 00
        //  3 - dd 22 00
        //  4 - ff 00 00
        //  5 - ff 00 00
        //  6 - ff 00 00
        //  7 - ff 00 00
        //  8 - ff 00 00
        //  9 - ff 00 00
        // 10 - ff 00 00
        // 11 - ff 00 00
        // 12 - ff 00 00
        // 13 - ff 00 00
        // 14 - ff 00 00
        // 15 - ff 00 00

        if (maxAnisotropy >= 16) {
            EXPECT_PIXEL_RGBA8_BETWEEN(colors[0], colors[1], mRenderPass.color, 8, 2);
            EXPECT_PIXEL_RGBA8_EQ(colors[0], mRenderPass.color, 8, 6);
        } else if (maxAnisotropy == 2) {
            EXPECT_PIXEL_RGBA8_BETWEEN(colors[1], colors[2], mRenderPass.color, 8, 2);
            EXPECT_PIXEL_RGBA8_EQ(colors[0], mRenderPass.color, 8, 6);
        } else if (maxAnisotropy <= 1) {
            EXPECT_PIXEL_RGBA8_EQ(colors[2], mRenderPass.color, 8, 2);
            EXPECT_PIXEL_RGBA8_BETWEEN(colors[0], colors[1], mRenderPass.color, 8, 6);
        }
    }

    utils::BasicRenderPass mRenderPass;
    wgpu::BindGroupLayout mBindGroupLayout;
    wgpu::RenderPipeline mPipeline;
    wgpu::TextureView mTextureView;
};

TEST_P(SamplerFilterAnisotropicTest, SlantedPlaneMipmap) {
    DAWN_SKIP_TEST_IF(IsOpenGL());
    const uint16_t maxAnisotropyLists[] = {1, 2, 16, 128};
    for (uint16_t t : maxAnisotropyLists) {
        TestFilterAnisotropic(t);
    }
}

DAWN_INSTANTIATE_TEST(SamplerFilterAnisotropicTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
