// Copyright 2022 The Dawn & Tint Authors
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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class Packed4x8IntegerDotProductTests : public DawnTest {};

TEST_P(Packed4x8IntegerDotProductTests, Dot4x8Packed) {
    // TODO(dawn:1704): investigate why the creation of compute pipeline with dot4{U|I}8Packed()
    // fails on Pixel 4
    DAWN_SUPPRESS_TEST_IF(IsQualcomm());

    const char* computeShader = R"(
        struct Buf {
            data1 : i32,
            data2 : u32,
            data3 : i32,
            data4 : u32,
        }
        @group(0) @binding(0) var<storage, read_write> buf : Buf;
        struct InputData {
            a : u32,
            b : u32,
            c : u32,
        }
        @group(0) @binding(1) var<storage, read_write> inputBuf : InputData;

        @compute @workgroup_size(1)
        fn main() {
            let a = inputBuf.a;
            let b = inputBuf.b;
            let c = inputBuf.c;
            buf.data1 = dot4I8Packed(a, b);
            buf.data2 = dot4U8Packed(a, b);
            buf.data3 = dot4I8Packed(a, c);
            buf.data4 = dot4U8Packed(a, c);
        }
)";

    ASSERT_TRUE(instance.HasWGSLLanguageFeature(wgpu::WGSLFeatureName::Packed4x8IntegerDotProduct));

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4 * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::Buffer inputBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
        {0xFFFEFDFCu, 0xFBFAF9F8u, 0x01020304u});

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, bufferOut},
                                                         {1, inputBuffer},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected[] = {70, 252998, static_cast<uint32_t>(-30), 2530};
    EXPECT_BUFFER_U32_RANGE_EQ(expected, bufferOut, 0, 4);
}

TEST_P(Packed4x8IntegerDotProductTests, Pack4x8) {
    // TODO(dawn:1704): investigate why the creation of compute pipeline with pack4x{U|I}8()
    // fails on Pixel 6
    DAWN_SUPPRESS_TEST_IF(IsQualcomm());

    const char* computeShader = R"(
        struct Buf {
            data1 : u32,
            data2 : u32,
            data3 : u32,
            data4 : u32,
        }
        @group(0) @binding(0) var<storage, read_write> buf : Buf;
        struct InputData {
            a : vec4i,
            b : vec4i,
            c : vec4u,
            d : vec4u,
        }
        @group(0) @binding(1) var<storage, read_write> inputBuf : InputData;

        @compute @workgroup_size(1)
        fn main() {
            buf.data1 = pack4xI8(inputBuf.a);
            buf.data2 = pack4xI8(inputBuf.b);
            buf.data3 = pack4xU8(inputBuf.c);
            buf.data4 = pack4xU8(inputBuf.d);
        }
)";

    ASSERT_TRUE(instance.HasWGSLLanguageFeature(wgpu::WGSLFeatureName::Packed4x8IntegerDotProduct));

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4 * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::Buffer inputBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
        {127, 128, -128, -129, 32767, 32768, -32768, -32769, 1, 2, 3, 4, 0, 254, 255, 65535});

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, bufferOut},
                                                         {1, inputBuffer},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected[] = {0x7f80807f, 0xff0000ff, 0x04030201, 0xfffffe00};
    EXPECT_BUFFER_U32_RANGE_EQ(expected, bufferOut, 0, 4);
}

// DawnTestBase::CreateDeviceImpl always enables allow_unsafe_apis toggle.
DAWN_INSTANTIATE_TEST(Packed4x8IntegerDotProductTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_dxc"}),
                      D3D12Backend({"polyfill_packed_4x8_dot_product"}),
                      MetalBackend(),
                      VulkanBackend(),
                      VulkanBackend({"polyfill_packed_4x8_dot_product"}));

}  // anonymous namespace
}  // namespace dawn
