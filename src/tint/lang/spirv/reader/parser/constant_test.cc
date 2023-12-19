// Copyright 2023 The Dawn & Tint Authors
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

TEST_F(SpirvParserTest, Constant_Bool) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantTrue %bool
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %bool

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %bool
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %void %foo %true
          %2 = OpFunctionCall %void %foo %false
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void -> %b3 {
  %b3 = block {
    %5:void = call %2, true
    %6:void = call %2, true
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_I32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
      %i32_0 = OpConstant %i32 0
      %i32_1 = OpConstant %i32 1
     %i32_n1 = OpConstant %i32 -1
    %i32_max = OpConstant %i32 2147483647
    %i32_min = OpConstant %i32 -2147483648
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %i32

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %i32
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %void %foo %i32_0
          %2 = OpFunctionCall %void %foo %i32_1
          %3 = OpFunctionCall %void %foo %i32_n1
          %4 = OpFunctionCall %void %foo %i32_max
          %5 = OpFunctionCall %void %foo %i32_min
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void -> %b3 {
  %b3 = block {
    %5:void = call %2, 0i
    %6:void = call %2, 1i
    %7:void = call %2, -1i
    %8:void = call %2, 2147483647i
    %9:void = call %2, -2147483648i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Constant_U32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1
    %u32_max = OpConstant %u32 4294967295
    %void_fn = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %u32

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %u32
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

        %bar = OpFunction %void None %void_fn
  %bar_start = OpLabel
          %1 = OpFunctionCall %void %foo %u32_0
          %2 = OpFunctionCall %void %foo %u32_1
          %3 = OpFunctionCall %void %foo %u32_max
               OpReturn
               OpFunctionEnd
)",
              R"(
%4 = func():void -> %b3 {
  %b3 = block {
    %5:void = call %2, 0u
    %6:void = call %2, 1u
    %7:void = call %2, 4294967295u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
