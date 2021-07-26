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

std::string Caps() {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
  )";
}

std::string Preamble() {
  return Caps() + R"(
    OpEntryPoint Fragment %main "x_100"
    OpExecutionMode %main OriginUpperLeft
  )";
}

std::string MainBody() {
  return R"(
    %main = OpFunction %void None %voidfn
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd
  )";
}

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
  )";
}

std::string BuiltinPosition() {
  return R"(OpDecorate %position BuiltIn Position
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %ptr = OpTypePointer Output %v4float
    %position = OpVariable %ptr Output
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
)";
}

TEST_F(SpvParserTest, EmitFunctions_NoFunctions) {
  auto p = parser(test::Assemble(
      R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
)" + CommonTypes()));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, Not(HasSubstr("Function{")));
  p->SkipDumpingPending("Not valid for Vulkan: needs an entry point");
}

TEST_F(SpvParserTest, EmitFunctions_FunctionWithoutBody) {
  auto p =
      parser(test::Assemble(Preamble() + Names({"main"}) + CommonTypes() + R"(
     %main = OpFunction %void None %voidfn
     OpFunctionEnd
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, Not(HasSubstr("Function{")));
  p->SkipDumpingPending("Missing an entry point body requires Linkage");
}

TEST_F(SpvParserTest, EmitFunctions_Function_EntryPoint_Vertex) {
  std::string input = Caps() +
                      R"(OpEntryPoint Vertex %main "main" %position )" +
                      Names({"main"}) + BuiltinPosition() + R"(

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
  Struct $3 {
    StructMember{[[ BuiltinDecoration{position}
 ]] $4: __vec_4__f32})"))
      << program_ast;

  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("main").to_str() +
                                     R"( -> __type_name_$3
  StageDecoration{vertex}
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_Function_EntryPoint_Fragment) {
  std::string input = Caps() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
)" + Names({"main"}) + CommonTypes() +
                      MainBody();

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
  std::string input = Caps() + R"(
      OpEntryPoint GLCompute %main "main"
      OpExecutionMode %main LocalSize 1 1 1
)" + Names({"main"}) + CommonTypes() +
                      MainBody();

  auto p = parser(test::Assemble(input));
  ASSERT_TRUE(p->BuildAndParseInternalModule());
  ASSERT_TRUE(p->error().empty()) << p->error();
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("main").to_str() +
                                     R"( -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{1}
    ScalarConstructor[not set]{1}
    ScalarConstructor[not set]{1}
  }
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_Function_EntryPoint_MultipleEntryPoints) {
  std::string input = Caps() +
                      R"(
OpEntryPoint Fragment %main "first_shader"
OpEntryPoint Fragment %main "second_shader"
OpExecutionMode %main OriginUpperLeft
)" + Names({"main"}) + CommonTypes() +
                      MainBody();

  auto p = parser(test::Assemble(input));
  ASSERT_TRUE(p->BuildAndParseInternalModule());
  ASSERT_TRUE(p->error().empty()) << p->error();
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("first_shader").to_str() +
                                     R"( -> __void
  StageDecoration{fragment}
  ()
  {)"));
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("second_shader").to_str() +
                                     R"( -> __void
  StageDecoration{fragment}
  ()
  {)"));
}

TEST_F(SpvParserTest,
       EmitFunctions_Function_EntryPoint_GLCompute_LocalSize_Only) {
  std::string input = Caps() + R"(
OpEntryPoint GLCompute %main "comp_main"
OpExecutionMode %main LocalSize 2 4 8
)" + Names({"main"}) + CommonTypes() +
                      R"(
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
  Function )" + program.Symbols().Get("comp_main").to_str() +
                                     R"( -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{2}
    ScalarConstructor[not set]{4}
    ScalarConstructor[not set]{8}
  }
  ()
  {)"))
      << program_ast;
}

TEST_F(SpvParserTest,
       EmitFunctions_Function_EntryPoint_WorkgroupSizeBuiltin_Constant_Only) {
  std::string input = Caps() + R"(OpEntryPoint GLCompute %main "comp_main"
OpDecorate %wgsize BuiltIn WorkgroupSize
)" + CommonTypes() + R"(
%uvec3 = OpTypeVector %uint 3
%uint_3 = OpConstant %uint 3
%uint_5 = OpConstant %uint 5
%uint_7 = OpConstant %uint 7
%wgsize = OpConstantComposite %uvec3 %uint_3 %uint_5 %uint_7
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
  Function )" + program.Symbols().Get("comp_main").to_str() +
                                     R"( -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{3}
    ScalarConstructor[not set]{5}
    ScalarConstructor[not set]{7}
  }
  ()
  {)"))
      << program_ast;
}

TEST_F(
    SpvParserTest,
    EmitFunctions_Function_EntryPoint_WorkgroupSizeBuiltin_SpecConstant_Only) {
  std::string input = Caps() +
                      R"(OpEntryPoint GLCompute %main "comp_main"
OpDecorate %wgsize BuiltIn WorkgroupSize
OpDecorate %uint_3 SpecId 0
OpDecorate %uint_5 SpecId 1
OpDecorate %uint_7 SpecId 2
)" + CommonTypes() + R"(
%uvec3 = OpTypeVector %uint 3
%uint_3 = OpSpecConstant %uint 3
%uint_5 = OpSpecConstant %uint 5
%uint_7 = OpSpecConstant %uint 7
%wgsize = OpSpecConstantComposite %uvec3 %uint_3 %uint_5 %uint_7
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
  Function )" + program.Symbols().Get("comp_main").to_str() +
                                     R"( -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{3}
    ScalarConstructor[not set]{5}
    ScalarConstructor[not set]{7}
  }
  ()
  {)"))
      << program_ast;
}

TEST_F(
    SpvParserTest,
    EmitFunctions_Function_EntryPoint_WorkgroupSize_MixedConstantSpecConstant) {
  std::string input = Caps() +
                      R"(OpEntryPoint GLCompute %main "comp_main"
OpDecorate %wgsize BuiltIn WorkgroupSize
OpDecorate %uint_3 SpecId 0
OpDecorate %uint_7 SpecId 2
)" + CommonTypes() + R"(
%uvec3 = OpTypeVector %uint 3
%uint_3 = OpSpecConstant %uint 3
%uint_5 = OpConstant %uint 5
%uint_7 = OpSpecConstant %uint 7
%wgsize = OpSpecConstantComposite %uvec3 %uint_3 %uint_5 %uint_7
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
  Function )" + program.Symbols().Get("comp_main").to_str() +
                                     R"( -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{3}
    ScalarConstructor[not set]{5}
    ScalarConstructor[not set]{7}
  }
  ()
  {)"))
      << program_ast;
}

TEST_F(
    SpvParserTest,
    // I had to shorten the name to pass the linter.
    EmitFunctions_Function_EntryPoint_LocalSize_And_WGSBuiltin_SpecConstant) {
  // WorkgroupSize builtin wins.
  std::string input = Caps() +
                      R"(OpEntryPoint GLCompute %main "comp_main"
OpExecutionMode %main LocalSize 2 4 8
OpDecorate %wgsize BuiltIn WorkgroupSize
OpDecorate %uint_3 SpecId 0
OpDecorate %uint_5 SpecId 1
OpDecorate %uint_7 SpecId 2
)" + CommonTypes() + R"(
%uvec3 = OpTypeVector %uint 3
%uint_3 = OpSpecConstant %uint 3
%uint_5 = OpSpecConstant %uint 5
%uint_7 = OpSpecConstant %uint 7
%wgsize = OpSpecConstantComposite %uvec3 %uint_3 %uint_5 %uint_7
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
  Function )" + program.Symbols().Get("comp_main").to_str() +
                                     R"( -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{3}
    ScalarConstructor[not set]{5}
    ScalarConstructor[not set]{7}
  }
  ()
  {)"))
      << program_ast;
}

TEST_F(SpvParserTest, EmitFunctions_VoidFunctionWithoutParams) {
  auto p = parser(test::Assemble(Preamble() + Names({"another_function"}) +
                                 CommonTypes() + R"(
    %another_function = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)" + MainBody()));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  Program program = p->program();
  const auto program_ast = program.to_str(false);
  EXPECT_THAT(program_ast, HasSubstr(R"(
  Function )" + program.Symbols().Get("another_function").to_str() +
                                     R"( -> __void
  ()
  {)"));
}

TEST_F(SpvParserTest, EmitFunctions_CalleePrecedesCaller) {
  auto p = parser(test::Assemble(
      Preamble() +
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
  )" + MainBody()));
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
        undefined
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
        undefined
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
  })")) << program_ast;
}

TEST_F(SpvParserTest, EmitFunctions_NonVoidResultType) {
  auto p = parser(
      test::Assemble(Preamble() + Names({"ret_float"}) + CommonTypes() + R"(
     %float_0 = OpConstant %float 0.0
     %fn_ret_float = OpTypeFunction %float

     %ret_float = OpFunction %float None %fn_ret_float
     %ret_float_entry = OpLabel
     OpReturnValue %float_0
     OpFunctionEnd
)" + MainBody()));
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
  auto p = parser(test::Assemble(
      Preamble() + Names({"mixed_params", "a", "b", "c"}) + CommonTypes() + R"(
     %fn_mixed_params = OpTypeFunction %void %uint %float %int

     %mixed_params = OpFunction %void None %fn_mixed_params
     %a = OpFunctionParameter %uint
     %b = OpFunctionParameter %float
     %c = OpFunctionParameter %int
     %mixed_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )" + MainBody()));
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
      undefined
      __u32
    }
    VariableConst{
      b
      none
      undefined
      __f32
    }
    VariableConst{
      c
      none
      undefined
      __i32
    }
  )
  {
    Return{}
  })"));
}

TEST_F(SpvParserTest, EmitFunctions_GenerateParamNames) {
  auto p = parser(
      test::Assemble(Preamble() + Names({"mixed_params"}) + CommonTypes() + R"(
     %fn_mixed_params = OpTypeFunction %void %uint %float %int

     %mixed_params = OpFunction %void None %fn_mixed_params
     %14 = OpFunctionParameter %uint
     %15 = OpFunctionParameter %float
     %16 = OpFunctionParameter %int
     %mixed_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )" + MainBody()));
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
      undefined
      __u32
    }
    VariableConst{
      x_15
      none
      undefined
      __f32
    }
    VariableConst{
      x_16
      none
      undefined
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
