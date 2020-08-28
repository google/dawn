// Copyright 2020 The Tint Authors.
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

#include "src/reader/spirv/parser_impl.h"

#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

TEST_F(SpvParserTest, Impl_Uint32VecEmpty) {
  std::vector<uint32_t> data;
  auto* p = parser(data);
  EXPECT_FALSE(p->Parse());
  // TODO(dneto): What message?
}

TEST_F(SpvParserTest, Impl_InvalidModuleFails) {
  auto invalid_spv = test::Assemble("%ty = OpTypeInt 3 0");
  auto* p = parser(invalid_spv);
  EXPECT_FALSE(p->Parse());
  EXPECT_THAT(
      p->error(),
      HasSubstr("TypeInt cannot appear before the memory model instruction"));
  EXPECT_THAT(p->error(), HasSubstr("OpTypeInt 3 0"));
}

TEST_F(SpvParserTest, Impl_GenericVulkanShader_SimpleMemoryModel) {
  auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)");
  auto* p = parser(spv);
  EXPECT_TRUE(p->Parse());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, Impl_GenericVulkanShader_GLSL450MemoryModel) {
  auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)");
  auto* p = parser(spv);
  EXPECT_TRUE(p->Parse());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, Impl_GenericVulkanShader_VulkanMemoryModel) {
  auto spv = test::Assemble(R"(
  OpCapability Shader
  OpCapability VulkanMemoryModelKHR
  OpExtension "SPV_KHR_vulkan_memory_model"
  OpMemoryModel Logical VulkanKHR
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)");
  auto* p = parser(spv);
  EXPECT_TRUE(p->Parse());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, Impl_OpenCLKernel_Fails) {
  auto spv = test::Assemble(R"(
  OpCapability Kernel
  OpCapability Addresses
  OpMemoryModel Physical32 OpenCL
  OpEntryPoint Kernel %main "main"
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)");
  auto* p = parser(spv);
  EXPECT_FALSE(p->Parse());
  EXPECT_THAT(p->error(), HasSubstr("Capability Kernel is not allowed"));
}

TEST_F(SpvParserTest, Impl_Source_NoOpLine) {
  auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %5 = OpTypeInt 32 0
  %60 = OpConstantNull %5
  %main = OpFunction %void None %voidfn
  %1 = OpLabel
  OpReturn
  OpFunctionEnd
)");
  auto* p = parser(spv);
  EXPECT_TRUE(p->Parse());
  EXPECT_TRUE(p->error().empty());
  // Use instruction counting.
  auto s5 = p->GetSourceForResultIdForTest(5);
  EXPECT_EQ(7u, s5.line);
  EXPECT_EQ(0u, s5.column);
  auto s60 = p->GetSourceForResultIdForTest(60);
  EXPECT_EQ(8u, s60.line);
  EXPECT_EQ(0u, s60.column);
  auto s1 = p->GetSourceForResultIdForTest(1);
  EXPECT_EQ(10u, s1.line);
  EXPECT_EQ(0u, s1.column);
}

TEST_F(SpvParserTest, Impl_Source_WithOpLine_WithOpNoLine) {
  auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %15 = OpString "myfile"
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  OpLine %15 42 53
  %5 = OpTypeInt 32 0
  %60 = OpConstantNull %5
  OpNoLine
  %main = OpFunction %void None %voidfn
  %1 = OpLabel
  OpReturn
  OpFunctionEnd
)");
  auto* p = parser(spv);
  EXPECT_TRUE(p->Parse());
  EXPECT_TRUE(p->error().empty());
  // Use the information from the OpLine that is still in scope.
  auto s5 = p->GetSourceForResultIdForTest(5);
  EXPECT_EQ(42u, s5.line);
  EXPECT_EQ(53u, s5.column);
  auto s60 = p->GetSourceForResultIdForTest(60);
  EXPECT_EQ(42u, s60.line);
  EXPECT_EQ(53u, s60.column);
  // After OpNoLine, revert back to instruction counting.
  auto s1 = p->GetSourceForResultIdForTest(1);
  EXPECT_EQ(13u, s1.line);
  EXPECT_EQ(0u, s1.column);
}

TEST_F(SpvParserTest, Impl_Source_InvalidId) {
  auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %15 = OpString "myfile"
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %1 = OpLabel
  OpReturn
  OpFunctionEnd
)");
  auto* p = parser(spv);
  EXPECT_TRUE(p->Parse());
  EXPECT_TRUE(p->error().empty());
  auto s99 = p->GetSourceForResultIdForTest(99);
  EXPECT_EQ(0u, s99.line);
  EXPECT_EQ(0u, s99.column);
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
