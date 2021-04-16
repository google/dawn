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

#include "gmock/gmock.h"
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
  auto p = parser(test::Assemble(CommonTypes()));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, Not(HasSubstr("Function{")));
}

TEST_F(SpvParserTest, EmitFunctions_FunctionWithoutBody) {
  auto p = parser(test::Assemble(Names({"main"}) + CommonTypes() + R"(
     %main = OpFunction %void None %voidfn
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, Not(HasSubstr("Function{")));
}

TEST_F(SpvParserTest, EmitFunctions_Function_EntryPoint_Vertex) {
  std::string input = Names({"main"}) + R"(OpEntryPoint Vertex %main "main"
)" + CommonTypes() + R"(
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

  auto p = parser(test::Assemble(input));
  ASSERT_TRUE(p->BuildAndParseInternalModule());
  ASSERT_TRUE(p->error().empty()) << p->error();
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("main").to_str() +
                                     R"( -> __void
  StageDecoration{vertex}
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_Function_EntryPoint_Fragment) {
  std::string input = Names({"main"}) + R"(OpEntryPoint Fragment %main "main"
)" + CommonTypes() + R"(
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

  auto p = parser(test::Assemble(input));
  ASSERT_TRUE(p->BuildAndParseInternalModule());
  ASSERT_TRUE(p->error().empty()) << p->error();
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("main").to_str() +
                                     R"( -> __void
  StageDecoration{fragment}
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_Function_EntryPoint_GLCompute) {
  std::string input = Names({"main"}) + R"(OpEntryPoint GLCompute %main "main"
)" + CommonTypes() + R"(
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

  auto p = parser(test::Assemble(input));
  ASSERT_TRUE(p->BuildAndParseInternalModule());
  ASSERT_TRUE(p->error().empty()) << p->error();
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("main").to_str() +
                                     R"( -> __void
  StageDecoration{compute}
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_Function_EntryPoint_MultipleEntryPoints) {
  std::string input = Names({"main"}) +
                      R"(OpEntryPoint GLCompute %main "comp_main"
OpEntryPoint Fragment %main "frag_main"
)" + CommonTypes() + R"(
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd)";

  auto p = parser(test::Assemble(input));
  ASSERT_TRUE(p->BuildAndParseInternalModule());
  ASSERT_TRUE(p->error().empty()) << p->error();
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("frag_main").to_str() +
                                     R"( -> __void
  StageDecoration{fragment}
  ()
  {)"));
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("comp_main").to_str() +
                                     R"( -> __void
  StageDecoration{compute}
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_VoidFunctionWithoutParams) {
  auto p = parser(test::Assemble(Names({"main"}) + CommonTypes() + R"(
     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("main").to_str() +
                                     R"( -> __void
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_CalleePrecedesCaller) {
  auto p = parser(test::Assemble(
      Names({"root", "branch", "leaf", "leaf_result", "branch_result"}) +
      CommonTypes() + R"(
     %uintfn = OpTypeFunction %uint
     %uint_0 = OpConstant %uint 0

     %root = OpFunction %void None %voidfn
     %root_entry = OpLabel
     %branch_result = OpFunctionCall %uint %branch
     OpReturn
     OpFunctionEnd

     %branch = OpFunction %uint None %uintfn
     %branch_entry = OpLabel
     %leaf_result = OpFunctionCall %uint %leaf
     OpReturnValue %leaf_result
     OpFunctionEnd

     %leaf = OpFunction %uint None %uintfn
     %leaf_entry = OpLabel
     OpReturnValue %uint_0
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  Program program = p->program();
  const auto program_ast = program.to_str();
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function leaf -> __u32
  ()
  {
    Return{
      {
        ScalarConstructor[not set]{0u}
      }
    }
  }
  Function branch -> __u32
  ()
  {
    VariableDeclStatement{
      VariableConst{
        leaf_result
        none
        __u32
        {
          Call[not set]{
            Identifier[not set]{leaf}
            (
            )
          }
        }
      }
    }
    Return{
      {
        Identifier[not set]{leaf_result}
      }
    }
  }
  Function root -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        branch_result
        none
        __u32
        {
          Call[not set]{
            Identifier[not set]{branch}
            (
            )
          }
        }
      }
    }
    Return{}
  }
})")) << program_ast;
}

TEST_F(SpvParserTest, EmitFunctions_NonVoidResultType) {
  auto p = parser(test::Assemble(Names({"ret_float"}) + CommonTypes() + R"(
     %fn_ret_float = OpTypeFunction %float

     %ret_float = OpFunction %float None %fn_ret_float
     %ret_float_entry = OpLabel
     OpReturnValue %float_0
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  Program program = p->program();
  const auto program_ast = program.to_str();
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function ret_float -> __f32
  ()
  {
    Return{
      {
        ScalarConstructor[not set]{0.000000}
      }
    }
  })"))
      << program_ast;
}

TEST_F(SpvParserTest, EmitFunctions_MixedParamTypes) {
  auto p = parser(test::Assemble(Names({"mixed_params", "a", "b", "c"}) +
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
  Program program = p->program();
  const auto program_ast = program.to_str();
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function mixed_params -> __void
  (
    VariableConst{
      a
      none
      __u32
    }
    VariableConst{
      b
      none
      __f32
    }
    VariableConst{
      c
      none
      __i32
    }
  )
  {
    Return{}
  })"));
}

TEST_F(SpvParserTest, EmitFunctions_GenerateParamNames) {
  auto p = parser(test::Assemble(Names({"mixed_params"}) + CommonTypes() + R"(
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
  Program program = p->program();
  const auto program_ast = program.to_str();
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function mixed_params -> __void
  (
    VariableConst{
      x_14
      none
      __u32
    }
    VariableConst{
      x_15
      none
      __f32
    }
    VariableConst{
      x_16
      none
      __i32
    }
  )
  {
    Return{}
  })"))
      << program_ast;
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
