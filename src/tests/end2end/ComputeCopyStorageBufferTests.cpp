// Copyright 2018 The NXT Authors
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

#include "tests/NXTTest.h"

#include "utils/NXTHelpers.h"

#include <array>

class ComputeCopyStorageBufferTests : public NXTTest {
  public:
    static constexpr int kInstances = 4;
    static constexpr int kUintsPerInstance = 4;
    static constexpr int kNumUints = kInstances * kUintsPerInstance;

    void BasicTest(const char* shader);
};

void ComputeCopyStorageBufferTests::BasicTest(const char* shader) {
    auto bgl = utils::MakeBindGroupLayout(
        device, {
                    {0, nxt::ShaderStageBit::Compute, nxt::BindingType::StorageBuffer},
                    {1, nxt::ShaderStageBit::Compute, nxt::BindingType::StorageBuffer},
                });

    // Set up shader and pipeline
    auto module = utils::CreateShaderModule(device, nxt::ShaderStage::Compute, shader);
    auto pl = utils::MakeBasicPipelineLayout(device, &bgl);
    auto pipeline = device.CreateComputePipelineBuilder()
                        .SetLayout(pl)
                        .SetStage(nxt::ShaderStage::Compute, module, "main")
                        .GetResult();

    // Set up src storage buffer
    auto src =
        device.CreateBufferBuilder()
            .SetSize(kNumUints * sizeof(uint32_t))
            .SetAllowedUsage(nxt::BufferUsageBit::Storage | nxt::BufferUsageBit::TransferSrc |
                             nxt::BufferUsageBit::TransferDst)
            .GetResult();
    std::array<uint32_t, kNumUints> expected;
    for (uint32_t i = 0; i < kNumUints; ++i) {
        expected[i] = (i + 1u) * 0x11111111u;
    }
    src.SetSubData(0, sizeof(expected), reinterpret_cast<const uint8_t*>(expected.data()));
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), src, 0, kNumUints);
    auto srcView =
        src.CreateBufferViewBuilder().SetExtent(0, kNumUints * sizeof(uint32_t)).GetResult();

    // Set up dst storage buffer
    auto dst =
        device.CreateBufferBuilder()
            .SetSize(kNumUints * sizeof(uint32_t))
            .SetAllowedUsage(nxt::BufferUsageBit::Storage | nxt::BufferUsageBit::TransferSrc |
                             nxt::BufferUsageBit::TransferDst)
            .GetResult();
    std::array<uint32_t, kNumUints> zero{};
    dst.SetSubData(0, sizeof(zero), reinterpret_cast<const uint8_t*>(zero.data()));
    auto dstView =
        dst.CreateBufferViewBuilder().SetExtent(0, kNumUints * sizeof(uint32_t)).GetResult();

    // Set up bind group and issue dispatch
    auto bindGroup = device.CreateBindGroupBuilder()
                         .SetLayout(bgl)
                         .SetUsage(nxt::BindGroupUsage::Frozen)
                         .SetBufferViews(0, 1, &srcView)
                         .SetBufferViews(1, 1, &dstView)
                         .GetResult();
    auto commands = device.CreateCommandBufferBuilder()
                        .BeginComputePass()
                        .SetComputePipeline(pipeline)
                        .SetBindGroup(0, bindGroup)
                        .Dispatch(kInstances, 1, 1)
                        .EndComputePass()
                        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), dst, 0, kNumUints);
}

// Test that a trivial compute-shader memcpy implementation works.
TEST_P(ComputeCopyStorageBufferTests, BasicTest) {
    BasicTest(R"(
        #version 450
        #define kInstances 4
        layout(std140, set = 0, binding = 0) buffer Src { uvec4 s[kInstances]; } src;
        layout(std140, set = 0, binding = 1) buffer Dst { uvec4 s[kInstances]; } dst;
        void main() {
            uint index = gl_GlobalInvocationID.x;
            if (index >= kInstances) { return; }
            dst.s[index] = src.s[index];
        })");
}

// Test that a slightly-less-trivial compute-shader memcpy implementation works.
TEST_P(ComputeCopyStorageBufferTests, StructTest) {
    BasicTest(R"(
        #version 450
        #define kInstances 4
        struct S {
            uvec2 a, b;  // kUintsPerInstance = 4
        };
        layout(std140, set = 0, binding = 0) buffer Src { S s[kInstances]; } src;
        layout(std140, set = 0, binding = 1) buffer Dst { S s[kInstances]; } dst;
        void main() {
            uint index = gl_GlobalInvocationID.x;
            if (index >= kInstances) { return; }
            dst.s[index] = src.s[index];
        })");
}

// Test with a sized array SSBO.
TEST_P(ComputeCopyStorageBufferTests, DISABLED_SizedArray) {
    // TODO(kainino@chromium.org): Fails on OpenGL (only copies one instance, not 4).
    // TODO(kainino@chromium.org): Fails on Vulkan (program hangs).
    BasicTest(R"(
        #version 450
        #define kInstances 4
        struct S {
            uvec2 a, b;  // kUintsPerInstance = 4
        };
        layout(std140, set = 0, binding = 0) buffer Src { S s; } src[kInstances];
        layout(std140, set = 0, binding = 1) buffer Dst { S s; } dst[kInstances];
        void main() {
            uint index = gl_GlobalInvocationID.x;
            if (index >= kInstances) { return; }
            dst[index].s = src[index].s;
        })");
}

// Test with an unsized array SSBO.
TEST_P(ComputeCopyStorageBufferTests, DISABLED_UnsizedArray) {
    // TODO(kainino@chromium.org): On OpenGL, compilation fails but the test passes. Why?
    // TODO(kainino@chromium.org): On Metal, compilation fails and test crashes.
    BasicTest(R"(
        #version 450
        #define kInstances 4
        struct S {
            uvec2 a, b;  // kUintsPerInstance = 4
        };
        layout(std140, set = 0, binding = 0) buffer Src { S s; } src[];
        layout(std140, set = 0, binding = 1) buffer Dst { S s; } dst[];
        void main() {
            uint index = gl_GlobalInvocationID.x;
            if (index >= kInstances) { return; }
            dst[index].s = src[index].s;
        })");
}

NXT_INSTANTIATE_TEST(ComputeCopyStorageBufferTests,
                     D3D12Backend,
                     MetalBackend,
                     OpenGLBackend,
                     VulkanBackend)
