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

#include <initializer_list>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

constexpr static std::initializer_list<uint32_t> kSentinelData{0, 0, 0};

class ComputeDispatchTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Write workgroup number into the output buffer if we saw the biggest dispatch
        // To make sure the dispatch was not called, write maximum u32 value for 0 dispatches
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            struct OutputBuf {
                workGroups : vec3<u32>
            }

            @group(0) @binding(0) var<storage, read_write> output : OutputBuf;

            @stage(compute) @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>,
                    @builtin(num_workgroups) dispatch : vec3<u32>) {
                if (dispatch.x == 0u || dispatch.y == 0u || dispatch.z == 0u) {
                    output.workGroups = vec3<u32>(0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
                    return;
                }

                if (all(GlobalInvocationID == dispatch - vec3<u32>(1u, 1u, 1u))) {
                    output.workGroups = dispatch;
                }
            })");

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = module;
        csDesc.compute.entryPoint = "main";
        pipeline = device.CreateComputePipeline(&csDesc);

        // Test the use of the compute pipelines without using @num_workgroups
        wgpu::ShaderModule moduleWithoutNumWorkgroups = utils::CreateShaderModule(device, R"(
            struct InputBuf {
                expectedDispatch : vec3<u32>
            }
            struct OutputBuf {
                workGroups : vec3<u32>
            }

            @group(0) @binding(0) var<uniform> input : InputBuf;
            @group(0) @binding(1) var<storage, read_write> output : OutputBuf;

            @stage(compute) @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
                let dispatch : vec3<u32> = input.expectedDispatch;

                if (dispatch.x == 0u || dispatch.y == 0u || dispatch.z == 0u) {
                    output.workGroups = vec3<u32>(0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
                    return;
                }

                if (all(GlobalInvocationID == dispatch - vec3<u32>(1u, 1u, 1u))) {
                    output.workGroups = dispatch;
                }
            })");
        csDesc.compute.module = moduleWithoutNumWorkgroups;
        pipelineWithoutNumWorkgroups = device.CreateComputePipeline(&csDesc);
    }

    void DirectTest(uint32_t x, uint32_t y, uint32_t z) {
        // Set up dst storage buffer to contain dispatch x, y, z
        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device,
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst,
            kSentinelData);

        // Set up bind group and issue dispatch
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {
                                                             {0, dst, 0, 3 * sizeof(uint32_t)},
                                                         });

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchWorkgroups(x, y, z);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        std::vector<uint32_t> expected =
            x == 0 || y == 0 || z == 0 ? kSentinelData : std::initializer_list<uint32_t>{x, y, z};

        // Verify the dispatch got called if all group counts are not zero
        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], dst, 0, 3);
    }

    void IndirectTest(std::vector<uint32_t> indirectBufferData,
                      uint64_t indirectOffset,
                      bool useNumWorkgroups = true) {
        // Set up dst storage buffer to contain dispatch x, y, z
        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device,
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst,
            kSentinelData);

        wgpu::Buffer indirectBuffer = utils::CreateBufferFromData(
            device, &indirectBufferData[0], indirectBufferData.size() * sizeof(uint32_t),
            wgpu::BufferUsage::Indirect);

        uint32_t indirectStart = indirectOffset / sizeof(uint32_t);

        // Set up bind group and issue dispatch
        wgpu::BindGroup bindGroup;
        wgpu::ComputePipeline computePipelineForTest;

        if (useNumWorkgroups) {
            computePipelineForTest = pipeline;
            bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                             {
                                                 {0, dst, 0, 3 * sizeof(uint32_t)},
                                             });
        } else {
            computePipelineForTest = pipelineWithoutNumWorkgroups;
            wgpu::Buffer expectedBuffer =
                utils::CreateBufferFromData(device, &indirectBufferData[indirectStart],
                                            3 * sizeof(uint32_t), wgpu::BufferUsage::Uniform);
            bindGroup =
                utils::MakeBindGroup(device, pipelineWithoutNumWorkgroups.GetBindGroupLayout(0),
                                     {
                                         {0, expectedBuffer, 0, 3 * sizeof(uint32_t)},
                                         {1, dst, 0, 3 * sizeof(uint32_t)},
                                     });
        }

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(computePipelineForTest);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchWorkgroupsIndirect(indirectBuffer, indirectOffset);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        std::vector<uint32_t> expected;

        uint32_t maxComputeWorkgroupsPerDimension =
            GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;
        if (indirectBufferData[indirectStart] == 0 || indirectBufferData[indirectStart + 1] == 0 ||
            indirectBufferData[indirectStart + 2] == 0 ||
            indirectBufferData[indirectStart] > maxComputeWorkgroupsPerDimension ||
            indirectBufferData[indirectStart + 1] > maxComputeWorkgroupsPerDimension ||
            indirectBufferData[indirectStart + 2] > maxComputeWorkgroupsPerDimension) {
            expected = kSentinelData;
        } else {
            expected.assign(indirectBufferData.begin() + indirectStart,
                            indirectBufferData.begin() + indirectStart + 3);
        }

        // Verify the dispatch got called with group counts in indirect buffer if all group counts
        // are not zero
        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], dst, 0, 3);
    }

  private:
    wgpu::ComputePipeline pipeline;
    wgpu::ComputePipeline pipelineWithoutNumWorkgroups;
};

// Test basic direct
TEST_P(ComputeDispatchTests, DirectBasic) {
    DirectTest(2, 3, 4);
}

// Test no-op direct
TEST_P(ComputeDispatchTests, DirectNoop) {
    // All dimensions are 0s
    DirectTest(0, 0, 0);

    // Only x dimension is 0
    DirectTest(0, 3, 4);

    // Only y dimension is 0
    DirectTest(2, 0, 4);

    // Only z dimension is 0
    DirectTest(2, 3, 0);
}

// Test basic indirect
TEST_P(ComputeDispatchTests, IndirectBasic) {
#ifdef DAWN_PLATFORM_32_BIT
    // TODO(crbug.com/dawn/1196): Fails on Chromium's Quadro P400 bots
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
#endif
    // TODO(crbug.com/dawn/1262): Fails with the full validation turned on.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    IndirectTest({2, 3, 4}, 0);
}

// Test basic indirect without using @num_workgroups
TEST_P(ComputeDispatchTests, IndirectBasicWithoutNumWorkgroups) {
    IndirectTest({2, 3, 4}, 0, false);
}

// Test no-op indirect
TEST_P(ComputeDispatchTests, IndirectNoop) {
    // All dimensions are 0s
    IndirectTest({0, 0, 0}, 0);

    // Only x dimension is 0
    IndirectTest({0, 3, 4}, 0);

    // Only y dimension is 0
    IndirectTest({2, 0, 4}, 0);

    // Only z dimension is 0
    IndirectTest({2, 3, 0}, 0);
}

// Test indirect with buffer offset
TEST_P(ComputeDispatchTests, IndirectOffset) {
#ifdef DAWN_PLATFORM_32_BIT
    // TODO(crbug.com/dawn/1196): Fails on Chromium's Quadro P400 bots
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
#endif
    // TODO(crbug.com/dawn/1262): Fails with the full validation turned on.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    IndirectTest({0, 0, 0, 2, 3, 4}, 3 * sizeof(uint32_t));
}

// Test indirect with buffer offset without using @num_workgroups
TEST_P(ComputeDispatchTests, IndirectOffsetWithoutNumWorkgroups) {
    IndirectTest({0, 0, 0, 2, 3, 4}, 3 * sizeof(uint32_t), false);
}

// Test indirect dispatches at max limit.
TEST_P(ComputeDispatchTests, MaxWorkgroups) {
#ifdef DAWN_PLATFORM_32_BIT
    // TODO(crbug.com/dawn/1196): Fails on Chromium's Quadro P400 bots
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());
#endif
    // TODO(crbug.com/dawn/1262): Fails with the full validation turned on.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    // TODO(crbug.com/dawn/1165): Fails with WARP
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    uint32_t max = GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;

    // Test that the maximum works in each dimension.
    // Note: Testing (max, max, max) is very slow.
    IndirectTest({max, 3, 4}, 0);
    IndirectTest({2, max, 4}, 0);
    IndirectTest({2, 3, max}, 0);
}

// Test indirect dispatches exceeding the max limit are noop-ed.
TEST_P(ComputeDispatchTests, ExceedsMaxWorkgroupsNoop) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    // TODO(crbug.com/dawn/839): Investigate why this test fails with WARP.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    uint32_t max = GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;

    // All dimensions are above the max
    IndirectTest({max + 1, max + 1, max + 1}, 0);

    // Only x dimension is above the max
    IndirectTest({max + 1, 3, 4}, 0);
    IndirectTest({2 * max, 3, 4}, 0);

    // Only y dimension is above the max
    IndirectTest({2, max + 1, 4}, 0);
    IndirectTest({2, 2 * max, 4}, 0);

    // Only z dimension is above the max
    IndirectTest({2, 3, max + 1}, 0);
    IndirectTest({2, 3, 2 * max}, 0);
}

// Test indirect dispatches exceeding the max limit with an offset are noop-ed.
TEST_P(ComputeDispatchTests, ExceedsMaxWorkgroupsWithOffsetNoop) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    // TODO(crbug.com/dawn/839): Investigate why this test fails with WARP.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    uint32_t max = GetSupportedLimits().limits.maxComputeWorkgroupsPerDimension;

    IndirectTest({1, 2, 3, max + 1, 4, 5}, 1 * sizeof(uint32_t));
    IndirectTest({1, 2, 3, max + 1, 4, 5}, 2 * sizeof(uint32_t));
    IndirectTest({1, 2, 3, max + 1, 4, 5}, 3 * sizeof(uint32_t));
}

DAWN_INSTANTIATE_TEST(ComputeDispatchTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
