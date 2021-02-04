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

#include "utils/WGPUHelpers.h"

#include <initializer_list>

constexpr static std::initializer_list<uint32_t> kSentinelData{0, 0, 0};

class ComputeDispatchTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Write workgroup number into the output buffer if we saw the biggest dispatch
        // This is a workaround since D3D12 doesn't have gl_NumWorkGroups
        // To make sure the dispatch was not called, write maximum u32 value for 0 dispatches
        wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[block]] struct InputBuf {
                [[offset(0)]] expectedDispatch : vec3<u32>;
            };
            [[block]] struct OutputBuf {
                [[offset(0)]] workGroups : vec3<u32>;
            };

            [[group(0), binding(0)]] var<uniform> input : InputBuf;
            [[group(0), binding(1)]] var<storage_buffer> output : OutputBuf;

            [[builtin(global_invocation_id)]] var<in> GlobalInvocationID : vec3<u32>;

            [[stage(compute), workgroup_size(1, 1, 1)]]
            fn main() -> void {
                const dispatch : vec3<u32> = input.expectedDispatch;

                if (dispatch.x == 0 || dispatch.y == 0 || dispatch.z == 0) {
                    output.workGroups = vec3<u32>(0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
                    return;
                }

                if (all(GlobalInvocationID == dispatch - vec3<u32>(1u, 1u, 1u))) {
                    output.workGroups = dispatch;
                }
            })");

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.computeStage.module = module;
        csDesc.computeStage.entryPoint = "main";
        pipeline = device.CreateComputePipeline(&csDesc);
    }

    void DirectTest(uint32_t x, uint32_t y, uint32_t z) {
        // Set up dst storage buffer to contain dispatch x, y, z
        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device,
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst,
            kSentinelData);

        std::initializer_list<uint32_t> expectedBufferData{x, y, z};
        wgpu::Buffer expectedBuffer = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Uniform, expectedBufferData);

        // Set up bind group and issue dispatch
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, expectedBuffer, 0, 3 * sizeof(uint32_t)},
                                     {1, dst, 0, 3 * sizeof(uint32_t)},
                                 });

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Dispatch(x, y, z);
            pass.EndPass();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        std::vector<uint32_t> expected =
            x == 0 || y == 0 || z == 0 ? kSentinelData : expectedBufferData;

        // Verify the dispatch got called if all group counts are not zero
        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], dst, 0, 3);
    }

    void IndirectTest(std::vector<uint32_t> indirectBufferData, uint64_t indirectOffset) {
        // Set up dst storage buffer to contain dispatch x, y, z
        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device,
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst,
            kSentinelData);

        wgpu::Buffer indirectBuffer = utils::CreateBufferFromData(
            device, &indirectBufferData[0], indirectBufferData.size() * sizeof(uint32_t),
            wgpu::BufferUsage::Indirect);

        uint32_t indirectStart = indirectOffset / sizeof(uint32_t);

        wgpu::Buffer expectedBuffer =
            utils::CreateBufferFromData(device, &indirectBufferData[indirectStart],
                                        3 * sizeof(uint32_t), wgpu::BufferUsage::Uniform);

        // Set up bind group and issue dispatch
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, expectedBuffer, 0, 3 * sizeof(uint32_t)},
                                     {1, dst, 0, 3 * sizeof(uint32_t)},
                                 });

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchIndirect(indirectBuffer, indirectOffset);
            pass.EndPass();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        std::vector<uint32_t> expected;
        if (indirectBufferData[indirectStart] == 0 || indirectBufferData[indirectStart + 1] == 0 ||
            indirectBufferData[indirectStart + 2] == 0) {
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
    IndirectTest({2, 3, 4}, 0);
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
    IndirectTest({0, 0, 0, 2, 3, 4}, 3 * sizeof(uint32_t));
}

DAWN_INSTANTIATE_TEST(ComputeDispatchTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
