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

#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

/// @returns a SPIR-V assembly segment which assigns debug names
/// to particular IDs.
std::string Names(std::vector<std::string> ids) {
  std::ostringstream outs;
  for (auto& id : ids) {
    outs << "    OpName %" << id << " \"" << id << "\"\n";
  }
  return outs.str();
}

std::string CommonTypes() {
  return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %float_0 = OpConstant %float 0.0
  )";
}

TEST_F(SpvParserTest, EmitFunctions_NoFunctions) {
  auto* p = parser(test::Assemble(CommonTypes()));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, Not(HasSubstr("Function{")));
}

TEST_F(SpvParserTest, EmitFunctions_FunctionWithoutBody) {
  auto* p = parser(test::Assemble(Names({"main"}) + CommonTypes() + R"(
     %main = OpFunction %void None %voidfn
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, Not(HasSubstr("Function{")));
}

TEST_F(SpvParserTest, EmitFunctions_VoidFunctionWithoutParams) {
  auto* p = parser(test::Assemble(Names({"main"}) + CommonTypes() + R"(
     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, HasSubstr(R"(
  Function main -> __void
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_CalleePrecedesCaller) {
  auto* p = parser(
      test::Assemble(Names({"root", "branch", "leaf"}) + CommonTypes() + R"(
     %root = OpFunction %void None %voidfn
     %root_entry = OpLabel
     %branch_result = OpFunctionCall %void %branch
     OpReturn
     OpFunctionEnd

     %branch = OpFunction %void None %voidfn
     %branch_entry = OpLabel
     %leaf_result = OpFunctionCall %void %leaf
     OpReturn
     OpFunctionEnd

     %leaf = OpFunction %void None %voidfn
     %leaf_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, HasSubstr(R"(
  Function leaf -> __void
  ()
  {
  }
  Function branch -> __void
  ()
  {
  }
  Function root -> __void
  ()
  {
  })"));
}

TEST_F(SpvParserTest, EmitFunctions_NonVoidResultType) {
  auto* p = parser(test::Assemble(Names({"ret_float"}) + CommonTypes() + R"(
     %fn_ret_float = OpTypeFunction %float

     %ret_float = OpFunction %float None %fn_ret_float
     %ret_float_entry = OpLabel
     OpReturnValue %float_0
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, HasSubstr(R"(
  Function ret_float -> __f32
  ()
  {
  })"));
}

TEST_F(SpvParserTest, EmitFunctions_MixedParamTypes) {
  auto* p = parser(test::Assemble(Names({"mixed_params", "a", "b", "c"}) +
                                  CommonTypes() + R"(
     %fn_mixed_params = OpTypeFunction %float %uint %float %int

     %mixed_params = OpFunction %void None %fn_mixed_params
     %a = OpFunctionParameter %uint
     %b = OpFunctionParameter %float
     %c = OpFunctionParameter %int
     %mixed_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, HasSubstr(R"(
  Function mixed_params -> __void
  (
    Variable{
      a
      none
      __u32
    }
    Variable{
      b
      none
      __f32
    }
    Variable{
      c
      none
      __i32
    }
  )
  {
  })"));
}

TEST_F(SpvParserTest, EmitFunctions_GenerateParamNames) {
  auto* p = parser(test::Assemble(Names({"mixed_params"}) + CommonTypes() + R"(
     %fn_mixed_params = OpTypeFunction %float %uint %float %int

     %mixed_params = OpFunction %void None %fn_mixed_params
     %14 = OpFunctionParameter %uint
     %15 = OpFunctionParameter %float
     %16 = OpFunctionParameter %int
     %mixed_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, HasSubstr(R"(
  Function mixed_params -> __void
  (
    Variable{
      x_14
      none
      __u32
    }
    Variable{
      x_15
      none
      __f32
    }
    Variable{
      x_16
      none
      __i32
    }
  )
  {
  })"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
