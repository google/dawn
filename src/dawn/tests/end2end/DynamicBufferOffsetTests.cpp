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

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

constexpr uint32_t kRTSize = 400;
constexpr uint32_t kBindingSize = 8;

class DynamicBufferOffsetTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        mMinUniformBufferOffsetAlignment =
            GetSupportedLimits().limits.minUniformBufferOffsetAlignment;

        // Mix up dynamic and non dynamic resources in one bind group and using not continuous
        // binding number to cover more cases.
        std::vector<uint32_t> uniformData(mMinUniformBufferOffsetAlignment / sizeof(uint32_t) + 2);
        uniformData[0] = 1;
        uniformData[1] = 2;

        mUniformBuffers[0] = utils::CreateBufferFromData(device, uniformData.data(),
                                                         sizeof(uint32_t) * uniformData.size(),
                                                         wgpu::BufferUsage::Uniform);

        uniformData[uniformData.size() - 2] = 5;
        uniformData[uniformData.size() - 1] = 6;

        // Dynamic uniform buffer
        mUniformBuffers[1] = utils::CreateBufferFromData(device, uniformData.data(),
                                                         sizeof(uint32_t) * uniformData.size(),
                                                         wgpu::BufferUsage::Uniform);

        wgpu::BufferDescriptor storageBufferDescriptor;
        storageBufferDescriptor.size = sizeof(uint32_t) * uniformData.size();
        storageBufferDescriptor.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;

        mStorageBuffers[0] = device.CreateBuffer(&storageBufferDescriptor);

        // Dynamic storage buffer
        mStorageBuffers[1] = device.CreateBuffer(&storageBufferDescriptor);

        // Default bind group layout
        mBindGroupLayouts[0] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform},
                     {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Storage},
                     {3, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform, true},
                     {4, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Storage, true}});

        // Default bind group
        mBindGroups[0] = utils::MakeBindGroup(device, mBindGroupLayouts[0],
                                              {{0, mUniformBuffers[0], 0, kBindingSize},
                                               {1, mStorageBuffers[0], 0, kBindingSize},
                                               {3, mUniformBuffers[1], 0, kBindingSize},
                                               {4, mStorageBuffers[1], 0, kBindingSize}});

        // Extra uniform buffer for inheriting test
        mUniformBuffers[2] = utils::CreateBufferFromData(device, uniformData.data(),
                                                         sizeof(uint32_t) * uniformData.size(),
                                                         wgpu::BufferUsage::Uniform);

        // Bind group layout for inheriting test
        mBindGroupLayouts[1] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform}});

        // Bind group for inheriting test
        mBindGroups[1] = utils::MakeBindGroup(device, mBindGroupLayouts[1],
                                              {{0, mUniformBuffers[2], 0, kBindingSize}});
    }
    // Create objects to use as resources inside test bind groups.

    uint32_t mMinUniformBufferOffsetAlignment;
    wgpu::BindGroup mBindGroups[2];
    wgpu::BindGroupLayout mBindGroupLayouts[2];
    wgpu::Buffer mUniformBuffers[3];
    wgpu::Buffer mStorageBuffers[2];
    wgpu::Texture mColorAttachment;

    wgpu::RenderPipeline CreateRenderPipeline(bool isInheritedPipeline = false) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @stage(vertex)
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                var pos = array<vec2<f32>, 3>(
                    vec2<f32>(-1.0, 0.0),
                    vec2<f32>(-1.0, 1.0),
                    vec2<f32>( 0.0, 1.0));
                return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        // Construct fragment shader source
        std::ostringstream fs;
        std::string multipleNumber = isInheritedPipeline ? "2" : "1";
        fs << R"(
            struct Buf {
                value : vec2<u32>
            }

            @group(0) @binding(0) var<uniform> uBufferNotDynamic : Buf;
            @group(0) @binding(1) var<storage, read_write> sBufferNotDynamic : Buf;
            @group(0) @binding(3) var<uniform> uBuffer : Buf;
            @group(0) @binding(4) var<storage, read_write> sBuffer : Buf;
        )";

        if (isInheritedPipeline) {
            fs << R"(
                @group(1) @binding(0) var<uniform> paddingBlock : Buf;
            )";
        }

        fs << "let multipleNumber : u32 = " << multipleNumber << "u;\n";
        fs << R"(
            @stage(fragment) fn main() -> @location(0) vec4<f32> {
                sBufferNotDynamic.value = uBufferNotDynamic.value.xy;
                sBuffer.value = vec2<u32>(multipleNumber, multipleNumber) * (uBuffer.value.xy + uBufferNotDynamic.value.xy);
                return vec4<f32>(f32(uBuffer.value.x) / 255.0, f32(uBuffer.value.y) / 255.0,
                                      1.0, 1.0);
            }
        )";

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fs.str().c_str());

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        if (isInheritedPipeline) {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
        } else {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        }
        pipelineLayoutDescriptor.bindGroupLayouts = mBindGroupLayouts;
        pipelineDescriptor.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateComputePipeline(bool isInheritedPipeline = false) {
        // Construct compute shader source
        std::ostringstream cs;
        std::string multipleNumber = isInheritedPipeline ? "2" : "1";
        cs << R"(
            struct Buf {
                value : vec2<u32>
            }

            @group(0) @binding(0) var<uniform> uBufferNotDynamic : Buf;
            @group(0) @binding(1) var<storage, read_write> sBufferNotDynamic : Buf;
            @group(0) @binding(3) var<uniform> uBuffer : Buf;
            @group(0) @binding(4) var<storage, read_write> sBuffer : Buf;
        )";

        if (isInheritedPipeline) {
            cs << R"(
                @group(1) @binding(0) var<uniform> paddingBlock : Buf;
            )";
        }

        cs << "let multipleNumber : u32 = " << multipleNumber << "u;\n";
        cs << R"(
            @stage(compute) @workgroup_size(1) fn main() {
                sBufferNotDynamic.value = uBufferNotDynamic.value.xy;
                sBuffer.value = vec2<u32>(multipleNumber, multipleNumber) * (uBuffer.value.xy + uBufferNotDynamic.value.xy);
            }
        )";

        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, cs.str().c_str());

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = csModule;
        csDesc.compute.entryPoint = "main";

        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        if (isInheritedPipeline) {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
        } else {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        }
        pipelineLayoutDescriptor.bindGroupLayouts = mBindGroupLayouts;
        csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        return device.CreateComputePipeline(&csDesc);
    }
};

// Dynamic offsets are all zero and no effect to result.
TEST_P(DynamicBufferOffsetTests, BasicRenderPipeline) {
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets = {0, 0};
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Have non-zero dynamic offsets.
TEST_P(DynamicBufferOffsetTests, SetDynamicOffsetsRenderPipeline) {
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Dynamic offsets are all zero and no effect to result.
TEST_P(DynamicBufferOffsetTests, BasicComputePipeline) {
    wgpu::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint32_t, 2> offsets = {0, 0};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Have non-zero dynamic offsets.
TEST_P(DynamicBufferOffsetTests, SetDynamicOffsetsComputePipeline) {
    wgpu::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Test inherit dynamic offsets on render pipeline
TEST_P(DynamicBufferOffsetTests, InheritDynamicOffsetsRenderPipeline) {
    // Using default pipeline and setting dynamic offsets
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    wgpu::RenderPipeline testPipeline = CreateRenderPipeline(true);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.SetPipeline(testPipeline);
    renderPassEncoder.SetBindGroup(1, mBindGroups[1]);
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {12, 16};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Test inherit dynamic offsets on compute pipeline
// TODO(shaobo.yan@intel.com) : Try this test on GTX1080 and cannot reproduce the failure.
// Suspect it is due to dawn doesn't handle sync between two dispatch and disable this case.
// Will double check root cause after got GTX1660.
TEST_P(DynamicBufferOffsetTests, InheritDynamicOffsetsComputePipeline) {
    DAWN_SUPPRESS_TEST_IF(IsWindows());
    wgpu::ComputePipeline pipeline = CreateComputePipeline();
    wgpu::ComputePipeline testPipeline = CreateComputePipeline(true);

    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.SetPipeline(testPipeline);
    computePassEncoder.SetBindGroup(1, mBindGroups[1]);
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {12, 16};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               mMinUniformBufferOffsetAlignment, expectedData.size());
}

// Setting multiple dynamic offsets for the same bindgroup in one render pass.
TEST_P(DynamicBufferOffsetTests, UpdateDynamicOffsetsMultipleTimesRenderPipeline) {
    // Using default pipeline and setting dynamic offsets
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};
    std::array<uint32_t, 2> testOffsets = {0, 0};

    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], testOffsets.size(), testOffsets.data());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Setting multiple dynamic offsets for the same bindgroup in one compute pass.
TEST_P(DynamicBufferOffsetTests, UpdateDynamicOffsetsMultipleTimesComputePipeline) {
    wgpu::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint32_t, 2> offsets = {mMinUniformBufferOffsetAlignment,
                                       mMinUniformBufferOffsetAlignment};
    std::array<uint32_t, 2> testOffsets = {0, 0};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], testOffsets.size(), testOffsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

namespace {
using ReadBufferUsage = wgpu::BufferUsage;
using OOBRead = bool;
using OOBWrite = bool;

DAWN_TEST_PARAM_STRUCT(ClampedOOBDynamicBufferOffsetParams, ReadBufferUsage, OOBRead, OOBWrite);
}  // anonymous namespace

class ClampedOOBDynamicBufferOffsetTests
    : public DawnTestWithParams<ClampedOOBDynamicBufferOffsetParams> {};

// Test robust buffer access behavior for out of bounds accesses to dynamic buffer bindings.
TEST_P(ClampedOOBDynamicBufferOffsetTests, CheckOOBAccess) {
    static constexpr uint32_t kArrayLength = 10u;

    // Out-of-bounds access will start halfway into the array and index off the end.
    static constexpr uint32_t kOOBOffset = kArrayLength / 2;

    wgpu::BufferBindingType sourceBindingType;
    switch (GetParam().mReadBufferUsage) {
        case wgpu::BufferUsage::Uniform:
            sourceBindingType = wgpu::BufferBindingType::Uniform;
            break;
        case wgpu::BufferUsage::Storage:
            sourceBindingType = wgpu::BufferBindingType::ReadOnlyStorage;
            break;
        default:
            UNREACHABLE();
    }
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, sourceBindingType, true},
                 {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage, true}});
    wgpu::PipelineLayout layout = utils::MakeBasicPipelineLayout(device, &bgl);

    wgpu::ComputePipeline pipeline;
    {
        std::ostringstream shader;
        shader << "let kArrayLength: u32 = " << kArrayLength << "u;\n";
        if (GetParam().mOOBRead) {
            shader << "let kReadOffset: u32 = " << kOOBOffset << "u;\n";
        } else {
            shader << "let kReadOffset: u32 = 0u;\n";
        }

        if (GetParam().mOOBWrite) {
            shader << "let kWriteOffset: u32 = " << kOOBOffset << "u;\n";
        } else {
            shader << "let kWriteOffset: u32 = 0u;\n";
        }
        switch (GetParam().mReadBufferUsage) {
            case wgpu::BufferUsage::Uniform:
                shader << R"(
                    struct Src {
                        values : array<vec4<u32>, kArrayLength>
                    }
                    @group(0) @binding(0) var<uniform> src : Src;
                )";
                break;
            case wgpu::BufferUsage::Storage:
                shader << R"(
                    struct Src {
                        values : array<vec4<u32>>
                    }
                    @group(0) @binding(0) var<storage, read> src : Src;
                )";
                break;
            default:
                UNREACHABLE();
        }

        shader << R"(
            struct Dst {
                values : array<vec4<u32>>
            }
            @group(0) @binding(1) var<storage, read_write> dst : Dst;
        )";
        shader << R"(
            @stage(compute) @workgroup_size(1) fn main() {
                for (var i: u32 = 0u; i < kArrayLength; i = i + 1u) {
                    dst.values[i + kWriteOffset] = src.values[i + kReadOffset];
                }
            }
        )";
        wgpu::ComputePipelineDescriptor pipelineDesc;
        pipelineDesc.layout = layout;
        pipelineDesc.compute.module = utils::CreateShaderModule(device, shader.str().c_str());
        pipelineDesc.compute.entryPoint = "main";
        pipeline = device.CreateComputePipeline(&pipelineDesc);
    }

    uint32_t minUniformBufferOffsetAlignment =
        GetSupportedLimits().limits.minUniformBufferOffsetAlignment;
    uint32_t minStorageBufferOffsetAlignment =
        GetSupportedLimits().limits.minStorageBufferOffsetAlignment;

    uint32_t arrayByteLength = kArrayLength * 4 * sizeof(uint32_t);

    uint32_t uniformBufferOffset = Align(arrayByteLength, minUniformBufferOffsetAlignment);
    uint32_t storageBufferOffset = Align(arrayByteLength, minStorageBufferOffsetAlignment);

    // Enough space to bind at a dynamic offset.
    uint32_t uniformBufferSize = uniformBufferOffset + arrayByteLength;
    uint32_t storageBufferSize = storageBufferOffset + arrayByteLength;

    // Buffers are padded so we can check that bytes after the bound range are not changed.
    static constexpr uint32_t kEndPadding = 16;

    uint64_t srcBufferSize;
    uint32_t srcBufferByteOffset;
    uint32_t dstBufferByteOffset = storageBufferOffset;
    uint64_t dstBufferSize = storageBufferSize + kEndPadding;
    switch (GetParam().mReadBufferUsage) {
        case wgpu::BufferUsage::Uniform:
            srcBufferSize = uniformBufferSize + kEndPadding;
            srcBufferByteOffset = uniformBufferOffset;
            break;
        case wgpu::BufferUsage::Storage:
            srcBufferSize = storageBufferSize + kEndPadding;
            srcBufferByteOffset = storageBufferOffset;
            break;
        default:
            UNREACHABLE();
    }

    std::vector<uint32_t> srcData(srcBufferSize / sizeof(uint32_t));
    std::vector<uint32_t> expectedDst(dstBufferSize / sizeof(uint32_t));

    // Fill the src buffer with 0, 1, 2, ...
    std::iota(srcData.begin(), srcData.end(), 0);
    wgpu::Buffer src = utils::CreateBufferFromData(device, &srcData[0], srcBufferSize,
                                                   GetParam().mReadBufferUsage);

    // Fill the dst buffer with 0xFF.
    memset(expectedDst.data(), 0xFF, dstBufferSize);
    wgpu::Buffer dst =
        utils::CreateBufferFromData(device, &expectedDst[0], dstBufferSize,
                                    wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    // Produce expected data assuming the implementation performs clamping.
    for (uint32_t i = 0; i < kArrayLength; ++i) {
        uint32_t readIndex = GetParam().mOOBRead ? std::min(kOOBOffset + i, kArrayLength - 1) : i;
        uint32_t writeIndex = GetParam().mOOBWrite ? std::min(kOOBOffset + i, kArrayLength - 1) : i;

        for (uint32_t c = 0; c < 4; ++c) {
            uint32_t value = srcData[srcBufferByteOffset / 4 + 4 * readIndex + c];
            expectedDst[dstBufferByteOffset / 4 + 4 * writeIndex + c] = value;
        }
    }

    std::array<uint32_t, 2> dynamicOffsets = {srcBufferByteOffset, dstBufferByteOffset};

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl,
                                                     {
                                                         {0, src, 0, arrayByteLength},
                                                         {1, dst, 0, arrayByteLength},
                                                     });

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, bindGroup, dynamicOffsets.size(), dynamicOffsets.data());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expectedDst.data(), dst, 0, dstBufferSize / sizeof(uint32_t));
}

DAWN_INSTANTIATE_TEST(DynamicBufferOffsetTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Only instantiate on D3D12 / Metal where we are sure of the robustness implementation.
// Tint injects clamping in the shader. OpenGL(ES) / Vulkan robustness is less constrained.
DAWN_INSTANTIATE_TEST_P(ClampedOOBDynamicBufferOffsetTests,
                        {D3D12Backend(), MetalBackend()},
                        {wgpu::BufferUsage::Uniform, wgpu::BufferUsage::Storage},
                        {false, true},
                        {false, true});
