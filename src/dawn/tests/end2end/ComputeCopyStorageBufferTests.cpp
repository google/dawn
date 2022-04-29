// Copyright 2018 The Dawn Authors
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

#include <array>

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/WGPUHelpers.h"

class ComputeCopyStorageBufferTests : public DawnTest {
  public:
    static constexpr int kInstances = 4;
    static constexpr int kUintsPerInstance = 4;
    static constexpr int kNumUints = kInstances * kUintsPerInstance;

    void BasicTest(const char* shader);
};

void ComputeCopyStorageBufferTests::BasicTest(const char* shader) {
    // Set up shader and pipeline
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "main";

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up src storage buffer
    wgpu::BufferDescriptor srcDesc;
    srcDesc.size = kNumUints * sizeof(uint32_t);
    srcDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer src = device.CreateBuffer(&srcDesc);

    std::array<uint32_t, kNumUints> expected;
    for (uint32_t i = 0; i < kNumUints; ++i) {
        expected[i] = (i + 1u) * 0x11111111u;
    }
    queue.WriteBuffer(src, 0, expected.data(), sizeof(expected));
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), src, 0, kNumUints);

    // Set up dst storage buffer
    wgpu::BufferDescriptor dstDesc;
    dstDesc.size = kNumUints * sizeof(uint32_t);
    dstDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dst = device.CreateBuffer(&dstDesc);

    std::array<uint32_t, kNumUints> zero{};
    queue.WriteBuffer(dst, 0, zero.data(), sizeof(zero));

    // Set up bind group and issue dispatch
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, src, 0, kNumUints * sizeof(uint32_t)},
                                                         {1, dst, 0, kNumUints * sizeof(uint32_t)},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(kInstances);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), dst, 0, kNumUints);
}

// Test that a trivial compute-shader memcpy implementation works.
TEST_P(ComputeCopyStorageBufferTests, SizedArrayOfBasic) {
    BasicTest(R"(
        struct Buf {
            s : array<vec4<u32>, 4>
        }

        @group(0) @binding(0) var<storage, read_write> src : Buf;
        @group(0) @binding(1) var<storage, read_write> dst : Buf;

        @stage(compute) @workgroup_size(1)
        fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
            let index : u32 = GlobalInvocationID.x;
            if (index >= 4u) { return; }
            dst.s[index] = src.s[index];
        })");
}

// Test that a slightly-less-trivial compute-shader memcpy implementation works.
TEST_P(ComputeCopyStorageBufferTests, SizedArrayOfStruct) {
    BasicTest(R"(
        struct S {
            a : vec2<u32>,
            b : vec2<u32>,
        }

        struct Buf {
            s : array<S, 4>
        }

        @group(0) @binding(0) var<storage, read_write> src : Buf;
        @group(0) @binding(1) var<storage, read_write> dst : Buf;

        @stage(compute) @workgroup_size(1)
        fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
            let index : u32 = GlobalInvocationID.x;
            if (index >= 4u) { return; }
            dst.s[index] = src.s[index];
        })");
}

// Test that a trivial compute-shader memcpy implementation works.
TEST_P(ComputeCopyStorageBufferTests, UnsizedArrayOfBasic) {
    BasicTest(R"(
        struct Buf {
            s : array<vec4<u32>>
        }

        @group(0) @binding(0) var<storage, read_write> src : Buf;
        @group(0) @binding(1) var<storage, read_write> dst : Buf;

        @stage(compute) @workgroup_size(1)
        fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
            let index : u32 = GlobalInvocationID.x;
            if (index >= 4u) { return; }
            dst.s[index] = src.s[index];
        })");
}

DAWN_INSTANTIATE_TEST(ComputeCopyStorageBufferTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
