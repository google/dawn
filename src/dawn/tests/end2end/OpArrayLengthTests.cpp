// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cstdint>
#include <string>
#include <vector>

#include "src/dawn/common/Math.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"
#include "src/utils/assert.h"

namespace dawn {
namespace {

class OpArrayLengthTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        // maxStorageBuffersInVertexStage
        supported.UnlinkedCopyTo(&required);
    }

    void SetUp() override {
        DawnTest::SetUp();

        // Create buffers of various size to check the length() implementation
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = 4;
        bufferDesc.usage = wgpu::BufferUsage::Storage;
        mStorageBuffer4 = device.CreateBuffer(&bufferDesc);

        bufferDesc.size = 256;
        mStorageBuffer256 = device.CreateBuffer(&bufferDesc);

        bufferDesc.size = 512 + 256;
        mStorageBuffer512 = device.CreateBuffer(&bufferDesc);

        // Common shader code to use these buffers in shaders, assuming they are in bindgroup index
        // 0.
        mShaderInterface = R"(
            struct DataBuffer {
                data : array<f32>
            }

            // The length should be 1 because the buffer is 4-byte long.
            @group(0) @binding(0) var<storage, read> buffer1 : DataBuffer;

            // The length should be 64 because the buffer is 256 bytes long.
            @group(0) @binding(1) var<storage, read> buffer2 : DataBuffer;

            // The length should be (512 - 16*4) / 8 = 56 because the buffer is 512 bytes long
            // and the structure is 8 bytes big.
            struct Buffer3Data {
                a : f32,
                b : i32,
            }

            struct Buffer3 {
                @size(64) garbage : mat4x4<f32>,
                data : array<Buffer3Data>,
            }
            @group(0) @binding(2) var<storage, read> buffer3 : Buffer3;
        )";

        // See comments in the shader for an explanation of these values
        mExpectedLengths = {1, 64, 56};
    }

    wgpu::BindGroupLayout MakeBindGroupLayout(wgpu::ShaderStage stages) {
        // Put them all in a bind group for tests to bind them easily.
        return utils::MakeBindGroupLayout(device,
                                          {{0, stages, wgpu::BufferBindingType::ReadOnlyStorage},
                                           {1, stages, wgpu::BufferBindingType::ReadOnlyStorage},
                                           {2, stages, wgpu::BufferBindingType::ReadOnlyStorage}});
    }

    wgpu::BindGroup MakeBindGroup(wgpu::BindGroupLayout bindGroupLayout) {
        return utils::MakeBindGroup(device, bindGroupLayout,
                                    {
                                        {0, mStorageBuffer4, 0, 4},
                                        {1, mStorageBuffer256, 0, wgpu::kWholeSize},
                                        {2, mStorageBuffer512, 256, wgpu::kWholeSize},
                                    });
    }

    wgpu::Buffer mStorageBuffer4;
    wgpu::Buffer mStorageBuffer256;
    wgpu::Buffer mStorageBuffer512;

    std::string mShaderInterface;
    std::array<uint32_t, 3> mExpectedLengths;
};

// Test OpArrayLength in the compute stage
TEST_P(OpArrayLengthTest, Compute) {
    // Create a buffer to hold the result sizes and create a bindgroup for it.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    bufferDesc.size = sizeof(uint32_t) * mExpectedLengths.size();
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroupLayout resultLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

    wgpu::BindGroup resultBindGroup =
        utils::MakeBindGroup(device, resultLayout, {{0, resultBuffer, 0, wgpu::kWholeSize}});

    // Create the compute pipeline that stores the length()s in the result buffer.
    wgpu::BindGroupLayout bindGroupLayout = MakeBindGroupLayout(wgpu::ShaderStage::Compute);
    wgpu::BindGroupLayout bgls[] = {bindGroupLayout, resultLayout};
    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 2;
    plDesc.bindGroupLayouts = bgls;
    wgpu::PipelineLayout pl = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, (R"(
        struct ResultBuffer {
            data : array<u32, 3>
        }
        @group(1) @binding(0) var<storage, read_write> result : ResultBuffer;
        )" + mShaderInterface + R"(
        @compute @workgroup_size(1) fn main() {
            result.data[0] = arrayLength(&buffer1.data);
            result.data[1] = arrayLength(&buffer2.data);
            result.data[2] = arrayLength(&buffer3.data);
        })")
                                                                        .c_str());
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);
    wgpu::BindGroup bindGroup = MakeBindGroup(bindGroupLayout);

    // Run a single instance of the compute shader
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.SetBindGroup(1, resultBindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(mExpectedLengths.data(), resultBuffer, 0, 3);
}

// Test OpArrayLength in the fragment stage
TEST_P(OpArrayLengthTest, Fragment) {
    // TODO(crbug.com/408042465): investigate this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM() &&
                          HasToggleEnabled("gl_use_array_length_from_uniform"));

    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 3);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Create the pipeline that computes the length of the buffers and writes it to the only render
    // pass pixel.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, (mShaderInterface + R"(
        @fragment fn main() -> @location(0) vec4f {
            var fragColor : vec4f;
            fragColor.r = f32(arrayLength(&buffer1.data)) / 255.0;
            fragColor.g = f32(arrayLength(&buffer2.data)) / 255.0;
            fragColor.b = f32(arrayLength(&buffer3.data)) / 255.0;
            fragColor.a = 0.0;
            return fragColor;
        })")
                                                                        .c_str());

    wgpu::BindGroupLayout bindGroupLayout = MakeBindGroupLayout(wgpu::ShaderStage::Fragment);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
    descriptor.cTargets[0].format = renderPass.colorFormat;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &bindGroupLayout);
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::BindGroup bindGroup = MakeBindGroup(bindGroupLayout);

    // "Draw" the lengths to the texture.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 expectedColor =
        utils::RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
    EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
}

// Test OpArrayLength in the vertex stage
TEST_P(OpArrayLengthTest, Vertex) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 3);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Create the pipeline that computes the length of the buffers and writes it to the only render
    // pass pixel.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, (mShaderInterface + R"(
        struct VertexOut {
            @location(0) color : vec4f,
            @builtin(position) position : vec4f,
        }

        @vertex fn main() -> VertexOut {
            var output : VertexOut;
            output.color.r = f32(arrayLength(&buffer1.data)) / 255.0;
            output.color.g = f32(arrayLength(&buffer2.data)) / 255.0;
            output.color.b = f32(arrayLength(&buffer3.data)) / 255.0;
            output.color.a = 0.0;

            output.position = vec4f(0.0, 0.0, 0.0, 1.0);
            return output;
        })")
                                                                        .c_str());

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment
        fn main(@location(0) color : vec4f) -> @location(0) vec4f {
            return color;
        })");

    wgpu::BindGroupLayout bindGroupLayout = MakeBindGroupLayout(wgpu::ShaderStage::Vertex);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
    descriptor.cTargets[0].format = renderPass.colorFormat;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &bindGroupLayout);
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::BindGroup bindGroup = MakeBindGroup(bindGroupLayout);

    // "Draw" the lengths to the texture.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 expectedColor =
        utils::RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
    EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(OpArrayLengthTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_use_array_length_from_uniform"}),
                      VulkanBackend(),
                      WebGPUBackend());

enum class TieredLimits {
    No,
    Yes,
};

std::ostream& operator<<(std::ostream& o, TieredLimits tieredLimits) {
    switch (tieredLimits) {
        case TieredLimits::No:
            o << "NoTieredLimits";
            break;
        case TieredLimits::Yes:
            o << "TieredLimits";
            break;
    }
    return o;
}

DAWN_TEST_PARAM_STRUCT(MaxArrayLengthTestParams, TieredLimits);

class MaxArrayLengthTest : public DawnTestWithParams<MaxArrayLengthTestParams> {
  protected:
    bool GetRequireUseTieredLimits() override {
        return GetParam().mTieredLimits == TieredLimits::Yes;
    }

    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }

    void SetUp() override {
        DawnTestWithParams<MaxArrayLengthTestParams>::SetUp();

        // Will fail with 'VK_ERROR_OUT_OF_DEVICE_MEMORY' due to the maxStorageBufferBindingSize
        // portion of the test
        DAWN_SUPPRESS_TEST_IF(IsCompatibilityMode() || IsSwiftshader() || IsANGLESwiftShader() ||
                              IsOpenGLES());

        // Warp runs into memory issues.
        DAWN_SUPPRESS_TEST_IF(IsWARP());

        // TODO(crbug.com/473894293): [Capture] buffer mapping: investigate.
        DAWN_SUPPRESS_TEST_IF(IsCaptureReplayCheckingEnabled());

        // x86 has issues with OpArrayLength.
        DAWN_SUPPRESS_TEST_IF(IsX86());

        // TODO(crbug.com/485946556): Dawn native will report the wrong arrayLength for Apple
        // hardware.
        DAWN_SUPPRESS_TEST_IF(IsApple() && !GetRequireUseTieredLimits());

        auto maxBindingSizeSize = AlignDown(GetSupportedLimits().maxStorageBufferBindingSize, 4);

        // Create buffers of various sizes to check the length() implementation
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = maxBindingSizeSize;
        bufferDesc.usage = wgpu::BufferUsage::Storage;
        mStorageBufferMax = device.CreateBuffer(&bufferDesc);

        mExpectedLength = static_cast<uint32_t>(maxBindingSizeSize / 4);
    }

    wgpu::Buffer mStorageBufferMax;
    uint32_t mExpectedLength;
};

// Test OpArrayLength in the compute stage
TEST_P(MaxArrayLengthTest, Compute) {
    // Create a buffer to hold the result sizes and create a bindgroup for it.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    bufferDesc.size = sizeof(uint32_t);
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroupLayout resultLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

    wgpu::BindGroup resultBindGroup =
        utils::MakeBindGroup(device, resultLayout, {{0, resultBuffer, 0, wgpu::kWholeSize}});

    // Create the compute pipeline that stores the length()s in the result buffer.
    wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});

    wgpu::BindGroupLayout bgls[] = {bindGroupLayout, resultLayout};
    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 2;
    plDesc.bindGroupLayouts = bgls;
    wgpu::PipelineLayout pl = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(1) @binding(0) var<storage, read_write> result : u32;

        struct Buffer {
            data : array<u32>,
        }
        @group(0) @binding(0) var<storage, read> buffer1 : Buffer;

        @compute @workgroup_size(1) fn main() {
            result  = arrayLength(&buffer1.data);
        })");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bindGroupLayout,
                                                     {{0, mStorageBufferMax, 0, wgpu::kWholeSize}});

    // Run a single instance of the compute shader
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.SetBindGroup(1, resultBindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(mExpectedLength, resultBuffer, 0);
}

DAWN_INSTANTIATE_TEST_P(MaxArrayLengthTest,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), OpenGLESBackend({"gl_use_array_length_from_uniform"}),
                         VulkanBackend(), WebGPUBackend()},
                        {TieredLimits::No, TieredLimits::Yes});

class GLArrayLengthOverflowTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }
};

// Test that using more than 96 ShaderStage::None bind group entries
// (which don't count against Dawn's validation limit) don't cause GL
// errors and failed buffer transfers.
TEST_P(GLArrayLengthOverflowTest, VisibilityNoneOverflowsArrayLengthBuffer) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxBindGroups < 4);

    constexpr uint32_t kLargeSize = 512u;
    constexpr uint32_t kSmallSize = 256u;
    constexpr uint32_t kPadPerGroup = 33;

    wgpu::BufferDescriptor bd;
    bd.size = kLargeSize;
    bd.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer largeBuf = device.CreateBuffer(&bd);

    // Get the arrayLength() of the passed-in storage buffer, and store it into
    // the first element of the array.
    wgpu::ComputePipelineDescriptor primeDesc;
    primeDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<storage, read_write> a : array<u32>;
        @compute @workgroup_size(1) fn main() {
            a[0] = arrayLength(&a);
        })");
    wgpu::ComputePipeline primePipeline = device.CreateComputePipeline(&primeDesc);
    wgpu::BindGroupLayout primeBGL = primePipeline.GetBindGroupLayout(0);
    wgpu::BindGroup primeBG = utils::MakeBindGroup(device, primeBGL, {{0, largeBuf}});

    {
        wgpu::CommandEncoder enc = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = enc.BeginComputePass();
        pass.SetPipeline(primePipeline);
        pass.SetBindGroup(0, primeBG);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer cb = enc.Finish();
        queue.Submit(1, &cb);
    }

    bd.size = kSmallSize;
    bd.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer smallBuf = device.CreateBuffer(&bd);

    bd.size = 4;
    bd.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer tinyBuffer = device.CreateBuffer(&bd);

    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

    std::vector<wgpu::BindGroupLayoutEntry> padEntries(kPadPerGroup);
    for (uint32_t i = 0; i < kPadPerGroup; i++) {
        padEntries[i].binding = i;
        padEntries[i].visibility = wgpu::ShaderStage::None;
        padEntries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    }
    wgpu::BindGroupLayoutDescriptor padDesc;
    padDesc.entryCount = padEntries.size();
    padDesc.entries = padEntries.data();
    wgpu::BindGroupLayout bglPad = device.CreateBindGroupLayout(&padDesc);

    wgpu::BindGroupLayout bgls[] = {bgl0, bglPad, bglPad, bglPad};
    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 4;
    plDesc.bindGroupLayouts = bgls;
    wgpu::PipelineLayout manyBindingsPL = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor manyBindingsDesc;
    manyBindingsDesc.layout = manyBindingsPL;
    manyBindingsDesc.compute.module = primeDesc.compute.module;
    wgpu::ComputePipeline manyBindingsPipeline = device.CreateComputePipeline(&manyBindingsDesc);

    wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, smallBuf}});

    std::vector<wgpu::BindGroupEntry> padBinds(kPadPerGroup);
    for (uint32_t i = 0; i < kPadPerGroup; i++) {
        padBinds[i].binding = i;
        padBinds[i].buffer = tinyBuffer;
    }
    wgpu::BindGroupDescriptor bgPadDesc;
    bgPadDesc.layout = bglPad;
    bgPadDesc.entryCount = padBinds.size();
    bgPadDesc.entries = padBinds.data();
    wgpu::BindGroup bgPad = device.CreateBindGroup(&bgPadDesc);

    {
        wgpu::CommandEncoder enc = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = enc.BeginComputePass();
        pass.SetPipeline(manyBindingsPipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bgPad);
        pass.SetBindGroup(2, bgPad);
        pass.SetBindGroup(3, bgPad);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer cb = enc.Finish();
        queue.Submit(1, &cb);
    }

    // Check that the stored arrayLength is the (new) small buffer length
    // and not the (stale) large buffer length.
    EXPECT_BUFFER_U32_EQ(kSmallSize / 4u, smallBuf, 0);
}

DAWN_INSTANTIATE_TEST(GLArrayLengthOverflowTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_use_array_length_from_uniform"}),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
