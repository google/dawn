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

#include "tests/unittests/validation/ValidationTest.h"

#include "tests/MockCallback.h"
#include "utils/ComboRenderBundleEncoderDescriptor.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class UnsafeAPIValidationTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice() override {
        dawn_native::DeviceDescriptor descriptor;
        descriptor.forceEnabledToggles.push_back("disallow_unsafe_apis");
        return adapter.CreateDevice(&descriptor);
    }
};

// Check that DispatchIndirect is disallowed as part of unsafe APIs.
TEST_F(UnsafeAPIValidationTest, DispatchIndirectDisallowed) {
    // Create the index and indirect buffers.
    wgpu::BufferDescriptor indirectBufferDesc;
    indirectBufferDesc.size = 64;
    indirectBufferDesc.usage = wgpu::BufferUsage::Indirect;
    wgpu::Buffer indirectBuffer = device.CreateBuffer(&indirectBufferDesc);

    // Create the dummy compute pipeline.
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.compute.entryPoint = "main";
    pipelineDesc.compute.module =
        utils::CreateShaderModule(device, "[[stage(compute), workgroup_size(1)]] fn main() {}");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    // Control case: dispatch is allowed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        pass.SetPipeline(pipeline);
        pass.Dispatch(1, 1, 1);

        pass.EndPass();
        encoder.Finish();
    }

    // Error case: dispatch indirect is disallowed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        pass.SetPipeline(pipeline);
        pass.DispatchIndirect(indirectBuffer, 0);

        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that dynamic storage buffers are disallowed.
TEST_F(UnsafeAPIValidationTest, DynamicStorageBuffer) {
    wgpu::BindGroupLayoutEntry entry;
    entry.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entries = &entry;
    desc.entryCount = 1;

    // Control case: storage buffer without a dynamic offset is allowed.
    {
        entry.buffer.type = wgpu::BufferBindingType::Storage;
        entry.buffer.hasDynamicOffset = false;
        device.CreateBindGroupLayout(&desc);
    }

    // Control case: readonly storage buffer without a dynamic offset is allowed.
    {
        entry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        entry.buffer.hasDynamicOffset = false;
        device.CreateBindGroupLayout(&desc);
    }

    // Storage buffer with a dynamic offset is disallowed.
    {
        entry.buffer.type = wgpu::BufferBindingType::Storage;
        entry.buffer.hasDynamicOffset = true;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }

    // Readonly storage buffer with a dynamic offset is disallowed.
    {
        entry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        entry.buffer.hasDynamicOffset = true;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }
}

class UnsafeQueryAPIValidationTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice() override {
        dawn_native::DeviceDescriptor descriptor;
        descriptor.requiredFeatures.push_back("pipeline_statistics_query");
        descriptor.requiredFeatures.push_back("timestamp_query");
        descriptor.forceEnabledToggles.push_back("disallow_unsafe_apis");
        return adapter.CreateDevice(&descriptor);
    }
};

// Check that pipeline statistics query are disallowed.
TEST_F(UnsafeQueryAPIValidationTest, PipelineStatisticsDisallowed) {
    wgpu::QuerySetDescriptor descriptor;
    descriptor.count = 1;

    // Control case: occlusion query creation is allowed.
    {
        descriptor.type = wgpu::QueryType::Occlusion;
        device.CreateQuerySet(&descriptor);
    }

    // Error case: pipeline statistics query creation is disallowed.
    {
        descriptor.type = wgpu::QueryType::PipelineStatistics;
        std::vector<wgpu::PipelineStatisticName> pipelineStatistics = {
            wgpu::PipelineStatisticName::VertexShaderInvocations};
        descriptor.pipelineStatistics = pipelineStatistics.data();
        descriptor.pipelineStatisticsCount = pipelineStatistics.size();
        ASSERT_DEVICE_ERROR(device.CreateQuerySet(&descriptor));
    }
}

// Check timestamp queries are disallowed.
TEST_F(UnsafeQueryAPIValidationTest, TimestampQueryDisallowed) {
    wgpu::QuerySetDescriptor descriptor;
    descriptor.count = 1;

    // Control case: occlusion query creation is allowed.
    {
        descriptor.type = wgpu::QueryType::Occlusion;
        device.CreateQuerySet(&descriptor);
    }

    // Error case: timestamp query creation is disallowed.
    {
        descriptor.type = wgpu::QueryType::Timestamp;
        ASSERT_DEVICE_ERROR(device.CreateQuerySet(&descriptor));
    }
}
