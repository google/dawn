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

// This file contains test for deprecated parts of Dawn's API while following WebGPU's evolution.
// It contains test for the "old" behavior that will be deleted once users are migrated, tests that
// a deprecation warning is emitted when the "old" behavior is used, and tests that an error is
// emitted when both the old and the new behavior are used (when applicable).

#include "tests/DawnTest.h"

#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class DeprecationTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Skip when validation is off because warnings might be emitted during validation calls
        DAWN_SKIP_TEST_IF(IsDawnValidationSkipped());
    }

    void TearDown() override {
        if (!UsesWire()) {
            EXPECT_EQ(mLastWarningCount,
                      dawn_native::GetDeprecationWarningCountForTesting(device.Get()));
        }
        DawnTest::TearDown();
    }

    size_t mLastWarningCount = 0;
};

#define EXPECT_DEPRECATION_WARNING(statement)                                    \
    do {                                                                         \
        if (UsesWire()) {                                                        \
            statement;                                                           \
        } else {                                                                 \
            size_t warningsBefore =                                              \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            statement;                                                           \
            size_t warningsAfter =                                               \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            EXPECT_EQ(mLastWarningCount, warningsBefore);                        \
            EXPECT_EQ(warningsAfter, warningsBefore + 1);                        \
            mLastWarningCount = warningsAfter;                                   \
        }                                                                        \
    } while (0)

// Test that using BGLEntry.multisampled = true emits a deprecation warning.
TEST_P(DeprecationTests, BGLEntryMultisampledDeprecated) {
    wgpu::BindGroupLayoutEntry entry{};
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.type = wgpu::BindingType::SampledTexture;
    entry.multisampled = true;
    entry.binding = 0;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroupLayout(&desc));
}

// Test that using BGLEntry.multisampled = true with MultisampledTexture is an error.
TEST_P(DeprecationTests, BGLEntryMultisampledBooleanAndTypeIsAnError) {
    wgpu::BindGroupLayoutEntry entry{};
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.type = wgpu::BindingType::MultisampledTexture;
    entry.multisampled = true;
    entry.binding = 0;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Test that a using BGLEntry.multisampled produces the correct state tracking.
TEST_P(DeprecationTests, BGLEntryMultisampledBooleanTracking) {
    // Create a BGL with the deprecated multisampled boolean
    wgpu::BindGroupLayoutEntry entry{};
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.type = wgpu::BindingType::SampledTexture;
    entry.multisampled = true;
    entry.binding = 0;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;
    wgpu::BindGroupLayout bgl;
    EXPECT_DEPRECATION_WARNING(bgl = device.CreateBindGroupLayout(&desc));

    // Create both a multisampled and non-multisampled texture.
    wgpu::TextureDescriptor textureDesc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::Sampled;
    textureDesc.size = {1, 1, 1};
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.sampleCount = 1;
    wgpu::Texture texture1Sample = device.CreateTexture(&textureDesc);

    textureDesc.sampleCount = 4;
    wgpu::Texture texture4Sample = device.CreateTexture(&textureDesc);

    // Creating a bindgroup with that layout is only valid with multisampled = true
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, texture1Sample.CreateView()}}));
    utils::MakeBindGroup(device, bgl, {{0, texture4Sample.CreateView()}});
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());

class BufferCopyViewDeprecationTests : public DeprecationTests {
  protected:
    wgpu::TextureCopyView MakeTextureCopyView() {
        wgpu::TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.size = {1, 1, 2};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::TextureCopyView copy;
        copy.texture = device.CreateTexture(&desc);
        copy.origin = {0, 0, 1};
        return copy;
    }

    wgpu::Extent3D copySize = {1, 1, 1};
};

constexpr uint32_t kRTSize = 400;

class SetIndexBufferDeprecationTests : public DeprecationTests {
  protected:
    void SetUp() override {
        DeprecationTests::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    }

    utils::BasicRenderPass renderPass;

    wgpu::RenderPipeline MakeTestPipeline(wgpu::IndexFormat format) {
        wgpu::ShaderModule vsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
                #version 450
                layout(location = 0) in vec4 pos;
                void main() {
                    gl_Position = pos;
                })");

        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
                })");

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleStrip;
        descriptor.cVertexState.indexFormat = format;
        descriptor.cVertexState.vertexBufferCount = 1;
        descriptor.cVertexState.cVertexBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cVertexState.cVertexBuffers[0].attributeCount = 1;
        descriptor.cVertexState.cAttributes[0].format = wgpu::VertexFormat::Float4;
        descriptor.cColorStates[0].format = renderPass.colorFormat;

        return device.CreateRenderPipeline(&descriptor);
    }
};

// Test that the Uint32 index format is correctly interpreted
TEST_P(SetIndexBufferDeprecationTests, Uint32) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f,  // Note Vertices[0] = Vertices[1]
         -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    // If this is interpreted as Uint16, then it would be 0, 1, 0, ... and would draw nothing.
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {1, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        EXPECT_DEPRECATION_WARNING(pass.SetIndexBuffer(indexBuffer));
        pass.DrawIndexed(3);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 100, 300);
}

// Test that the Uint16 index format is correctly interpreted
TEST_P(SetIndexBufferDeprecationTests, Uint16) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint16);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    // If this is interpreted as uint32, it will have index 1 and 2 be both 0 and render nothing
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index, {1, 2, 0, 0, 0, 0});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        EXPECT_DEPRECATION_WARNING(pass.SetIndexBuffer(indexBuffer));
        pass.DrawIndexed(3);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 100, 300);
}

DAWN_INSTANTIATE_TEST(SetIndexBufferDeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
