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

#include <vector>

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {
using testing::HasSubstr;
}  // anonymous namespace

class UnsafeAPIValidationTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice() override {
        wgpu::DeviceDescriptor descriptor;
        wgpu::DawnTogglesDeviceDescriptor togglesDesc;
        descriptor.nextInChain = &togglesDesc;
        const char* toggle = "disallow_unsafe_apis";
        togglesDesc.forceEnabledToggles = &toggle;
        togglesDesc.forceEnabledTogglesCount = 1;
        return adapter.CreateDevice(&descriptor);
    }
};

// Check that pipeline overridable constants are disallowed as part of unsafe APIs.
// TODO(dawn:1041) Remove when implementation for all backend is added
TEST_F(UnsafeAPIValidationTest, PipelineOverridableConstants) {
    // Create the placeholder compute pipeline.
    wgpu::ComputePipelineDescriptor pipelineDescBase;
    pipelineDescBase.compute.entryPoint = "main";

    // Control case: shader without overridable constant is allowed.
    {
        wgpu::ComputePipelineDescriptor pipelineDesc = pipelineDescBase;
        pipelineDesc.compute.module =
            utils::CreateShaderModule(device, "@stage(compute) @workgroup_size(1) fn main() {}");

        device.CreateComputePipeline(&pipelineDesc);
    }

    // Error case: shader with overridable constant with default value
    {
        ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
@id(1000) override c0: u32 = 1u;
@id(1000) override c1: u32;

@stage(compute) @workgroup_size(1) fn main() {
    _ = c0;
    _ = c1;
})"));
    }

    // Error case: pipeline stage with constant entry is disallowed
    {
        wgpu::ComputePipelineDescriptor pipelineDesc = pipelineDescBase;
        pipelineDesc.compute.module =
            utils::CreateShaderModule(device, "@stage(compute) @workgroup_size(1) fn main() {}");
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c", 1u}};
        pipelineDesc.compute.constants = constants.data();
        pipelineDesc.compute.constantCount = constants.size();
        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&pipelineDesc));
    }
}

class UnsafeQueryAPIValidationTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice() override {
        wgpu::DeviceDescriptor descriptor;
        wgpu::FeatureName requiredFeatures[2] = {wgpu::FeatureName::PipelineStatisticsQuery,
                                                 wgpu::FeatureName::TimestampQuery};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeaturesCount = 2;

        wgpu::DawnTogglesDeviceDescriptor togglesDesc;
        descriptor.nextInChain = &togglesDesc;
        const char* toggle = "disallow_unsafe_apis";
        togglesDesc.forceEnabledToggles = &toggle;
        togglesDesc.forceEnabledTogglesCount = 1;

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
