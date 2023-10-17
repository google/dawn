// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cstring>
#include <memory>
#include <string>

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "gmock/gmock.h"

namespace dawn {
namespace {

using testing::_;
using testing::Exactly;
using testing::MockCallback;

class MockQueueWorkDoneCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUQueueWorkDoneStatus status, void* userdata));
};

static std::unique_ptr<MockQueueWorkDoneCallback> mockQueueWorkDoneCallback;
static void ToMockQueueWorkDone(WGPUQueueWorkDoneStatus status, void* userdata) {
    mockQueueWorkDoneCallback->Call(status, userdata);
}

static const int fakeUserData = 0;

class DeviceLostTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        mockQueueWorkDoneCallback = std::make_unique<MockQueueWorkDoneCallback>();
    }

    void TearDown() override {
        mockQueueWorkDoneCallback = nullptr;
        DawnTest::TearDown();
    }

    static void MapFailCallback(WGPUBufferMapAsyncStatus status, void* userdata) {
        EXPECT_EQ(WGPUBufferMapAsyncStatus_DeviceLost, status);
        EXPECT_EQ(&fakeUserData, userdata);
    }

    void MapAsyncAndWait(const wgpu::Buffer& buffer,
                         wgpu::MapMode mode,
                         size_t offset,
                         size_t size) {
        bool done = false;
        buffer.MapAsync(
            mode, offset, size,
            [](WGPUBufferMapAsyncStatus status, void* userdata) {
                ASSERT_EQ(WGPUBufferMapAsyncStatus_Success, status);
                *static_cast<bool*>(userdata) = true;
            },
            &done);

        while (!done) {
            WaitABit();
        }
    }
};

// Test that DeviceLostCallback is invoked when LostForTestimg is called
TEST_P(DeviceLostTest, DeviceLostCallbackIsCalled) {
    LoseDeviceForTesting();
}

// Test that submit fails when device is lost
TEST_P(DeviceLostTest, SubmitFails) {
    wgpu::CommandBuffer commands;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    commands = encoder.Finish();

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(queue.Submit(0, &commands));
}

// Test that CreateBindGroupLayout fails when device is lost
TEST_P(DeviceLostTest, CreateBindGroupLayoutFails) {
    LoseDeviceForTesting();

    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::None;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));
}

// Test that GetBindGroupLayout fails when device is lost
TEST_P(DeviceLostTest, GetBindGroupLayoutFails) {
    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        struct UniformBuffer {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> ubo : UniformBuffer;
        @compute @workgroup_size(1) fn main() {
        })");

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.compute.module = csModule;
    descriptor.compute.entryPoint = "main";

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(pipeline.GetBindGroupLayout(0).Get());
}

// Test that CreateBindGroup fails when device is lost
TEST_P(DeviceLostTest, CreateBindGroupFails) {
    LoseDeviceForTesting();

    wgpu::BindGroupEntry entry;
    entry.binding = 0;
    entry.sampler = nullptr;
    entry.textureView = nullptr;
    entry.buffer = nullptr;
    entry.offset = 0;
    entry.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
}

// Test that CreatePipelineLayout fails when device is lost
TEST_P(DeviceLostTest, CreatePipelineLayoutFails) {
    LoseDeviceForTesting();

    wgpu::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = 0;
    descriptor.bindGroupLayouts = nullptr;
    ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&descriptor));
}

// Tests that CreateRenderBundleEncoder fails when device is lost
TEST_P(DeviceLostTest, CreateRenderBundleEncoderFails) {
    LoseDeviceForTesting();

    wgpu::RenderBundleEncoderDescriptor descriptor;
    descriptor.colorFormatCount = 0;
    descriptor.colorFormats = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&descriptor));
}

// Tests that CreateComputePipeline fails when device is lost
TEST_P(DeviceLostTest, CreateComputePipelineFails) {
    LoseDeviceForTesting();

    wgpu::ComputePipelineDescriptor descriptor = {};
    descriptor.layout = nullptr;
    descriptor.compute.module = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&descriptor));
}

// Tests that CreateRenderPipeline fails when device is lost
TEST_P(DeviceLostTest, CreateRenderPipelineFails) {
    LoseDeviceForTesting();

    utils::ComboRenderPipelineDescriptor descriptor;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Tests that CreateSampler fails when device is lost
TEST_P(DeviceLostTest, CreateSamplerFails) {
    LoseDeviceForTesting();

    ASSERT_DEVICE_ERROR(device.CreateSampler());
}

// Tests that CreateShaderModule fails when device is lost
TEST_P(DeviceLostTest, CreateShaderModuleFails) {
    LoseDeviceForTesting();

    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @fragment
        fn main(@location(0) color : vec4f) -> @location(0) vec4f {
            return color;
        })"));
}

// Note that no device lost tests are done for swapchain because it is awkward to create a
// wgpu::Surface in this file. SwapChainValidationTests.CreateSwapChainFailsAfterDevLost covers
// this validation.

// Tests that CreateTexture fails when device is lost
TEST_P(DeviceLostTest, CreateTextureFails) {
    LoseDeviceForTesting();

    wgpu::TextureDescriptor descriptor;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.mipLevelCount = 1;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

// Test that CreateBuffer fails when device is lost
TEST_P(DeviceLostTest, CreateBufferFails) {
    LoseDeviceForTesting();

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&bufferDescriptor));
}

// Test that buffer.MapAsync for writing fails after device is lost
TEST_P(DeviceLostTest, BufferMapAsyncFailsForWriting) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 4;
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(buffer.MapAsync(wgpu::MapMode::Write, 0, 4, MapFailCallback,
                                        const_cast<int*>(&fakeUserData)));
}

// Test that BufferMapAsync for writing calls back with device lost status when device lost after
// mapping
TEST_P(DeviceLostTest, BufferMapAsyncBeforeLossFailsForWriting) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 4;
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    buffer.MapAsync(wgpu::MapMode::Write, 0, 4, MapFailCallback, const_cast<int*>(&fakeUserData));

    LoseDeviceForTesting();
}

// Test that buffer.Unmap fails after device is lost
TEST_P(DeviceLostTest, BufferUnmapFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    bufferDescriptor.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(buffer.Unmap());
}

// Test that mappedAtCreation fails after device is lost
TEST_P(DeviceLostTest, CreateBufferMappedAtCreationFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    bufferDescriptor.mappedAtCreation = true;

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&bufferDescriptor));
}

// Test that BufferMapAsync for reading fails after device is lost
TEST_P(DeviceLostTest, BufferMapAsyncFailsForReading) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 4;
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(buffer.MapAsync(wgpu::MapMode::Read, 0, 4, MapFailCallback,
                                        const_cast<int*>(&fakeUserData)));
}

// Test that BufferMapAsync for reading calls back with device lost status when device lost after
// mapping
TEST_P(DeviceLostTest, BufferMapAsyncBeforeLossFailsForReading) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    buffer.MapAsync(wgpu::MapMode::Read, 0, 4, MapFailCallback, const_cast<int*>(&fakeUserData));

    LoseDeviceForTesting();
}

// Test that WriteBuffer fails after device is lost
TEST_P(DeviceLostTest, WriteBufferFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    LoseDeviceForTesting();
    float data = 12.0f;
    ASSERT_DEVICE_ERROR(queue.WriteBuffer(buffer, 0, &data, sizeof(data)));
}

// Test it's possible to GetMappedRange on a buffer created mapped after device loss
TEST_P(DeviceLostTest, GetMappedRange_CreateBufferMappedAtCreationAfterLoss) {
    LoseDeviceForTesting();

    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc;
    desc.mappedAtCreation = true;
    ASSERT_DEVICE_ERROR(wgpu::Buffer buffer = device.CreateBuffer(&desc));

    ASSERT_NE(buffer.GetMappedRange(), nullptr);
}

// Test that device loss doesn't change the result of GetMappedRange, mappedAtCreation version.
TEST_P(DeviceLostTest, GetMappedRange_CreateBufferMappedAtCreationBeforeLoss) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc;
    desc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    void* rangeBeforeLoss = buffer.GetMappedRange();
    LoseDeviceForTesting();

    ASSERT_NE(buffer.GetMappedRange(), nullptr);
    ASSERT_EQ(buffer.GetMappedRange(), rangeBeforeLoss);
}

// Test that device loss doesn't change the result of GetMappedRange, mapping for reading version.
TEST_P(DeviceLostTest, GetMappedRange_MapAsyncReading) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, 4);
    queue.Submit(0, nullptr);

    const void* rangeBeforeLoss = buffer.GetConstMappedRange();
    LoseDeviceForTesting();

    ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
    ASSERT_EQ(buffer.GetConstMappedRange(), rangeBeforeLoss);
}

// Test that device loss doesn't change the result of GetMappedRange, mapping for writing version.
TEST_P(DeviceLostTest, GetMappedRange_MapAsyncWriting) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
    queue.Submit(0, nullptr);

    const void* rangeBeforeLoss = buffer.GetConstMappedRange();
    LoseDeviceForTesting();

    ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
    ASSERT_EQ(buffer.GetConstMappedRange(), rangeBeforeLoss);
}

// TODO(dawn:929): mapasync read + resolve + loss getmappedrange != nullptr.
// TODO(dawn:929): mapasync write + resolve + loss getmappedrange != nullptr.

// Test that Command Encoder Finish fails when device lost
TEST_P(DeviceLostTest, CommandEncoderFinishFails) {
    wgpu::CommandBuffer commands;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test that QueueOnSubmittedWorkDone fails after device is lost.
TEST_P(DeviceLostTest, QueueOnSubmittedWorkDoneFails) {
    LoseDeviceForTesting();

    // callback should have device lost status
    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_DeviceLost, nullptr))
        .Times(1);
    ASSERT_DEVICE_ERROR(queue.OnSubmittedWorkDone(ToMockQueueWorkDone, nullptr));
}

// Test that QueueOnSubmittedWorkDone when the device is lost after calling OnSubmittedWorkDone
TEST_P(DeviceLostTest, QueueOnSubmittedWorkDoneBeforeLossFails) {
    // callback should have device lost status
    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_DeviceLost, nullptr))
        .Times(1);
    queue.OnSubmittedWorkDone(ToMockQueueWorkDone, nullptr);

    LoseDeviceForTesting();
}

// Test that LostForTesting can only be called on one time
TEST_P(DeviceLostTest, LoseDeviceForTestingOnce) {
    // First LoseDeviceForTesting call should occur normally. The callback is already set in SetUp.
    LoseDeviceForTesting();

    // Second LoseDeviceForTesting call should result in no callbacks. Note we also reset the
    // callback first since by default the device clears the callback after the device is lost.
    device.SetDeviceLostCallback(mDeviceLostCallback.Callback(),
                                 mDeviceLostCallback.MakeUserdata(device.Get()));
    EXPECT_CALL(mDeviceLostCallback, Call(WGPUDeviceLostReason_Undefined, testing::_, device.Get()))
        .Times(0);
    device.ForceLoss(wgpu::DeviceLostReason::Undefined, "Device lost for testing");
    FlushWire();
    testing::Mock::VerifyAndClearExpectations(&mDeviceLostCallback);
}

TEST_P(DeviceLostTest, DeviceLostDoesntCallUncapturedError) {
    // Since the device has a default error callback set that fails if it is called, we just need
    // to lose the device and verify no failures.
    LoseDeviceForTesting();
}

// Test that WGPUCreatePipelineAsyncStatus_Success is returned when device is lost
// before the callback of Create*PipelineAsync() is called.
TEST_P(DeviceLostTest, DeviceLostBeforeCreatePipelineAsyncCallback) {
    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        })");

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.compute.module = csModule;
    descriptor.compute.entryPoint = "main";

    auto callback = [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
                       const char* message, void* userdata) {
        EXPECT_EQ(WGPUCreatePipelineAsyncStatus_Success, status);
        EXPECT_NE(returnPipeline, nullptr);
        wgpu::ComputePipeline::Acquire(returnPipeline);
    };

    device.CreateComputePipelineAsync(&descriptor, callback, nullptr);
    LoseDeviceForTesting();
}

// This is a regression test for crbug.com/1212385 where Dawn didn't clean up all
// references to bind group layouts such that the cache was non-empty at the end
// of shut down.
TEST_P(DeviceLostTest, FreeBindGroupAfterDeviceLossWithPendingCommands) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, sizeof(float)}});

    // Advance the pending command serial. We only a need a couple of these to repro the bug,
    // but include extra so this does not become a change-detecting test if the specific serial
    // value is sensitive.
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);
    queue.Submit(0, nullptr);

    LoseDeviceForTesting();

    // Releasing the bing group places the bind group layout into a queue in the Vulkan backend
    // for recycling of descriptor sets. So, after these release calls there is still one last
    // reference to the BGL which wouldn't be freed until the pending serial passes.
    // Since the device is lost, destruction will clean up immediately without waiting for the
    // serial. The implementation needs to be sure to clear these BGL references. At the end of
    // Device shut down, we DAWN_ASSERT that the BGL cache is empty.
    bgl = nullptr;
    bg = nullptr;
}

// This is a regression test for crbug.com/1365011 where ending a render pass with an indirect draw
// in it after the device is lost would cause render commands to be leaked.
TEST_P(DeviceLostTest, DeviceLostInRenderPassWithDrawIndirect) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 4u, 4u);
    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main(@builtin(vertex_index) i : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, -1.0),
                vec2f(3.0, -1.0),
                vec2f(-1.0, 3.0));
            return vec4f(pos[i], 0.0, 1.0);
        }
    )");
    desc.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        }
    )");
    desc.cTargets[0].format = renderPass.colorFormat;
    wgpu::Buffer indirectBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0});
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.DrawIndirect(indirectBuffer, 0);
    LoseDeviceForTesting();
    pass.End();
}

// Attempting to set an object label after device loss should not cause an error.
TEST_P(DeviceLostTest, SetLabelAfterDeviceLoss) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    std::string label = "test";
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    LoseDeviceForTesting();
    buffer.SetLabel(label.c_str());
}

DAWN_INSTANTIATE_TEST(DeviceLostTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
