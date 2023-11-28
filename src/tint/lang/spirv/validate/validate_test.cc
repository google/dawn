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

#include "gtest/gtest.h"

#include "src/tint/lang/spirv/validate/validate.h"

namespace tint::spirv::validate {
namespace {

TEST(SpirvValidateTest, Valid) {
    uint32_t spirv[] = {
        0x07230203, 0x00010600, 0x00070000, 0x00000011, 0x00000000, 0x00020011, 0x00000001,
        0x0003000e, 0x00000000, 0x00000001, 0x0005000f, 0x00000005, 0x00000001, 0x6e69616d,
        0x00000000, 0x00060010, 0x00000001, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
        0x00040005, 0x00000001, 0x6e69616d, 0x00000000, 0x00030005, 0x00000002, 0x0000006d,
        0x00020013, 0x00000003, 0x00030021, 0x00000004, 0x00000003, 0x00030016, 0x00000005,
        0x00000020, 0x00040017, 0x00000006, 0x00000005, 0x00000003, 0x00040018, 0x00000007,
        0x00000006, 0x00000003, 0x00040020, 0x00000008, 0x00000007, 0x00000007, 0x0003002e,
        0x00000007, 0x00000009, 0x00040015, 0x0000000a, 0x00000020, 0x00000001, 0x0004002b,
        0x0000000a, 0x0000000b, 0x00000001, 0x00040020, 0x0000000c, 0x00000007, 0x00000006,
        0x00050036, 0x00000003, 0x00000001, 0x00000000, 0x00000004, 0x000200f8, 0x0000000d,
        0x0005003b, 0x00000008, 0x00000002, 0x00000007, 0x00000009, 0x00050041, 0x0000000c,
        0x0000000e, 0x00000002, 0x0000000b, 0x0004003d, 0x00000006, 0x0000000f, 0x0000000e,
        0x00050051, 0x00000005, 0x00000010, 0x0000000f, 0x00000001, 0x000100fd, 0x00010038,
    };
    auto res = Validate(spirv);
    EXPECT_TRUE(res) << res;
}

TEST(SpirvValidateTest, Invalid) {
    uint32_t spirv[] = {
        0x07230203, 0x00010600, 0x00070000, 0x00000011, 0x00000000, 0x00020011, 0x00000001,
        0x0003000e, 0x00000000, 0x00000001, 0x0005000f, 0x00000005, 0x00000001, 0x6e69616d,
        0x00000000, 0x00060010, 0x00000001, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
        0x00040005, 0x00000001, 0x6e69616d, 0x00000000, 0x00030005, 0x00000002, 0x0000006d,
        0x00020013, 0x00000003, 0x00030021, 0x00000004, 0x00000003, 0x00030016, 0x00000005,
        0x00000020, 0x00040017, 0x00000006, 0x00000005, 0x00000003, 0x00040018, 0x00000007,
        0x00000006, 0x00000003, 0x00040020, 0x00000008, 0x00000007, 0x00000007, 0x0003002e,
        0x00000006, 0x00000009, 0x00040015, 0x0000000a, 0x00000020, 0x00000001, 0x0004002b,
        0x0000000a, 0x0000000b, 0x00000001, 0x00040020, 0x0000000c, 0x00000007, 0x00000006,
        0x00050036, 0x00000003, 0x00000001, 0x00000000, 0x00000004, 0x000200f8, 0x0000000d,
        0x0005003b, 0x00000008, 0x00000002, 0x00000007, 0x00000009, 0x00050041, 0x0000000c,
        0x0000000e, 0x00000002, 0x0000000b, 0x0004003d, 0x00000006, 0x0000000f, 0x0000000e,
        0x00050051, 0x00000005, 0x00000010, 0x0000000f, 0x00000001, 0x000100fd, 0x00010038,
    };
    auto res = Validate(spirv);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure().reason.str(), R"(spirv error: SPIR-V failed validation.

Disassembly:
; SPIR-V
; Version: 1.6
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 17
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
               OpName %m "m"
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
          %9 = OpConstantNull %v3float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Function_v3float = OpTypePointer Function %v3float
       %main = OpFunction %void None %4
         %13 = OpLabel
          %m = OpVariable %_ptr_Function_mat3v3float Function %9
         %14 = OpAccessChain %_ptr_Function_v3float %m %int_1
         %15 = OpLoad %v3float %14
         %16 = OpCompositeExtract %float %15 1
               OpReturn
               OpFunctionEnd

spirv:1:1 error: Initializer type must match the type pointed to by the Result Type
  %m = OpVariable %_ptr_Function_mat3v3float Function %9

)");
}

}  // namespace
}  // namespace tint::spirv::validate
