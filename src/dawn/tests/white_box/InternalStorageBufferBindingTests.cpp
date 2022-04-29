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

#include <vector>

#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

class InternalStorageBufferBindingTests : public DawnTest {
  protected:
    static constexpr uint32_t kNumValues = 4;
    static constexpr uint32_t kIterations = 4;

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    wgpu::ComputePipeline CreateComputePipelineWithInternalStorage() {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            struct Buf {
                data : array<u32, 4>
            }

            @group(0) @binding(0) var<storage, read_write> buf : Buf;

            @stage(compute) @workgroup_size(1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
                buf.data[GlobalInvocationID.x] = buf.data[GlobalInvocationID.x] + 0x1234u;
            }
        )");

        // Create binding group layout with internal storage buffer binding type
        dawn::native::BindGroupLayoutEntry bglEntry;
        bglEntry.binding = 0;
        bglEntry.buffer.type = dawn::native::kInternalStorageBufferBinding;
        bglEntry.visibility = wgpu::ShaderStage::Compute;

        dawn::native::BindGroupLayoutDescriptor bglDesc;
        bglDesc.entryCount = 1;
        bglDesc.entries = &bglEntry;

        dawn::native::DeviceBase* nativeDevice = dawn::native::FromAPI(device.Get());

        Ref<dawn::native::BindGroupLayoutBase> bglRef =
            nativeDevice->CreateBindGroupLayout(&bglDesc, true).AcquireSuccess();

        wgpu::BindGroupLayout bgl =
            wgpu::BindGroupLayout::Acquire(dawn::native::ToAPI(bglRef.Detach()));

        // Create pipeline layout
        wgpu::PipelineLayoutDescriptor plDesc;
        plDesc.bindGroupLayoutCount = 1;
        plDesc.bindGroupLayouts = &bgl;
        wgpu::PipelineLayout layout = device.CreatePipelineLayout(&plDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc = {};
        pipelineDesc.layout = layout;
        pipelineDesc.compute.module = module;
        pipelineDesc.compute.entryPoint = "main";

        return device.CreateComputePipeline(&pipelineDesc);
    }
};

// Test that query resolve buffer can be bound as internal storage buffer, multiple dispatches to
// increment values in the query resolve buffer are synchronized.
TEST_P(InternalStorageBufferBindingTests, QueryResolveBufferBoundAsInternalStorageBuffer) {
    std::vector<uint32_t> data(kNumValues, 0);
    std::vector<uint32_t> expected(kNumValues, 0x1234u * kIterations);

    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, data.data(), bufferSize,
                                    wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::CopySrc);

    wgpu::ComputePipeline pipeline = CreateComputePipelineWithInternalStorage();

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer, 0, bufferSize}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    for (uint32_t i = 0; i < kIterations; ++i) {
        pass.DispatchWorkgroups(kNumValues);
    }
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kNumValues);
}

DAWN_INSTANTIATE_TEST(InternalStorageBufferBindingTests,
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend());
