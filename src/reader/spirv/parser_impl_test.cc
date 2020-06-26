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

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
