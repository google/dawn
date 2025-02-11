// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvParserTest, Dot) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %ep_type = OpTypeFunction %void
   %float_50 = OpConstant %float 50
   %float_60 = OpConstant %float 60
%v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
%v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpDot %float %v2float_50_60 %v2float_60_50
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = dot vec2<f32>(50.0f, 60.0f), vec2<f32>(60.0f, 50.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Scalar_UnsignedToUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %uint %uint_10
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_count<u32> 10u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Scalar_UnsignedToSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_10 = OpConstant %uint 10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %int %uint_10
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_count<i32> 10u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Scalar_SignedToUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %uint %int_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_count<u32> 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Scalar_SignedToSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %int %int_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_count<i32> 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Vector_UnsignedToUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %v2_uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2_uint_10_20 = OpConstantComposite %v2_uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %v2_uint %v2_uint_10_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_count<u32> vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Vector_UnsignedToSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2_int = OpTypeVector %int 2
    %v2_uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2_uint_10_20 = OpConstantComposite %v2_uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %v2_int %v2_uint_10_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_count<i32> vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Vector_SignedToUnsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2_int = OpTypeVector %int 2
    %v2_uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
%v2_int_10_20 = OpConstantComposite %v2_int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %v2_uint %v2_int_10_20
               OpReturn
               OpFunctionEnd)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_count<u32> vec2<i32>(10i, 20i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitCount_Vector_SignedToSigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %v2_int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
%v2_int_10_20 = OpConstantComposite %v2_int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitCount %v2_int %v2_int_10_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_count<i32> vec2<i32>(10i, 20i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_Int_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %int %int_30 %int_40 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_insert 30i, 40i, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_Int_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %int %int_30 %int_40 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_insert 30i, 40i, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_IntVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %v2int = OpTypeVector %int 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
%v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
%v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2int %v2int_30_40 %v2int_40_30 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_insert vec2<i32>(30i, 40i), vec2<i32>(40i, 30i), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_IntVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
%v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
%v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2int %v2int_30_40 %v2int_40_30 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_insert vec2<i32>(30i, 40i), vec2<i32>(40i, 30i), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_Uint_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %uint %uint_10 %uint_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_Uint_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %uint %uint_10 %uint_20 %int_30 %int_40
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 30i, 40i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_UintVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
%v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2uint %v2uint_10_20 %v2uint_20_10 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_UintVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
     %int_30 = OpConstant %int 30
     %int_40 = OpConstant %int 40
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
%v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2uint %v2uint_10_20 %v2uint_20_10 %int_30 %int_40
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), 30i, 40i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldInsert_UintVector_SignedOffsetAndUnsignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
%v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldInsert %v2uint %v2uint_10_20 %v2uint_20_10 %int_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u, 20u), vec2<u32>(20u, 10u), 10i, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_Int_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %int %int_10 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_s_extract 10i, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_Int_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %int %int_10 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_s_extract 10i, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_IntVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2int %v2int_10_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i, 20i), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_IntVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
%v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2int %v2int_10_20 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i, 20i), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_IntVector_SignedOffsetAndUnsignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_20 = OpConstant %uint 20
%v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2int %v2int_10_20 %int_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i, 20i), 10i, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_Uint_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %uint %uint_10 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_s_extract 10u, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_Uint_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_10 = OpConstant %uint 10
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %uint %uint_10 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_s_extract 10u, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_UintVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2uint %v2uint_10_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u, 20u), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_UintVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2uint %v2uint_10_20 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u, 20u), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldSExtract_UintVector_SignedOffsetAndUnsignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldSExtract %v2uint %v2uint_10_20 %int_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u, 20u), 10i, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_Uint_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %uint %uint_10 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_u_extract 10u, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_Uint_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
    %uint_10 = OpConstant %uint 10
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %uint %uint_10 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_u_extract 10u, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_UintVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2uint %v2uint_10_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u, 20u), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_UintVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2uint %v2uint_10_20 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u, 20u), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_UintVector_UnsignedOffsetAndSignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2uint %v2uint_10_20 %uint_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u, 20u), 10u, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_Int_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %int_10 = OpConstant %int 10
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %int %int_10 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_u_extract 10i, 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_Int_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %int %int_10 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_u_extract 10i, 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_IntVector_UnsignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
%v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2int %v2int_10_20 %uint_10 %uint_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i, 20i), 10u, 20u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_IntVector_SignedOffsetAndCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
 %v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2int %v2int_10_20 %int_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i, 20i), 10i, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitFieldUExtract_IntVector_UnsignedOffsetAndSignedCount) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
 %v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitFieldUExtract %v2int %v2int_10_20 %uint_10 %int_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i, 20i), 10u, 20i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitReverse_Uint) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitReverse %uint %uint_10
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = reverseBits 10u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitReverse_Int) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
     %int_10 = OpConstant %int 10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitReverse %int %int_10
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = reverseBits 10i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitReverse_UintVector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
    %uint_10 = OpConstant %uint 10
    %uint_20 = OpConstant %uint 20
 %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitReverse %v2uint %v2uint_10_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = reverseBits vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BitReverse_IntVector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
     %int_10 = OpConstant %int 10
     %int_20 = OpConstant %int 20
 %v2int_10_20 = OpConstantComposite %v2int %int_10 %int_20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpBitReverse %v2int %v2int_10_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = reverseBits vec2<i32>(10i, 20i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, All) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
 %v2bool_true_false = OpConstantComposite %v2bool %true %false
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpAll %bool %v2bool_true_false
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = all vec2<bool>(true, false)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Any) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
 %v2bool_true_false = OpConstantComposite %v2bool %true %false
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
          %1 = OpAny %bool %v2bool_true_false
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = any vec2<bool>(true, false)
    ret
  }
}
)");
}

struct BuiltinData {
    const std::string spirv;
    const std::string ir;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.spirv << "," << data.ir;
    return out;
}

using SpirvParser_DerivativeTest = SpirvParserTestWithParam<BuiltinData>;

TEST_P(SpirvParser_DerivativeTest, Scalar) {
    auto& builtin = GetParam();

    EXPECT_IR(R"(
               OpCapability DerivativeControl
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
       %void = OpTypeVoid
      %float = OpTypeFloat 32
   %float_50 = OpConstant %float 50
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
      %entry = OpLabel
         %1 = )" + builtin.spirv +
                  R"( %float %float_50
     OpReturn
     OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:f32 = )" + builtin.ir +
                  R"( 50.0f
    ret
  }
}
)");
}

TEST_P(SpirvParser_DerivativeTest, Vector) {
    auto& builtin = GetParam();

    EXPECT_IR(R"(
                OpCapability DerivativeControl
                OpCapability Shader
                OpMemoryModel Logical GLSL450
                OpEntryPoint Fragment %main "main"
                OpExecutionMode %main OriginUpperLeft
        %void = OpTypeVoid
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
   %float_50 = OpConstant %float 50
   %float_60 = OpConstant %float 60
 %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
     %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
       %entry = OpLabel
          %1 = )" +
                  builtin.spirv + R"( %v2float %v2float_50_60
      OpReturn
      OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:vec2<f32> = )" +
                  builtin.ir + R"( vec2<f32>(50.0f, 60.0f)
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParserTest,
                         SpirvParser_DerivativeTest,
                         testing::Values(BuiltinData{"OpDPdx", "dpdx"},
                                         BuiltinData{"OpDPdy", "dpdy"},
                                         BuiltinData{"OpFwidth", "fwidth"},
                                         BuiltinData{"OpDPdxFine", "dpdxFine"},
                                         BuiltinData{"OpDPdyFine", "dpdyFine"},
                                         BuiltinData{"OpFwidthFine", "fwidthFine"},
                                         BuiltinData{"OpDPdxCoarse", "dpdxCoarse"},
                                         BuiltinData{"OpDPdyCoarse", "dpdyCoarse"},
                                         BuiltinData{"OpFwidthCoarse", "fwidthCoarse"}));

}  // namespace
}  // namespace tint::spirv::reader
