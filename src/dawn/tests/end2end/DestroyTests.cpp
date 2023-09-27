// Copyright 2019 The Dawn Authors
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

#include <string>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using ::testing::HasSubstr;

constexpr uint32_t kRTSize = 4;

class DestroyTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
              @vertex
              fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
                  return pos;
              })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
              @fragment fn main() -> @location(0) vec4f {
                  return vec4f(0.0, 1.0, 0.0, 1.0);
              })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = renderPass.colorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// The bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.BeginRenderPass(&renderPass.renderPassInfo).End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;

    wgpu::CommandBuffer CreateTriangleCommandBuffer() {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.Draw(3);
            pass.End();
        }
        wgpu::CommandBuffer commands = encoder.Finish();
        return commands;
    }
};

// Destroy before submit will result in error, and nothing drawn
TEST_P(DestroyTest, BufferDestroyBeforeSubmit) {
    utils::RGBA8 notFilled(0, 0, 0, 0);

    wgpu::CommandBuffer commands = CreateTriangleCommandBuffer();
    vertexBuffer.Destroy();
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 1, 3);
}

// Destroy after submit will draw successfully
TEST_P(DestroyTest, BufferDestroyAfterSubmit) {
    utils::RGBA8 filled(0, 255, 0, 255);

    wgpu::CommandBuffer commands = CreateTriangleCommandBuffer();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
    vertexBuffer.Destroy();
}

// First submit succeeds, draws triangle, second submit fails
// after destroy is called on the buffer, pixel does not change
TEST_P(DestroyTest, BufferSubmitDestroySubmit) {
    utils::RGBA8 filled(0, 255, 0, 255);

    wgpu::CommandBuffer commands = CreateTriangleCommandBuffer();
    queue.Submit(1, &commands);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);

    vertexBuffer.Destroy();

    // Submit fails because vertex buffer was destroyed
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

    // Pixel stays the same
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
}

// Destroy texture before submit should fail submit
TEST_P(DestroyTest, TextureDestroyBeforeSubmit) {
    wgpu::CommandBuffer commands = CreateTriangleCommandBuffer();
    renderPass.color.Destroy();
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Destroy after submit will draw successfully
TEST_P(DestroyTest, TextureDestroyAfterSubmit) {
    utils::RGBA8 filled(0, 255, 0, 255);

    wgpu::CommandBuffer commands = CreateTriangleCommandBuffer();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
    renderPass.color.Destroy();
}

// First submit succeeds, draws triangle, second submit fails
// after destroy is called on the texture
TEST_P(DestroyTest, TextureSubmitDestroySubmit) {
    utils::RGBA8 filled(0, 255, 0, 255);

    wgpu::CommandBuffer commands = CreateTriangleCommandBuffer();
    queue.Submit(1, &commands);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);

    renderPass.color.Destroy();

    // Submit fails because texture was destroyed
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Attempting to set an object label after it has been destroyed should not cause an error.
TEST_P(DestroyTest, DestroyObjectThenSetLabel) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    std::string label = "test";
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    buffer.Destroy();
    buffer.SetLabel(label.c_str());
}

// Attempting to set a device label after it has been destroyed should not cause an error.
TEST_P(DestroyTest, DestroyDeviceThenSetLabel) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    std::string label = "test";
    device.Destroy();
    device.SetLabel(label.c_str());
}

// Device destroy before buffer submit will result in error.
TEST_P(DestroyTest, DestroyDeviceBeforeSubmit) {
    // TODO(crbug.com/dawn/628) Add more comprehensive tests with destroy and backends.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    wgpu::CommandBuffer commands = CreateTriangleCommandBuffer();

    DestroyDevice();
    ASSERT_DEVICE_ERROR_MSG(queue.Submit(1, &commands), HasSubstr("[Device] is lost."));
}

// Regression test for crbug.com/1276928 where a lingering BGL reference in Vulkan with at least one
// BG instance could cause bad memory reads because members in the BGL whose destuctors expected a
// live device were not released until after the device was destroyed.
TEST_P(DestroyTest, DestroyDeviceLingeringBGL) {
    // Create and hold the layout reference so that its destructor gets called after the device has
    // been destroyed via device.Destroy().
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});
    utils::MakeBindGroup(device, layout, {{0, device.CreateSampler()}});

    DestroyDevice();
}

// Regression test for crbug.com/1327865 where the device set the queue to null
// inside Destroy() such that it could no longer return it to a call to GetQueue().
TEST_P(DestroyTest, GetQueueAfterDeviceDestroy) {
    DestroyDevice();

    wgpu::Queue queue = device.GetQueue();
    ASSERT_DEVICE_ERROR(queue.OnSubmittedWorkDone(
        0u,
        [](WGPUQueueWorkDoneStatus status, void* userdata) {
            // TODO(crbug.com/dawn/2021): Wire and native differ slightly for now. Unify once we
            // decide on the correct result. In theory maybe we want to pretend that things succeed
            // when the device is lost.
            DestroyTest* test = static_cast<DestroyTest*>(userdata);
            if (test->UsesWire()) {
                EXPECT_EQ(status, WGPUQueueWorkDoneStatus_Success);
            } else {
                EXPECT_EQ(status, WGPUQueueWorkDoneStatus_DeviceLost);
            }
        },
        this));
}

DAWN_INSTANTIATE_TEST(DestroyTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
