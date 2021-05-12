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
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::Eq;
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
  return
      R"(

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %ptr_bool = OpTypePointer Function %bool
    %ptr_float = OpTypePointer Function %float
    %ptr_uint = OpTypePointer Function %uint
    %ptr_int = OpTypePointer Function %int

    %true = OpConstantTrue %bool
    %false = OpConstantFalse %bool
    %float_0 = OpConstant %float 0.0
    %float_1p5 = OpConstant %float 1.5
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %int_m1 = OpConstant %int -1
    %uint_2 = OpConstant %uint 2
    %uint_3 = OpConstant %uint 3
    %uint_4 = OpConstant %uint 4
    %uint_5 = OpConstant %uint 5

    %v2float = OpTypeVector %float 2
    %m3v2float = OpTypeMatrix %v2float 3

    %arr2uint = OpTypeArray %uint %uint_2
    %strct = OpTypeStruct %uint %float %arr2uint
  )";
}

// Returns the SPIR-V assembly for a vertex shader, optionally
// with OpName decorations for certain SPIR-V IDs
std::string PreambleNames(std::vector<std::string> ids) {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %100 "main"
)" + Names(ids) +
         CommonTypes();
}

std::string Preamble() {
  return PreambleNames({});
}

using SpvParserFunctionVarTest = SpvParserTest;

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_AnonymousVars) {
  auto p = parser(test::Assemble(Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_uint Function
     %2 = OpVariable %ptr_uint Function
     %3 = OpVariable %ptr_uint Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_1
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    x_2
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    x_3
    none
    __u32
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_NamedVars) {
  auto p = parser(test::Assemble(PreambleNames({"a", "b", "c"}) + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_uint Function
     %b = OpVariable %ptr_uint Function
     %c = OpVariable %ptr_uint Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    b
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    c
    none
    __u32
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_MixedTypes) {
  auto p = parser(test::Assemble(PreambleNames({"a", "b", "c"}) + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_uint Function
     %b = OpVariable %ptr_int Function
     %c = OpVariable %ptr_float Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    b
    none
    __i32
  }
}
VariableDeclStatement{
  Variable{
    c
    none
    __f32
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ScalarInitializers) {
  auto p = parser(test::Assemble(PreambleNames({"a", "b", "c", "d", "e"}) + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_bool Function %true
     %b = OpVariable %ptr_bool Function %false
     %c = OpVariable %ptr_int Function %int_m1
     %d = OpVariable %ptr_uint Function %uint_1
     %e = OpVariable %ptr_float Function %float_1p5
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    none
    __bool
    {
      ScalarConstructor[not set]{true}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    none
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    none
    __i32
    {
      ScalarConstructor[not set]{-1}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    none
    __u32
    {
      ScalarConstructor[not set]{1u}
    }
  }
}
VariableDeclStatement{
  Variable{
    e
    none
    __f32
    {
      ScalarConstructor[not set]{1.500000}
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ScalarNullInitializers) {
  auto p = parser(test::Assemble(PreambleNames({"a", "b", "c", "d"}) + R"(
     %null_bool = OpConstantNull %bool
     %null_int = OpConstantNull %int
     %null_uint = OpConstantNull %uint
     %null_float = OpConstantNull %float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_bool Function %null_bool
     %b = OpVariable %ptr_int Function %null_int
     %c = OpVariable %ptr_uint Function %null_uint
     %d = OpVariable %ptr_float Function %null_float
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    none
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    none
    __i32
    {
      ScalarConstructor[not set]{0}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    none
    __u32
    {
      ScalarConstructor[not set]{0u}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    none
    __f32
    {
      ScalarConstructor[not set]{0.000000}
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_VectorInitializer) {
  auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %v2float
     %two = OpConstant %float 2.0
     %const = OpConstantComposite %v2float %float_1p5 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    none
    __vec_2__f32
    {
      TypeConstructor[not set]{
        __vec_2__f32
        ScalarConstructor[not set]{1.500000}
        ScalarConstructor[not set]{2.000000}
      }
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_MatrixInitializer) {
  auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %m3v2float
     %two = OpConstant %float 2.0
     %three = OpConstant %float 3.0
     %four = OpConstant %float 4.0
     %v0 = OpConstantComposite %v2float %float_1p5 %two
     %v1 = OpConstantComposite %v2float %two %three
     %v2 = OpConstantComposite %v2float %three %four
     %const = OpConstantComposite %m3v2float %v0 %v1 %v2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    none
    __mat_2_3__f32
    {
      TypeConstructor[not set]{
        __mat_2_3__f32
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{1.500000}
          ScalarConstructor[not set]{2.000000}
        }
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{2.000000}
          ScalarConstructor[not set]{3.000000}
        }
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{3.000000}
          ScalarConstructor[not set]{4.000000}
        }
      }
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ArrayInitializer) {
  auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    none
    __array__u32_2
    {
      TypeConstructor[not set]{
        __array__u32_2
        ScalarConstructor[not set]{1u}
        ScalarConstructor[not set]{2u}
      }
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ArrayInitializer_Alias) {
  auto p = parser(test::Assemble(R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Vertex %100 "main"
     OpDecorate %arr2uint ArrayStride 16
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  auto got = ToString(p->builder(), fe.ast_body());
  const char* expect = R"(VariableDeclStatement{
  Variable{
    x_200
    none
    __type_name_Arr
    {
      TypeConstructor[not set]{
        __type_name_Arr
        ScalarConstructor[not set]{1u}
        ScalarConstructor[not set]{2u}
      }
    }
  }
}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_ArrayInitializer_Null) {
  auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantNull %arr2uint

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    none
    __array__u32_2
    {
      TypeConstructor[not set]{
        __array__u32_2
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest,
       EmitFunctionVariables_ArrayInitializer_Alias_Null) {
  auto p = parser(test::Assemble(R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Vertex %100 "main"
     OpDecorate %arr2uint ArrayStride 16
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantNull %arr2uint

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    none
    __type_name_Arr
    {
      TypeConstructor[not set]{
        __type_name_Arr
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_StructInitializer) {
  auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantComposite %strct %uint_1 %float_1p5 %arrconst

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    none
    __type_name_S
    {
      TypeConstructor[not set]{
        __type_name_S
        ScalarConstructor[not set]{1u}
        ScalarConstructor[not set]{1.500000}
        TypeConstructor[not set]{
          __array__u32_2
          ScalarConstructor[not set]{1u}
          ScalarConstructor[not set]{2u}
        }
      }
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest, EmitFunctionVariables_StructInitializer_Null) {
  auto p = parser(test::Assemble(Preamble() + R"(
     %ptr = OpTypePointer Function %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantNull %strct

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    none
    __type_name_S
    {
      TypeConstructor[not set]{
        __type_name_S
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0.000000}
        TypeConstructor[not set]{
          __array__u32_2
          ScalarConstructor[not set]{0u}
          ScalarConstructor[not set]{0u}
        }
      }
    }
  }
}
)"));
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialValue_Defer_UsedOnceSameConstruct) {
  auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %25 = OpVariable %ptr_uint Function
     %2 = OpIAdd %uint %uint_1 %uint_1
     OpStore %25 %uint_1 ; Do initial store to mark source location
     OpBranch %20

     %20 = OpLabel
     OpStore %25 %2 ; defer emission of the addition until here.
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect =
      R"(VariableDeclStatement{
  Variable{
    x_25
    none
    __u32
  }
}
Assignment{
  Identifier[not set]{x_25}
  ScalarConstructor[not set]{1u}
}
Assignment{
  Identifier[not set]{x_25}
  Binary[not set]{
    ScalarConstructor[not set]{1u}
    add
    ScalarConstructor[not set]{1u}
  }
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialValue_Immediate_UsedTwice) {
  auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %25 = OpVariable %ptr_uint Function
     %2 = OpIAdd %uint %uint_1 %uint_1
     OpStore %25 %uint_1 ; Do initial store to mark source location
     OpBranch %20

     %20 = OpLabel
     OpStore %25 %2
     OpStore %25 %2
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  Variable{
    x_25
    none
    __u32
  }
}
VariableDeclStatement{
  VariableConst{
    x_2
    none
    __u32
    {
      Binary[not set]{
        ScalarConstructor[not set]{1u}
        add
        ScalarConstructor[not set]{1u}
      }
    }
  }
}
Assignment{
  Identifier[not set]{x_25}
  ScalarConstructor[not set]{1u}
}
Assignment{
  Identifier[not set]{x_25}
  Identifier[not set]{x_2}
}
Assignment{
  Identifier[not set]{x_25}
  Identifier[not set]{x_2}
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialValue_Immediate_UsedOnceDifferentConstruct) {
  // Translation should not sink expensive operations into or out of control
  // flow. As a simple heuristic, don't move *any* combinatorial operation
  // across any control flow.
  auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     %25 = OpVariable %ptr_uint Function
     %2 = OpIAdd %uint %uint_1 %uint_1
     OpStore %25 %uint_1 ; Do initial store to mark source location
     OpBranch %20

     %20 = OpLabel  ; Introduce a new construct
     OpLoopMerge %99 %80 None
     OpBranch %80

     %80 = OpLabel
     OpStore %25 %2  ; store combinatorial value %2, inside the loop
     OpBranch %20

     %99 = OpLabel ; merge block
     OpStore %25 %uint_2
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  Variable{
    x_25
    none
    __u32
  }
}
VariableDeclStatement{
  VariableConst{
    x_2
    none
    __u32
    {
      Binary[not set]{
        ScalarConstructor[not set]{1u}
        add
        ScalarConstructor[not set]{1u}
      }
    }
  }
}
Assignment{
  Identifier[not set]{x_25}
  ScalarConstructor[not set]{1u}
}
Loop{
  continuing {
    Assignment{
      Identifier[not set]{x_25}
      Identifier[not set]{x_2}
    }
  }
}
Assignment{
  Identifier[not set]{x_25}
  ScalarConstructor[not set]{2u}
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(
    SpvParserFunctionVarTest,
    EmitStatement_CombinatorialNonPointer_DefConstruct_DoesNotEncloseAllUses) {
  // Compensate for the difference between dominance and scoping.
  // Exercise hoisting of the constant definition to before its natural
  // location.
  //
  // The definition of %2 should be hoisted
  auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private

     %100 = OpFunction %void None %voidfn

     %3 = OpLabel
     OpStore %1 %uint_0
     OpBranch %5

     %5 = OpLabel
     OpStore %1 %uint_1
     OpLoopMerge  %99 %80 None
     OpBranchConditional %false %99 %20

     %20 = OpLabel
     OpStore %1 %uint_3
     OpSelectionMerge %50 None
     OpBranchConditional %true %30 %40

     %30 = OpLabel
     ; This combinatorial definition in nested control flow dominates
     ; the use in the merge block in %50
     %2 = OpIAdd %uint %uint_1 %uint_1
     OpBranch %50

     %40 = OpLabel
     OpReturn

     %50 = OpLabel ; merge block for if-selection
     OpStore %1 %2
     OpBranch %80

     %80 = OpLabel ; merge block
     OpStore %1 %uint_4
     OpBranchConditional %false %99 %5 ; loop backedge

     %99 = OpLabel
     OpStore %1 %uint_5
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(Assignment{
  Identifier[not set]{x_1}
  ScalarConstructor[not set]{0u}
}
Loop{
  VariableDeclStatement{
    Variable{
      x_2
      none
      __u32
    }
  }
  Assignment{
    Identifier[not set]{x_1}
    ScalarConstructor[not set]{1u}
  }
  If{
    (
      ScalarConstructor[not set]{false}
    )
    {
      Break{}
    }
  }
  Assignment{
    Identifier[not set]{x_1}
    ScalarConstructor[not set]{3u}
  }
  If{
    (
      ScalarConstructor[not set]{true}
    )
    {
      Assignment{
        Identifier[not set]{x_2}
        Binary[not set]{
          ScalarConstructor[not set]{1u}
          add
          ScalarConstructor[not set]{1u}
        }
      }
    }
  }
  Else{
    {
      Return{}
    }
  }
  Assignment{
    Identifier[not set]{x_1}
    Identifier[not set]{x_2}
  }
  continuing {
    Assignment{
      Identifier[not set]{x_1}
      ScalarConstructor[not set]{4u}
    }
    If{
      (
        ScalarConstructor[not set]{false}
      )
      {
        Break{}
      }
    }
  }
}
Assignment{
  Identifier[not set]{x_1}
  ScalarConstructor[not set]{5u}
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(
    SpvParserFunctionVarTest,
    EmitStatement_CombinatorialNonPointer_Hoisting_DefFirstBlockIf_InFunction) {
  // This is a hoisting case, where the definition is in the first block
  // of an if selection construct. In this case the definition should count
  // as being in the parent (enclosing) construct.
  //
  // The definition of %1 is in an IfSelection construct and also the enclosing
  // Function construct, both of which start at block %10. For the purpose of
  // determining the construct containing %10, go to the parent construct of
  // the IfSelection.
  auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %200 = OpVariable %pty Private
     %cond = OpConstantTrue %bool

     %100 = OpFunction %void None %voidfn

     ; in IfSelection construct, nested in Function construct
     %10 = OpLabel
     %1 = OpCopyObject %uint %uint_1
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel  ; in IfSelection construct
     OpBranch %99

     %99 = OpLabel
     %3 = OpCopyObject %uint %1; in Function construct
     OpStore %200 %3
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  // We don't hoist x_1 into its own mutable variable. It is emitted as
  // a const definition.
  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  VariableConst{
    x_1
    none
    __u32
    {
      ScalarConstructor[not set]{1u}
    }
  }
}
If{
  (
    ScalarConstructor[not set]{true}
  )
  {
  }
}
VariableDeclStatement{
  VariableConst{
    x_3
    none
    __u32
    {
      Identifier[not set]{x_1}
    }
  }
}
Assignment{
  Identifier[not set]{x_200}
  Identifier[not set]{x_3}
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialNonPointer_Hoisting_DefFirstBlockIf_InIf) {
  // This is like the previous case, but the IfSelection is nested inside
  // another IfSelection.
  // This tests that the hoisting algorithm goes to only one parent of
  // the definining if-selection block, and doesn't jump all the way out
  // to the Function construct that encloses everything.
  //
  // We should not hoist %1 because its definition should count as being
  // in the outer IfSelection, not the inner IfSelection.
  auto assembly = Preamble() + R"(

     %pty = OpTypePointer Private %uint
     %200 = OpVariable %pty Private
     %cond = OpConstantTrue %bool

     %100 = OpFunction %void None %voidfn

     ; outer IfSelection
     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     ; inner IfSelection
     %20 = OpLabel
     %1 = OpCopyObject %uint %uint_1
     OpSelectionMerge %89 None
     OpBranchConditional %cond %30 %89

     %30 = OpLabel ; last block of inner IfSelection
     OpBranch %89

     ; in outer IfSelection
     %89 = OpLabel
     %3 = OpCopyObject %uint %1; Last use of %1, in outer IfSelection
     OpStore %200 %3
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(If{
  (
    ScalarConstructor[not set]{true}
  )
  {
    VariableDeclStatement{
      VariableConst{
        x_1
        none
        __u32
        {
          ScalarConstructor[not set]{1u}
        }
      }
    }
    If{
      (
        ScalarConstructor[not set]{true}
      )
      {
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Assignment{
      Identifier[not set]{x_200}
      Identifier[not set]{x_3}
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(
    SpvParserFunctionVarTest,
    EmitStatement_CombinatorialNonPointer_Hoisting_DefFirstBlockSwitch_InIf) {
  // This is like the previous case, but the definition is in a SwitchSelection
  // inside another IfSelection.
  // Tests that definitions in the first block of a switch count as being
  // in the parent of the switch construct.
  auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %200 = OpVariable %pty Private
     %cond = OpConstantTrue %bool

     %100 = OpFunction %void None %voidfn

     ; outer IfSelection
     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     ; inner SwitchSelection
     %20 = OpLabel
     %1 = OpCopyObject %uint %uint_1
     OpSelectionMerge %89 None
     OpSwitch %uint_1 %89 0 %30

     %30 = OpLabel ; last block of inner SwitchSelection
     OpBranch %89

     ; in outer IfSelection
     %89 = OpLabel
     %3 = OpCopyObject %uint %1; Last use of %1, in outer IfSelection
     OpStore %200 %3
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(If{
  (
    ScalarConstructor[not set]{true}
  )
  {
    VariableDeclStatement{
      VariableConst{
        x_1
        none
        __u32
        {
          ScalarConstructor[not set]{1u}
        }
      }
    }
    Switch{
      ScalarConstructor[not set]{1u}
      {
        Case 0u{
        }
        Default{
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Assignment{
      Identifier[not set]{x_200}
      Identifier[not set]{x_3}
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_CombinatorialNonPointer_Hoisting_DefAndUseFirstBlockIf) {
  // In this test, both the defintion and the use are in the first block
  // of an IfSelection.  No hoisting occurs because hoisting is triggered
  // on whether the defining construct contains the last use, rather than
  // whether the two constructs are the same.
  //
  // This example has two SSA IDs which are tempting to hoist but should not:
  //   %1 is defined and used in the first block of an IfSelection.
  //       Do not hoist it.
  auto assembly = Preamble() + R"(
     %cond = OpConstantTrue %bool

     %100 = OpFunction %void None %voidfn

     ; in IfSelection construct, nested in Function construct
     %10 = OpLabel
     %1 = OpCopyObject %uint %uint_1
     %2 = OpCopyObject %uint %1
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel  ; in IfSelection construct
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  // We don't hoist x_1 into its own mutable variable. It is emitted as
  // a const definition.
  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  VariableConst{
    x_1
    none
    __u32
    {
      ScalarConstructor[not set]{1u}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_2
    none
    __u32
    {
      Identifier[not set]{x_1}
    }
  }
}
If{
  (
    ScalarConstructor[not set]{true}
  )
  {
  }
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_SingleBlockLoopIndex) {
  auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     %101 = OpLoad %bool %7
     %102 = OpLoad %bool %8
     OpLoopMerge %99 %89 None
     OpBranchConditional %101 %99 %20

     %20 = OpLabel
     %2 = OpPhi %uint %uint_0 %10 %4 %20  ; gets computed value
     %3 = OpPhi %uint %uint_1 %10 %3 %20  ; gets itself
     %4 = OpIAdd %uint %2 %uint_1
     OpLoopMerge %79 %20 None
     OpBranchConditional %102 %79 %20

     %79 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(Loop{
  VariableDeclStatement{
    Variable{
      x_2_phi
      none
      __u32
    }
  }
  VariableDeclStatement{
    Variable{
      x_3_phi
      none
      __u32
    }
  }
  VariableDeclStatement{
    VariableConst{
      x_101
      none
      __bool
      {
        Identifier[not set]{x_7}
      }
    }
  }
  VariableDeclStatement{
    VariableConst{
      x_102
      none
      __bool
      {
        Identifier[not set]{x_8}
      }
    }
  }
  Assignment{
    Identifier[not set]{x_2_phi}
    ScalarConstructor[not set]{0u}
  }
  Assignment{
    Identifier[not set]{x_3_phi}
    ScalarConstructor[not set]{1u}
  }
  If{
    (
      Identifier[not set]{x_101}
    )
    {
      Break{}
    }
  }
  Loop{
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_2_phi}
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        __u32
        {
          Identifier[not set]{x_3_phi}
        }
      }
    }
    Assignment{
      Identifier[not set]{x_2_phi}
      Binary[not set]{
        Identifier[not set]{x_2}
        add
        ScalarConstructor[not set]{1u}
      }
    }
    Assignment{
      Identifier[not set]{x_3_phi}
      Identifier[not set]{x_3}
    }
    If{
      (
        Identifier[not set]{x_102}
      )
      {
        Break{}
      }
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_MultiBlockLoopIndex) {
  auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     %101 = OpLoad %bool %7
     %102 = OpLoad %bool %8
     OpLoopMerge %99 %89 None
     OpBranchConditional %101 %99 %20

     %20 = OpLabel
     %2 = OpPhi %uint %uint_0 %10 %4 %30  ; gets computed value
     %3 = OpPhi %uint %uint_1 %10 %3 %30  ; gets itself
     OpLoopMerge %79 %30 None
     OpBranchConditional %102 %79 %30

     %30 = OpLabel
     %4 = OpIAdd %uint %2 %uint_1
     OpBranch %20

     %79 = OpLabel
     OpBranch %89

     %89 = OpLabel ; continue target for outer loop
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(Loop{
  VariableDeclStatement{
    Variable{
      x_2_phi
      none
      __u32
    }
  }
  VariableDeclStatement{
    Variable{
      x_3_phi
      none
      __u32
    }
  }
  VariableDeclStatement{
    VariableConst{
      x_101
      none
      __bool
      {
        Identifier[not set]{x_7}
      }
    }
  }
  VariableDeclStatement{
    VariableConst{
      x_102
      none
      __bool
      {
        Identifier[not set]{x_8}
      }
    }
  }
  Assignment{
    Identifier[not set]{x_2_phi}
    ScalarConstructor[not set]{0u}
  }
  Assignment{
    Identifier[not set]{x_3_phi}
    ScalarConstructor[not set]{1u}
  }
  If{
    (
      Identifier[not set]{x_101}
    )
    {
      Break{}
    }
  }
  Loop{
    VariableDeclStatement{
      Variable{
        x_4
        none
        __u32
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_2_phi}
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        __u32
        {
          Identifier[not set]{x_3_phi}
        }
      }
    }
    If{
      (
        Identifier[not set]{x_102}
      )
      {
        Break{}
      }
    }
    continuing {
      Assignment{
        Identifier[not set]{x_4}
        Binary[not set]{
          Identifier[not set]{x_2}
          add
          ScalarConstructor[not set]{1u}
        }
      }
      Assignment{
        Identifier[not set]{x_2_phi}
        Identifier[not set]{x_4}
      }
      Assignment{
        Identifier[not set]{x_3_phi}
        Identifier[not set]{x_3}
      }
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_Phi_ValueFromLoopBodyAndContinuing) {
  auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %17 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %9 = OpLabel
     %101 = OpLoad %bool %17
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     OpLoopMerge %99 %89 None
     OpBranch %20

     %20 = OpLabel
     %2 = OpPhi %uint %uint_0 %10 %4 %30  ; gets computed value
     %5 = OpPhi %uint %uint_1 %10 %7 %30
     %4 = OpIAdd %uint %2 %uint_1 ; define %4
     %6 = OpIAdd %uint %4 %uint_1 ; use %4
     OpLoopMerge %79 %30 None
     OpBranchConditional %101 %79 %30

     %30 = OpLabel
     %7 = OpIAdd %uint %4 %6 ; use %4 again
     OpBranch %20

     %79 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  VariableConst{
    x_101
    none
    __bool
    {
      Identifier[not set]{x_17}
    }
  }
}
Loop{
  VariableDeclStatement{
    Variable{
      x_2_phi
      none
      __u32
    }
  }
  VariableDeclStatement{
    Variable{
      x_5_phi
      none
      __u32
    }
  }
  Assignment{
    Identifier[not set]{x_2_phi}
    ScalarConstructor[not set]{0u}
  }
  Assignment{
    Identifier[not set]{x_5_phi}
    ScalarConstructor[not set]{1u}
  }
  Loop{
    VariableDeclStatement{
      Variable{
        x_7
        none
        __u32
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_2_phi}
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_5
        none
        __u32
        {
          Identifier[not set]{x_5_phi}
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        __u32
        {
          Binary[not set]{
            Identifier[not set]{x_2}
            add
            ScalarConstructor[not set]{1u}
          }
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_6
        none
        __u32
        {
          Binary[not set]{
            Identifier[not set]{x_4}
            add
            ScalarConstructor[not set]{1u}
          }
        }
      }
    }
    If{
      (
        Identifier[not set]{x_101}
      )
      {
        Break{}
      }
    }
    continuing {
      Assignment{
        Identifier[not set]{x_7}
        Binary[not set]{
          Identifier[not set]{x_4}
          add
          Identifier[not set]{x_6}
        }
      }
      Assignment{
        Identifier[not set]{x_2_phi}
        Identifier[not set]{x_4}
      }
      Assignment{
        Identifier[not set]{x_5_phi}
        Identifier[not set]{x_7}
      }
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_FromElseAndThen) {
  auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     %101 = OpLoad %bool %7
     %102 = OpLoad %bool %8
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     OpLoopMerge %99 %89 None
     OpBranchConditional %101 %99 %20

     %20 = OpLabel ; if seleciton
     OpSelectionMerge %79 None
     OpBranchConditional %102 %30 %40

     %30 = OpLabel
     OpBranch %89

     %40 = OpLabel
     OpBranch %89

     %79 = OpLabel ; disconnected selection merge node
     OpBranch %89

     %89 = OpLabel
     %2 = OpPhi %uint %uint_0 %30 %uint_1 %40
     OpStore %1 %2
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  VariableConst{
    x_101
    none
    __bool
    {
      Identifier[not set]{x_7}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_102
    none
    __bool
    {
      Identifier[not set]{x_8}
    }
  }
}
Loop{
  VariableDeclStatement{
    Variable{
      x_2_phi
      none
      __u32
    }
  }
  If{
    (
      Identifier[not set]{x_101}
    )
    {
      Break{}
    }
  }
  If{
    (
      Identifier[not set]{x_102}
    )
    {
      Assignment{
        Identifier[not set]{x_2_phi}
        ScalarConstructor[not set]{0u}
      }
      Continue{}
    }
  }
  Else{
    {
      Assignment{
        Identifier[not set]{x_2_phi}
        ScalarConstructor[not set]{1u}
      }
      Continue{}
    }
  }
  continuing {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_2_phi}
        }
      }
    }
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_2}
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got) << got;
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_Phi_FromHeaderAndThen) {
  auto assembly = Preamble() + R"(
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     %101 = OpLoad %bool %7
     %102 = OpLoad %bool %8
     OpBranch %10

     ; Use an outer loop to show we put the new variable in the
     ; smallest enclosing scope.
     %10 = OpLabel
     OpLoopMerge %99 %89 None
     OpBranchConditional %101 %99 %20

     %20 = OpLabel ; if seleciton
     OpSelectionMerge %79 None
     OpBranchConditional %102 %30 %89

     %30 = OpLabel
     OpBranch %89

     %79 = OpLabel ; disconnected selection merge node
     OpUnreachable

     %89 = OpLabel
     %2 = OpPhi %uint %uint_0 %20 %uint_1 %30
     OpStore %1 %2
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  VariableConst{
    x_101
    none
    __bool
    {
      Identifier[not set]{x_7}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_102
    none
    __bool
    {
      Identifier[not set]{x_8}
    }
  }
}
Loop{
  VariableDeclStatement{
    Variable{
      x_2_phi
      none
      __u32
    }
  }
  If{
    (
      Identifier[not set]{x_101}
    )
    {
      Break{}
    }
  }
  Assignment{
    Identifier[not set]{x_2_phi}
    ScalarConstructor[not set]{0u}
  }
  If{
    (
      Identifier[not set]{x_102}
    )
    {
      Assignment{
        Identifier[not set]{x_2_phi}
        ScalarConstructor[not set]{1u}
      }
      Continue{}
    }
  }
  Else{
    {
      Continue{}
    }
  }
  Return{}
  continuing {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_2_phi}
        }
      }
    }
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_2}
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got) << got;
}

TEST_F(SpvParserFunctionVarTest,
       EmitStatement_Phi_InMerge_PredecessorsDominatdByNestedSwitchCase) {
  // This is the essence of the bug report from crbug.com/tint/495
  auto assembly = Preamble() + R"(
     %cond = OpConstantTrue %bool
     %pty = OpTypePointer Private %uint
     %1 = OpVariable %pty Private
     %boolpty = OpTypePointer Private %bool
     %7 = OpVariable %boolpty Private
     %8 = OpVariable %boolpty Private

     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %uint_1 %20 0 %20 1 %30

       %20 = OpLabel ; case 0
       OpBranch %30 ;; fall through

       %30 = OpLabel ; case 1
       OpSelectionMerge %50 None
       OpBranchConditional %true %40 %45

         %40 = OpLabel
         OpBranch %50

         %45 = OpLabel
         OpBranch %99 ; break

       %50 = OpLabel ; end the case
       OpBranch %99

     %99 = OpLabel
     ; predecessors are all dominated by case construct head at %30
     %phi = OpPhi %uint %uint_0 %45 %uint_1 %50
     OpReturn

     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  Variable{
    x_35_phi
    none
    __u32
  }
}
Switch{
  ScalarConstructor[not set]{1u}
  {
    Default{
      Fallthrough{}
    }
    Case 0u{
      Fallthrough{}
    }
    Case 1u{
      If{
        (
          ScalarConstructor[not set]{true}
        )
        {
        }
      }
      Else{
        {
          Assignment{
            Identifier[not set]{x_35_phi}
            ScalarConstructor[not set]{0u}
          }
          Break{}
        }
      }
      Assignment{
        Identifier[not set]{x_35_phi}
        ScalarConstructor[not set]{1u}
      }
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_35
    none
    __u32
    {
      Identifier[not set]{x_35_phi}
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got) << got << assembly;
}

TEST_F(SpvParserFunctionVarTest, EmitStatement_UseInPhiCountsAsUse) {
  // From crbug.com/215
  // If the only use of a combinatorially computed ID is as the value
  // in an OpPhi, then we still have to emit it.  The algorithm fix
  // is to always count uses in Phis.
  // This is the reduced case from the bug report.
  //
  // The only use of %12 is in the phi.
  // The only use of %11 is in %12.
  // Both definintions need to be emitted to the output.
  auto assembly = Preamble() + R"(
        %100 = OpFunction %void None %voidfn

         %10 = OpLabel
         %11 = OpLogicalAnd %bool %true %true
         %12 = OpLogicalNot %bool %11  ;
               OpSelectionMerge %99 None
               OpBranchConditional %true %20 %99

         %20 = OpLabel
               OpBranch %99

         %99 = OpLabel
        %101 = OpPhi %bool %11 %10 %12 %20
               OpReturn

               OpFunctionEnd

  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();

  auto got = ToString(p->builder(), fe.ast_body());
  auto* expect = R"(VariableDeclStatement{
  Variable{
    x_101_phi
    none
    __bool
  }
}
VariableDeclStatement{
  VariableConst{
    x_11
    none
    __bool
    {
      Binary[not set]{
        ScalarConstructor[not set]{true}
        logical_and
        ScalarConstructor[not set]{true}
      }
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_12
    none
    __bool
    {
      UnaryOp[not set]{
        not
        Identifier[not set]{x_11}
      }
    }
  }
}
Assignment{
  Identifier[not set]{x_101_phi}
  Identifier[not set]{x_11}
}
If{
  (
    ScalarConstructor[not set]{true}
  )
  {
    Assignment{
      Identifier[not set]{x_101_phi}
      Identifier[not set]{x_12}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_101
    none
    __bool
    {
      Identifier[not set]{x_101_phi}
    }
  }
}
Return{}
)";
  EXPECT_EQ(expect, got);
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
