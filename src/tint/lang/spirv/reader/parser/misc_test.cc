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

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvParserTest, Misc_OpNop) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpNop
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
)");
}

TEST_F(SpirvParserTest, Misc_OpUndefInFunction) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_2 = OpConstant %uint 2
%_arr_uint_uint_2 = OpTypeArray %uint %uint_2
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
 %_struct_13 = OpTypeStruct %bool %uint %int %float
    %ep_type = OpTypeFunction %void
         %11 = OpTypeFunction %int
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpUndef %bool
          %2 = OpUndef %uint
          %3 = OpUndef %int
          %4 = OpUndef %float
          %5 = OpUndef %_arr_uint_uint_2
          %6 = OpUndef %mat2v2float
          %7 = OpUndef %v2uint
          %8 = OpUndef %v2int
          %9 = OpUndef %v2float
         %10 = OpUndef %_struct_13
               OpReturn
               OpFunctionEnd
          %m = OpFunction %int None %11
         %12 = OpLabel
         %13 = OpUndef %int
               OpReturnValue %13
               OpFunctionEnd

)",
              R"(
tint_symbol_4 = struct @align(4) {
  tint_symbol:bool @offset(0)
  tint_symbol_1:u32 @offset(4)
  tint_symbol_2:i32 @offset(8)
  tint_symbol_3:f32 @offset(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
%2 = func():i32 {
  $B2: {
    ret 0i
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
