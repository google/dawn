// Copyright 2018 The NXT Authors
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

#include <array>
#include <cmath>

#include "tests/NXTTest.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/NXTHelpers.h"

constexpr static unsigned int kRTSize = 64;

namespace {
    struct AddressModeTestCase {
        nxt::AddressMode mMode;
        uint8_t mExpected2;
        uint8_t mExpected3;
    };
    AddressModeTestCase addressModes[] = {
        { nxt::AddressMode::Repeat,           0, 255, },
        { nxt::AddressMode::MirroredRepeat, 255,   0, },
        { nxt::AddressMode::ClampToEdge,    255, 255, },
    };
}

class SamplerTest : public NXTTest {
protected:
    void SetUp() override {
        NXTTest::SetUp();
        mRenderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        mRenderPass.color.TransitionUsage(nxt::TextureUsageBit::OutputAttachment);

        mBindGroupLayout = device.CreateBindGroupLayoutBuilder()
            .SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::Sampler, 0, 1)
            .SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::SampledTexture, 1, 1)
            .GetResult();

        auto pipelineLayout = device.CreatePipelineLayoutBuilder()
            .SetBindGroupLayout(0, mBindGroupLayout)
            .GetResult();

        auto vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
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
        auto fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
            #version 450
            layout(set = 0, binding = 0) uniform sampler sampler0;
            layout(set = 0, binding = 1) uniform texture2D texture0;
            layout(location = 0) out vec4 fragColor;

            void main() {
                fragColor = texture(sampler2D(texture0, sampler0), gl_FragCoord.xy / 2.0);
            }
        )");

        mPipeline = device.CreateRenderPipelineBuilder()
            .SetColorAttachmentFormat(0, mRenderPass.colorFormat)
            .SetLayout(pipelineLayout)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .GetResult();

        auto texture = device.CreateTextureBuilder()
            .SetDimension(nxt::TextureDimension::e2D)
            .SetExtent(2, 2, 1)
            .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
            .SetMipLevels(1)
            .SetAllowedUsage(nxt::TextureUsageBit::TransferDst | nxt::TextureUsageBit::Sampled)
            .GetResult();

        // Create a 2x2 checkerboard texture, with black in the top left and bottom right corners.
        const uint32_t rowPixels = kTextureRowPitchAlignment / sizeof(RGBA8);
        RGBA8 data[rowPixels * 2];
        RGBA8 black(0, 0, 0, 255);
        RGBA8 white(255, 255, 255, 255);
        data[0] = data[rowPixels + 1] = black;
        data[1] = data[rowPixels] = white;

        nxt::Buffer stagingBuffer = utils::CreateFrozenBufferFromData(device, data, sizeof(data), nxt::BufferUsageBit::TransferSrc);
        nxt::CommandBuffer copy = device.CreateCommandBufferBuilder()
            .TransitionTextureUsage(texture, nxt::TextureUsageBit::TransferDst)
            .CopyBufferToTexture(stagingBuffer, 0, 256, texture, 0, 0, 0, 2, 2, 1, 0)
            .GetResult();

        queue.Submit(1, &copy);
        texture.FreezeUsage(nxt::TextureUsageBit::Sampled);
        mTextureView = texture.CreateTextureViewBuilder().GetResult();
    }

    void TestAddressModes(AddressModeTestCase u, AddressModeTestCase v, AddressModeTestCase w) {

        nxt::Sampler sampler = device.CreateSamplerBuilder()
            .SetAddressMode(u.mMode, v.mMode, w.mMode)
            .GetResult();

        auto bindGroup = device.CreateBindGroupBuilder()
            .SetLayout(mBindGroupLayout)
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetSamplers(0, 1, &sampler)
            .SetTextureViews(1, 1, &mTextureView)
            .GetResult();

        nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
            .BeginRenderPass(mRenderPass.renderPassInfo)
            .SetRenderPipeline(mPipeline)
            .SetBindGroup(0, bindGroup)
            .DrawArrays(6, 1, 0, 0)
            .EndRenderPass()
            .GetResult();

        queue.Submit(1, &commands);

        RGBA8 expectedU2(u.mExpected2, u.mExpected2, u.mExpected2, 255);
        RGBA8 expectedU3(u.mExpected3, u.mExpected3, u.mExpected3, 255);
        RGBA8 expectedV2(v.mExpected2, v.mExpected2, v.mExpected2, 255);
        RGBA8 expectedV3(v.mExpected3, v.mExpected3, v.mExpected3, 255);
        RGBA8 black(0, 0, 0, 255);
        RGBA8 white(255, 255, 255, 255);
        EXPECT_PIXEL_RGBA8_EQ(black, mRenderPass.color, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(white, mRenderPass.color, 0, 1);
        EXPECT_PIXEL_RGBA8_EQ(white, mRenderPass.color, 1, 0);
        EXPECT_PIXEL_RGBA8_EQ(black, mRenderPass.color, 1, 1);
        EXPECT_PIXEL_RGBA8_EQ(expectedU2, mRenderPass.color, 2, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedU3, mRenderPass.color, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedV2, mRenderPass.color, 0, 2);
        EXPECT_PIXEL_RGBA8_EQ(expectedV3, mRenderPass.color, 0, 3);
        // TODO: add tests for W address mode, once NXT supports 3D textures
    }

    utils::BasicRenderPass mRenderPass;
    nxt::BindGroupLayout mBindGroupLayout;
    nxt::RenderPipeline mPipeline;
    nxt::TextureView mTextureView;
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

NXT_INSTANTIATE_TEST(SamplerTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
