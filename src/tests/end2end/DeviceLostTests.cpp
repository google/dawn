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

#include "tests/DawnTest.h"

#include <gmock/gmock.h>
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

#include <cstring>

using namespace testing;

class MockDeviceLostCallback {
  public:
    MOCK_METHOD2(Call, void(const char* message, void* userdata));
};

static std::unique_ptr<MockDeviceLostCallback> mockDeviceLostCallback;
static void ToMockDeviceLostCallback(const char* message, void* userdata) {
    mockDeviceLostCallback->Call(message, userdata);
    DawnTestBase* self = static_cast<DawnTestBase*>(userdata);
    self->StartExpectDeviceError();
}

static const int fakeUserData = 0;

class DeviceLostTest : public DawnTest {
  protected:
    void TestSetUp() override {
        DAWN_SKIP_TEST_IF(UsesWire());
        DawnTest::TestSetUp();
        mockDeviceLostCallback = std::make_unique<MockDeviceLostCallback>();
    }

    void TearDown() override {
        DawnTest::TearDown();
        mockDeviceLostCallback = nullptr;
    }

    void SetCallbackAndLoseForTesting() {
        device.SetDeviceLostCallback(ToMockDeviceLostCallback, this);
        EXPECT_CALL(*mockDeviceLostCallback, Call(_, this)).Times(1);
        device.LoseForTesting();
    }

    static void CheckMapWriteFail(WGPUBufferMapAsyncStatus status,
                                  void* data,
                                  uint64_t datalength,
                                  void* userdata) {
        EXPECT_EQ(WGPUBufferMapAsyncStatus_DeviceLost, status);
        EXPECT_EQ(nullptr, data);
        EXPECT_EQ(0u, datalength);
        EXPECT_EQ(&fakeUserData, userdata);
    }

    static void CheckMapReadFail(WGPUBufferMapAsyncStatus status,
                                 const void* data,
                                 uint64_t datalength,
                                 void* userdata) {
        EXPECT_EQ(WGPUBufferMapAsyncStatus_DeviceLost, status);
        EXPECT_EQ(nullptr, data);
        EXPECT_EQ(0u, datalength);
        EXPECT_EQ(&fakeUserData, userdata);
    }
};

// Test that DeviceLostCallback is invoked when LostForTestimg is called
TEST_P(DeviceLostTest, DeviceLostCallbackIsCalled) {
    SetCallbackAndLoseForTesting();
}

// Test that submit fails when device is lost
TEST_P(DeviceLostTest, SubmitFails) {
    wgpu::CommandBuffer commands;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    commands = encoder.Finish();

    SetCallbackAndLoseForTesting();
    ASSERT_DEVICE_ERROR(queue.Submit(0, &commands));
}

// Test that CreateBindGroupLayout fails when device is lost
TEST_P(DeviceLostTest, CreateBindGroupLayoutFails) {
    SetCallbackAndLoseForTesting();

    wgpu::BindGroupLayoutBinding binding = {0, wgpu::ShaderStage::None,
                                            wgpu::BindingType::UniformBuffer};
    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.bindingCount = 1;
    descriptor.bindings = &binding;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));
}

// Test that GetBindGroupLayout fails when device is lost
TEST_P(DeviceLostTest, GetBindGroupLayoutFails) {
    wgpu::ShaderModule csModule =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, R"(
    #version 450
    layout(set = 0, binding = 0) uniform UniformBuffer {
        vec4 pos;
    };
    void main() {
    })");

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.computeStage.module = csModule;
    descriptor.computeStage.entryPoint = "main";

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);

    SetCallbackAndLoseForTesting();
    ASSERT_DEVICE_ERROR(pipeline.GetBindGroupLayout(0).Get());
}

// Test that CreateBindGroup fails when device is lost
TEST_P(DeviceLostTest, CreateBindGroupFails) {
    SetCallbackAndLoseForTesting();

    wgpu::BindGroupBinding binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.bindingCount = 1;
    descriptor.bindings = &binding;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
}

// Test that CreatePipelineLayout fails when device is lost
TEST_P(DeviceLostTest, CreatePipelineLayoutFails) {
    SetCallbackAndLoseForTesting();

    wgpu::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = 0;
    descriptor.bindGroupLayouts = nullptr;
    ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&descriptor));
}

// Tests that CreateRenderBundleEncoder fails when device is lost
TEST_P(DeviceLostTest, CreateRenderBundleEncoderFails) {
    SetCallbackAndLoseForTesting();

    wgpu::RenderBundleEncoderDescriptor descriptor;
    descriptor.colorFormatsCount = 0;
    descriptor.colorFormats = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&descriptor));
}

// Tests that CreateComputePipeline fails when device is lost
TEST_P(DeviceLostTest, CreateComputePipelineFails) {
    SetCallbackAndLoseForTesting();

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.computeStage.module = nullptr;
    descriptor.nextInChain = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&descriptor));
}

// Tests that CreateRenderPipeline fails when device is lost
TEST_P(DeviceLostTest, CreateRenderPipelineFails) {
    SetCallbackAndLoseForTesting();

    utils::ComboRenderPipelineDescriptor descriptor(device);
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Tests that CreateSampler fails when device is lost
TEST_P(DeviceLostTest, CreateSamplerFails) {
    SetCallbackAndLoseForTesting();

    wgpu::SamplerDescriptor descriptor = utils::GetDefaultSamplerDescriptor();
    ASSERT_DEVICE_ERROR(device.CreateSampler(&descriptor));
}

// Tests that CreateShaderModule fails when device is lost
TEST_P(DeviceLostTest, CreateShaderModuleFails) {
    SetCallbackAndLoseForTesting();

    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) in vec4 color;
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = color;
                })"));
}

// Tests that CreateSwapChain fails when device is lost
TEST_P(DeviceLostTest, CreateSwapChainFails) {
    SetCallbackAndLoseForTesting();

    wgpu::SwapChainDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(nullptr, &descriptor));
}

// Tests that CreateTexture fails when device is lost
TEST_P(DeviceLostTest, CreateTextureFails) {
    SetCallbackAndLoseForTesting();

    wgpu::TextureDescriptor descriptor;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.size.depth = 1;
    descriptor.arrayLayerCount = 1;
    descriptor.mipLevelCount = 1;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.usage = wgpu::TextureUsage::OutputAttachment;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

TEST_P(DeviceLostTest, TickFails) {
    SetCallbackAndLoseForTesting();
    ASSERT_DEVICE_ERROR(device.Tick());
}

// Test that CreateBuffer fails when device is lost
TEST_P(DeviceLostTest, CreateBufferFails) {
    SetCallbackAndLoseForTesting();

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&bufferDescriptor));
}

// Test that buffer.MapWriteAsync fails after device is lost
TEST_P(DeviceLostTest, BufferMapWriteAsyncFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    SetCallbackAndLoseForTesting();
    ASSERT_DEVICE_ERROR(buffer.MapWriteAsync(CheckMapWriteFail, const_cast<int*>(&fakeUserData)));
}

// Test that buffer.MapWriteAsync calls back with device loss status
TEST_P(DeviceLostTest, BufferMapWriteAsyncBeforeLossFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    buffer.MapWriteAsync(CheckMapWriteFail, const_cast<int*>(&fakeUserData));
    SetCallbackAndLoseForTesting();
}

// Test that buffer.Unmap fails after device is lost
TEST_P(DeviceLostTest, BufferUnmapFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);
    wgpu::CreateBufferMappedResult result = device.CreateBufferMapped(&bufferDescriptor);

    SetCallbackAndLoseForTesting();
    ASSERT_DEVICE_ERROR(result.buffer.Unmap());
}

// Test that CreateBufferMapped fails after device is lost
TEST_P(DeviceLostTest, CreateBufferMappedFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;

    SetCallbackAndLoseForTesting();
    ASSERT_DEVICE_ERROR(device.CreateBufferMapped(&bufferDescriptor));
}

// Test that CreateBufferMappedAsync fails after device is lost
TEST_P(DeviceLostTest, CreateBufferMappedAsyncFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapWrite;

    SetCallbackAndLoseForTesting();
    struct ResultInfo {
        wgpu::CreateBufferMappedResult result;
        bool done = false;
    } resultInfo;

    ASSERT_DEVICE_ERROR(device.CreateBufferMappedAsync(
        &bufferDescriptor,
        [](WGPUBufferMapAsyncStatus status, WGPUCreateBufferMappedResult result, void* userdata) {
            auto* resultInfo = static_cast<ResultInfo*>(userdata);
            EXPECT_EQ(WGPUBufferMapAsyncStatus_DeviceLost, status);
            EXPECT_NE(nullptr, result.data);
            resultInfo->result.buffer = wgpu::Buffer::Acquire(result.buffer);
            resultInfo->result.data = result.data;
            resultInfo->result.dataLength = result.dataLength;
            resultInfo->done = true;
        },
        &resultInfo));

    while (!resultInfo.done) {
        ASSERT_DEVICE_ERROR(WaitABit());
    }

    ASSERT_DEVICE_ERROR(resultInfo.result.buffer.Unmap());
}

// Test that BufferMapReadAsync fails after device is lost
TEST_P(DeviceLostTest, BufferMapReadAsyncFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    SetCallbackAndLoseForTesting();
    ASSERT_DEVICE_ERROR(buffer.MapReadAsync(CheckMapReadFail, const_cast<int*>(&fakeUserData)));
}

// Test that BufferMapReadAsync calls back with device lost status when device lost after map read
TEST_P(DeviceLostTest, BufferMapReadAsyncBeforeLossFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    buffer.MapReadAsync(CheckMapReadFail, const_cast<int*>(&fakeUserData));
    SetCallbackAndLoseForTesting();
}

// Test that SetSubData fails after device is lost
TEST_P(DeviceLostTest, SetSubDataFails) {
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(float);
    bufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    SetCallbackAndLoseForTesting();
    std::array<float, 1> data = {12};
    ASSERT_DEVICE_ERROR(buffer.SetSubData(0, sizeof(float), data.data()));
}

// Test that Command Encoder Finish fails when device lost
TEST_P(DeviceLostTest, CommandEncoderFinishFails) {
    wgpu::CommandBuffer commands;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    SetCallbackAndLoseForTesting();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

DAWN_INSTANTIATE_TEST(DeviceLostTest, D3D12Backend, VulkanBackend);