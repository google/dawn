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

#include "tests/DawnTest.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/DawnHelpers.h"

constexpr static unsigned int kRTSize = 64;

class TextureViewTest : public DawnTest {
protected:
    void SetUp() override {
        DawnTest::SetUp();

        mRenderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        mBindGroupLayout = utils::MakeBindGroupLayout(
            device, {
                        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler},
                        {1, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture},
            });

        dawn::FilterMode kFilterMode = dawn::FilterMode::Nearest;
        dawn::AddressMode kAddressMode = dawn::AddressMode::ClampToEdge;

        dawn::SamplerDescriptor samplerDescriptor;
        samplerDescriptor.minFilter = kFilterMode;
        samplerDescriptor.magFilter = kFilterMode;
        samplerDescriptor.mipmapFilter = kFilterMode;
        samplerDescriptor.addressModeU = kAddressMode;
        samplerDescriptor.addressModeV = kAddressMode;
        samplerDescriptor.addressModeW = kAddressMode;
        mSampler = device.CreateSampler(&samplerDescriptor);

        mPipelineLayout = utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);

        mVSModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450
            void main() {
                const vec2 pos[6] = vec2[6](vec2(-2.f, -2.f),
                                            vec2(-2.f,  2.f),
                                            vec2( 2.f, -2.f),
                                            vec2(-2.f,  2.f),
                                            vec2( 2.f, -2.f),
                                            vec2( 2.f,  2.f));
                gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
            }
        )");
    }

    void initTexture(uint32_t layerCount) {
        ASSERT(layerCount > 0);

        dawn::TextureDescriptor descriptor;
        descriptor.dimension = dawn::TextureDimension::e2D;
        descriptor.size.width = 2;
        descriptor.size.height = 2;
        descriptor.size.depth = 1;
        descriptor.arrayLayer = layerCount;
        descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
        descriptor.mipLevel = 1;
        descriptor.usage = dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::Sampled;
        mTexture = device.CreateTexture(&descriptor);

        // Create a 2x2 checkerboard texture, with black in the top left and bottom right corners.
        constexpr uint32_t kRowPixels = kTextureRowPitchAlignment / sizeof(RGBA8);

        dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
        for (uint32_t layer = 0; layer < layerCount; ++layer) {
            RGBA8 data[kRowPixels * 2];
            int pixelValue = static_cast<int>(layer);
            data[0] = data[kRowPixels + 1] = RGBA8(0, 0, 0, pixelValue);
            data[1] = data[kRowPixels] = RGBA8(pixelValue, pixelValue, pixelValue, pixelValue);;
            dawn::Buffer stagingBuffer = utils::CreateBufferFromData(
                device, data, sizeof(data), dawn::BufferUsageBit::TransferSrc);
            builder.CopyBufferToTexture(
                stagingBuffer, 0, 256, mTexture, 0, 0, 0, 2, 2, 1, 0, layer);
        }
        dawn::CommandBuffer copy = builder.GetResult();
        queue.Submit(1, &copy);
    }

    void Test(const dawn::TextureView &textureView, const char* fragmentShader, int expected) {
        dawn::BindGroup bindGroup = device.CreateBindGroupBuilder()
            .SetLayout(mBindGroupLayout)
            .SetSamplers(0, 1, &mSampler)
            .SetTextureViews(1, 1, &textureView)
            .GetResult();

        dawn::ShaderModule fsModule =
            utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, fragmentShader);

        dawn::RenderPipeline pipeline = device.CreateRenderPipelineBuilder()
            .SetColorAttachmentFormat(0, mRenderPass.colorFormat)
            .SetLayout(mPipelineLayout)
            .SetStage(dawn::ShaderStage::Vertex, mVSModule, "main")
            .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
            .GetResult();

        dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
        {
            dawn::RenderPassEncoder pass = builder.BeginRenderPass(mRenderPass.renderPassInfo);
            pass.SetRenderPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DrawArrays(6, 1, 0, 0);
            pass.EndPass();
        }

        dawn::CommandBuffer commands = builder.GetResult();
        queue.Submit(1, &commands);

        RGBA8 expectedPixel0(0, 0, 0, expected);
        RGBA8 expectedPixel1(expected, expected, expected, expected);
        EXPECT_PIXEL_RGBA8_EQ(expectedPixel0, mRenderPass.color, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedPixel1, mRenderPass.color, 0, 1);
        EXPECT_PIXEL_RGBA8_EQ(expectedPixel1, mRenderPass.color, 1, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedPixel0, mRenderPass.color, 1, 1);
        // TODO(jiawei.shao@intel.com): add tests for 3D textures once Dawn supports 3D textures
    }

    dawn::BindGroupLayout mBindGroupLayout;
    dawn::PipelineLayout mPipelineLayout;
    dawn::Sampler mSampler;
    dawn::Texture mTexture;
    dawn::ShaderModule mVSModule;
    utils::BasicRenderPass mRenderPass;
};

// Test drawing a rect with a checkerboard 2D array texture.
TEST_P(TextureViewTest, Default2DArrayTexture) {
    constexpr uint32_t kLayers = 3;
    initTexture(kLayers);

    dawn::TextureView textureView = mTexture.CreateDefaultTextureView();

    const char* fragmentShader = R"(
            #version 450
            layout(set = 0, binding = 0) uniform sampler sampler0;
            layout(set = 0, binding = 1) uniform texture2DArray texture0;
            layout(location = 0) out vec4 fragColor;

            void main() {
                fragColor =
                    texture(sampler2DArray(texture0, sampler0), vec3(gl_FragCoord.xy / 2.0, 0)) +
                    texture(sampler2DArray(texture0, sampler0), vec3(gl_FragCoord.xy / 2.0, 1)) +
                    texture(sampler2DArray(texture0, sampler0), vec3(gl_FragCoord.xy / 2.0, 2));
            }
        )";
    Test(textureView, fragmentShader, kLayers);
}

DAWN_INSTANTIATE_TEST(TextureViewTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
