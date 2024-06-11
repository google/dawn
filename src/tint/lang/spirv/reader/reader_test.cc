// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/reader.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/reader/common/helper_test.h"

namespace tint::spirv::reader {
namespace {

class SpirvReaderTest : public testing::Test {
  protected:
    /// Run the reader on a SPIR-V module and return the Tint IR or an error string.
    /// @param spirv_asm the SPIR-V assembly to read
    /// @returns the disassembled Tint IR or an error
    Result<std::string> Run(std::string spirv_asm) {
        // Assemble the SPIR-V input.
        auto binary = Assemble(spirv_asm);
        if (binary != Success) {
            return binary.Failure();
        }

        // Read the SPIR-V to produce a core IR module.
        auto ir = ReadIR(binary.Get());
        if (ir != Success) {
            return ir.Failure();
        }

        // Validate the IR module against the core dialect.
        auto validated = core::ir::Validate(ir.Get());
        if (validated != Success) {
            return validated.Failure();
        }

        // Return the disassembled IR module.
        return "\n" + core::ir::Disassembler(ir.Get()).Plain();
    }
};

TEST_F(SpirvReaderTest, UnsupportedExtension) {
    auto got = Run(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_variable_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)");
    ASSERT_NE(got, Success);
    EXPECT_EQ(got.Failure().reason.Str(),
              "error: SPIR-V extension 'SPV_KHR_variable_pointers' is not supported");
}

TEST_F(SpirvReaderTest, Load_VectorComponent) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
    %u32_ptr = OpTypePointer Function %u32
  %vec4u_ptr = OpTypePointer Function %vec4u
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %vec4u_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_2
       %load = OpLoad %u32 %access
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var
    %3:u32 = load_vector_element %2, 2u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Store_VectorComponent) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %u32_42 = OpConstant %u32 42
      %vec4u = OpTypeVector %u32 4
    %u32_ptr = OpTypePointer Function %u32
  %vec4u_ptr = OpTypePointer Function %vec4u
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %vec4u_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_2
               OpStore %access %u32_42
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var
    store_vector_element %2, 2u, 42u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ShaderInputs) {
    auto got = Run(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %coord %colors
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %coord BuiltIn FragCoord
               OpDecorate %colors Location 1
               OpMemberDecorate %str 1 NoPerspective
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void
        %str = OpTypeStruct %vec4f %vec4f
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1

%_ptr_Input_vec4f = OpTypePointer Input %vec4f
  %_ptr_Input_str = OpTypePointer Input %str
      %coord = OpVariable %_ptr_Input_vec4f Input
     %colors = OpVariable %_ptr_Input_str Input

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
   %access_a = OpAccessChain %_ptr_Input_vec4f %colors %u32_0
   %access_b = OpAccessChain %_ptr_Input_vec4f %colors %u32_1
          %a = OpLoad %vec4f %access_a
          %b = OpLoad %vec4f %access_b
          %c = OpLoad %vec4f %coord
        %mul = OpFMul %vec4f %a %b
        %add = OpFAdd %vec4f %mul %c
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
tint_symbol_2 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @location(1)
  tint_symbol_1:vec4<f32> @offset(16), @location(2), @interpolate(linear, center)
}

%main = @fragment func(%2:vec4<f32> [@position], %3:tint_symbol_2):void {
  $B1: {
    %4:vec4<f32> = access %3, 0u
    %5:vec4<f32> = access %3, 1u
    %6:vec4<f32> = mul %4, %5
    %7:vec4<f32> = add %6, %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ShaderOutputs) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %depth %colors
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main DepthReplacing
               OpDecorate %depth BuiltIn FragDepth
               OpDecorate %colors Location 1
               OpMemberDecorate %str 1 NoPerspective
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void
        %str = OpTypeStruct %vec4f %vec4f
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1
     %f32_42 = OpConstant %f32 42.0
     %f32_n1 = OpConstant %f32 -1.0
   %f32_v4_a = OpConstantComposite %vec4f %f32_42 %f32_42 %f32_42 %f32_n1
   %f32_v4_b = OpConstantComposite %vec4f %f32_n1 %f32_n1 %f32_n1 %f32_42

%_ptr_Output_f32 = OpTypePointer Output %f32
%_ptr_Output_vec4f = OpTypePointer Output %vec4f
  %_ptr_Output_str = OpTypePointer Output %str
      %depth = OpVariable %_ptr_Output_f32 Output
     %colors = OpVariable %_ptr_Output_str Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
   %access_a = OpAccessChain %_ptr_Output_vec4f %colors %u32_0
   %access_b = OpAccessChain %_ptr_Output_vec4f %colors %u32_1
               OpStore %access_a %f32_v4_a
               OpStore %access_b %f32_v4_b
               OpStore %depth %f32_42
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
tint_symbol_2 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0)
  tint_symbol_1:vec4<f32> @offset(16)
}

tint_symbol_4 = struct @align(16) {
  tint_symbol_3:f32 @offset(0), @builtin(frag_depth)
  tint_symbol:vec4<f32> @offset(16), @location(1)
  tint_symbol_1:vec4<f32> @offset(32), @location(2), @interpolate(linear, center)
}

$B1: {  # root
  %1:ptr<private, f32, read_write> = var
  %2:ptr<private, tint_symbol_2, read_write> = var
}

%main_inner = func():void {
  $B2: {
    %4:ptr<private, vec4<f32>, read_write> = access %2, 0u
    %5:ptr<private, vec4<f32>, read_write> = access %2, 1u
    store %4, vec4<f32>(42.0f, 42.0f, 42.0f, -1.0f)
    store %5, vec4<f32>(-1.0f, -1.0f, -1.0f, 42.0f)
    store %1, 42.0f
    ret
  }
}
%main = @fragment func():tint_symbol_4 {
  $B3: {
    %7:void = call %main_inner
    %8:f32 = load %1
    %9:ptr<private, vec4<f32>, read_write> = access %2, 0u
    %10:vec4<f32> = load %9
    %11:ptr<private, vec4<f32>, read_write> = access %2, 1u
    %12:vec4<f32> = load %11
    %13:tint_symbol_4 = construct %8, %10, %12
    ret %13
  }
}
)");
}

TEST_F(SpirvReaderTest, SampleMask) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %mask_in %mask_out
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %mask_in BuiltIn SampleMask
               OpDecorate %mask_out BuiltIn SampleMask
       %void = OpTypeVoid
    %fn_type = OpTypeFunction %void
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1
    %arr_u32 = OpTypeArray %u32 %u32_1

%_ptr_Input_u32 = OpTypePointer Input %u32
%_ptr_Input_arr_u32 = OpTypePointer Input %arr_u32
%_ptr_Output_u32 = OpTypePointer Output %u32
%_ptr_Output_arr_u32 = OpTypePointer Output %arr_u32
    %mask_in = OpVariable %_ptr_Input_arr_u32 Input
   %mask_out = OpVariable %_ptr_Output_arr_u32 Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
  %mask_in_0 = OpAccessChain %_ptr_Input_u32 %mask_in %u32_0
%mask_in_val = OpLoad %u32 %mask_in_0
   %plus_one = OpIAdd %u32 %mask_in_val %u32_1
 %mask_out_0 = OpAccessChain %_ptr_Output_u32 %mask_out %u32_0
               OpStore %mask_out_0 %plus_one
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
$B1: {  # root
  %1:ptr<private, array<u32, 1>, read_write> = var
}

%main_inner = func(%3:array<u32, 1>):void {
  $B2: {
    %4:u32 = access %3, 0u
    %5:u32 = add %4, 1u
    %6:ptr<private, u32, read_write> = access %1, 0u
    store %6, %5
    ret
  }
}
%main = @fragment func(%8:u32 [@sample_mask]):u32 [@sample_mask] {
  $B3: {
    %9:array<u32, 1> = construct %8
    %10:void = call %main_inner, %9
    %11:array<u32, 1> = load %1
    %12:u32 = access %11, 0u
    ret %12
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
