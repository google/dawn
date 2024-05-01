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
#include "src/tint/lang/core/ir/disassembly.h"
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
        return "\n" + core::ir::Disassemble(ir.Get()).Plain();
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

}  // namespace
}  // namespace tint::spirv::reader
