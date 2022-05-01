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

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

class ShaderFloat16Tests : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        mIsShaderFloat16Supported = SupportsFeatures({wgpu::FeatureName::DawnShaderFloat16});
        if (!mIsShaderFloat16Supported) {
            return {};
        }

        return {wgpu::FeatureName::DawnShaderFloat16};
    }

    bool IsShaderFloat16Supported() const { return mIsShaderFloat16Supported; }

    bool mIsShaderFloat16Supported = false;
};

// Test basic 16bit float arithmetic and 16bit storage features.
// TODO(crbug.com/tint/404): Implement float16 in Tint.
TEST_P(ShaderFloat16Tests, DISABLED_Basic16BitFloatFeaturesTest) {
    DAWN_TEST_UNSUPPORTED_IF(!IsShaderFloat16Supported());
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsIntel());  // Flaky crashes. crbug.com/dawn/586

    uint16_t uniformData[] = {Float32ToFloat16(1.23), Float32ToFloat16(0.0)};  // 0.0 is a padding.
    wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
        device, &uniformData, sizeof(uniformData), wgpu::BufferUsage::Uniform);

    uint16_t bufferInData[] = {Float32ToFloat16(2.34), Float32ToFloat16(0.0)};  // 0.0 is a padding.
    wgpu::Buffer bufferIn = utils::CreateBufferFromData(device, &bufferInData, sizeof(bufferInData),
                                                        wgpu::BufferUsage::Storage);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 2 * sizeof(uint16_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    // SPIR-V ASM produced by glslang for the following fragment shader:
    //
    //   #version 450
    //   #extension GL_AMD_gpu_shader_half_float : require
    //
    //   struct S {
    //       float16_t f;
    //       float16_t padding;
    //   };
    //   layout(std140, set = 0, binding = 0) uniform uniformBuf { S c; };
    //   layout(std140, set = 0, binding = 1) readonly buffer bufA { S a; };
    //   layout(std140, set = 0, binding = 2) buffer bufB { S b; };
    //
    //   void main() {
    //       b.f = a.f + c.f;
    //   }

    wgpu::ShaderModule module = utils::CreateShaderModuleFromASM(device, R"(
; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 26
; Schema: 0
               OpCapability Shader
               OpCapability Float16
               OpCapability StorageBuffer16BitAccess
               OpCapability UniformAndStorageBuffer16BitAccess
               OpExtension "SPV_KHR_16bit_storage"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_AMD_gpu_shader_half_float"
               OpName %main "main"
               OpName %S "S"
               OpMemberName %S 0 "f"
               OpMemberName %S 1 "padding"
               OpName %bufB "bufB"
               OpMemberName %bufB 0 "b"
               OpName %_ ""
               OpName %bufA "bufA"
               OpMemberName %bufA 0 "a"
               OpName %__0 ""
               OpName %uniformBuf "uniformBuf"
               OpMemberName %uniformBuf 0 "c"
               OpName %__1 ""
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 2
               OpMemberDecorate %bufB 0 Offset 0
               OpDecorate %bufB BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 2
               OpMemberDecorate %bufA 0 NonWritable
               OpMemberDecorate %bufA 0 Offset 0
               OpDecorate %bufA BufferBlock
               OpDecorate %__0 DescriptorSet 0
               OpDecorate %__0 Binding 1
               OpMemberDecorate %uniformBuf 0 Offset 0
               OpDecorate %uniformBuf Block
               OpDecorate %__1 DescriptorSet 0
               OpDecorate %__1 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %half = OpTypeFloat 16
          %S = OpTypeStruct %half %half
       %bufB = OpTypeStruct %S
%_ptr_Uniform_bufB = OpTypePointer Uniform %bufB
          %_ = OpVariable %_ptr_Uniform_bufB Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %bufA = OpTypeStruct %S
%_ptr_Uniform_bufA = OpTypePointer Uniform %bufA
        %__0 = OpVariable %_ptr_Uniform_bufA Uniform
%_ptr_Uniform_half = OpTypePointer Uniform %half
 %uniformBuf = OpTypeStruct %S
%_ptr_Uniform_uniformBuf = OpTypePointer Uniform %uniformBuf
        %__1 = OpVariable %_ptr_Uniform_uniformBuf Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_Uniform_half %__0 %int_0 %int_0
         %18 = OpLoad %half %17
         %22 = OpAccessChain %_ptr_Uniform_half %__1 %int_0 %int_0
         %23 = OpLoad %half %22
         %24 = OpFAdd %half %18 %23
         %25 = OpAccessChain %_ptr_Uniform_half %_ %int_0 %int_0
               OpStore %25 %24
               OpReturn
               OpFunctionEnd
    )");

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, uniformBuffer, 0, sizeof(uniformData)},
                                                         {1, bufferIn, 0, sizeof(bufferInData)},
                                                         {2, bufferOut},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint16_t expected[] = {Float32ToFloat16(3.57), Float32ToFloat16(0.0)};  // 0.0 is a padding.

    EXPECT_BUFFER_U16_RANGE_EQ(expected, bufferOut, 0, 2);
}

DAWN_INSTANTIATE_TEST(ShaderFloat16Tests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
