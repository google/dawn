// Copyright 2017 The Dawn Authors
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

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

// TODO(cwallez@chromium.org): Add a regression test for Disptach validation trying to acces the
// input state.

class ComputeValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        wgpu::ShaderModule computeModule = utils::CreateShaderModule(device, R"(
            @stage(compute) @workgroup_size(1) fn main() {
            })");

        // Set up compute pipeline
        wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, nullptr);

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = pl;
        csDesc.compute.module = computeModule;
        csDesc.compute.entryPoint = "main";
        pipeline = device.CreateComputePipeline(&csDesc);
    }

    void TestDispatch(uint32_t x, uint32_t y, uint32_t z) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(x, y, z);
        pass.End();
        encoder.Finish();
    }

    wgpu::ComputePipeline pipeline;
};

// Check that 1x1x1 dispatch is OK.
TEST_F(ComputeValidationTest, PerDimensionDispatchSizeLimits_SmallestValid) {
    TestDispatch(1, 1, 1);
}

// Check that the largest allowed dispatch is OK.
TEST_F(ComputeValidationTest, PerDimensionDispatchSizeLimits_LargestValid) {
    const uint32_t max = GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;
    TestDispatch(max, max, max);
}

// Check that exceeding the maximum on the X dimension results in validation failure.
TEST_F(ComputeValidationTest, PerDimensionDispatchSizeLimits_InvalidX) {
    const uint32_t max = GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;
    ASSERT_DEVICE_ERROR(TestDispatch(max + 1, 1, 1));
}

// Check that exceeding the maximum on the Y dimension results in validation failure.
TEST_F(ComputeValidationTest, PerDimensionDispatchSizeLimits_InvalidY) {
    const uint32_t max = GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;
    ASSERT_DEVICE_ERROR(TestDispatch(1, max + 1, 1));
}

// Check that exceeding the maximum on the Z dimension results in validation failure.
TEST_F(ComputeValidationTest, PerDimensionDispatchSizeLimits_InvalidZ) {
    const uint32_t max = GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;
    ASSERT_DEVICE_ERROR(TestDispatch(1, 1, max + 1));
}

// Check that exceeding the maximum on all dimensions results in validation failure.
TEST_F(ComputeValidationTest, PerDimensionDispatchSizeLimits_InvalidAll) {
    const uint32_t max = GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;
    ASSERT_DEVICE_ERROR(TestDispatch(max + 1, max + 1, max + 1));
}
