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

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

TEST_F(SpvParserTest, EmitStatement_StoreBoolConst) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeBool
     %true = OpConstantTrue %ty
     %false = OpConstantFalse %ty
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %true
     OpStore %1 %false
     OpStore %1 %null
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{true}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{false}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{false}
})"));
}

TEST_F(SpvParserTest, EmitStatement_StoreUintConst) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{42}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{0}
})"));
}

TEST_F(SpvParserTest, EmitStatement_StoreIntConst) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 1
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{42}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{0}
})"));
}

TEST_F(SpvParserTest, EmitStatement_StoreFloatConst) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeFloat 32
     %val = OpConstant %ty 42
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function
     OpStore %1 %val
     OpStore %1 %null
     OpReturn
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{42.000000}
}
Assignment{
  Identifier{x_1}
  ScalarConstructor{0.000000}
})"));
}

TEST_F(SpvParserTest, EmitStatement_LoadBool) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeBool
     %true = OpConstantTrue %ty
     %false = OpConstantFalse %ty
     %null = OpConstantNull %ty
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %true
     %2 = OpLoad %ty %1
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_2
    none
    __bool
    {
      Identifier{x_1}
    }
  })"));
}

TEST_F(SpvParserTest, EmitStatement_LoadScalar) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %ty_42 = OpConstant %ty 42
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %ty_42
     %2 = OpLoad %ty %1
     %3 = OpLoad %ty %1
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_2
    none
    __u32
    {
      Identifier{x_1}
    }
  }
}
VariableDeclStatement{
  Variable{
    x_3
    none
    __u32
    {
      Identifier{x_1}
    }
  }
})"));
}

TEST_F(SpvParserTest, EmitStatement_UseLoadedScalarTwice) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %ty_42 = OpConstant %ty 42
     %ptr_ty = OpTypePointer Function %ty
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_ty Function %ty_42
     %2 = OpLoad %ty %1
     OpStore %1 %2
     OpStore %1 %2
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_2
    none
    __u32
    {
      Identifier{x_1}
    }
  }
}
Assignment{
  Identifier{x_1}
  Identifier{x_2}
}
Assignment{
  Identifier{x_1}
  Identifier{x_2}
}
)"));
}

TEST_F(SpvParserTest, EmitStatement_StoreToModuleScopeVar) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %val = OpConstant %ty 42
     %ptr_ty = OpTypePointer Workgroup %ty
     %1 = OpVariable %ptr_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpStore %1 %val
     OpReturn
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  Identifier{x_1}
  ScalarConstructor{42}
})"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_NoOperands) {
  auto err = test::AssembleFailure(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %ty = OpTypeInt 32 0
     %val = OpConstant %ty 42
     %ptr_ty = OpTypePointer Workgroup %ty
     %1 = OpVariable %ptr_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel

     %2 = OpAccessChain %ptr_ty  ; Needs a base operand
     OpStore %1 %val
     OpReturn
  )");
  EXPECT_THAT(err,
              Eq("11:5: Expected operand, found next instruction instead."));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_BaseIsNotPointer) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %10 = OpTypeInt 32 0
     %val = OpConstant %10 42
     %ptr_ty = OpTypePointer Workgroup %10
     %20 = OpVariable %10 Workgroup ; bad pointer type
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpAccessChain %ptr_ty %20
     OpStore %1 %val
     OpReturn
  )"));
  EXPECT_FALSE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_THAT(p->error(), Eq("variable with ID 20 has non-pointer type 10"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_VectorSwizzle) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %uint_2 = OpConstant %uint 2
     %uint_42 = OpConstant %uint 42
     %elem_ty = OpTypePointer Workgroup %uint
     %var_ty = OpTypePointer Workgroup %store_ty
     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_2
     OpStore %2 %uint_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  MemberAccessor{
    Identifier{myvar}
    Identifier{z}
  }
  ScalarConstructor{42}
})")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_VectorConstOutOfBounds) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %42 = OpConstant %uint 42
     %uint_99 = OpConstant %uint 99
     %elem_ty = OpTypePointer Workgroup %uint
     %var_ty = OpTypePointer Workgroup %store_ty
     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %42
     OpStore %2 %uint_99
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(), Eq("Access chain %2 index %42 value 42 is out of "
                             "bounds for vector of 4 elements"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_VectorNonConstIndex) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %store_ty = OpTypeVector %uint 4
     %uint_2 = OpConstant %uint 2
     %uint_42 = OpConstant %uint 42
     %elem_ty = OpTypePointer Workgroup %uint
     %var_ty = OpTypePointer Workgroup %store_ty
     %1 = OpVariable %var_ty Workgroup
     %10 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %11 = OpLoad %uint %10
     %2 = OpAccessChain %elem_ty %1 %11
     OpStore %2 %uint_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  ArrayAccessor{
    Identifier{myvar}
    Identifier{x_11}
  }
  ScalarConstructor{42}
})"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_Matrix) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
     %m3v4float = OpTypeMatrix %v4float 3
     %elem_ty = OpTypePointer Workgroup %v4float
     %var_ty = OpTypePointer Workgroup %m3v4float
     %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %float_42 = OpConstant %float 42
     %v4float_42 = OpConstantComposite %v4float %float_42 %float_42 %float_42 %float_42

     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_2
     OpStore %2 %v4float_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  ArrayAccessor{
    Identifier{myvar}
    ScalarConstructor{2}
  }
  TypeConstructor{
    __vec_4__f32
    ScalarConstructor{42.000000}
    ScalarConstructor{42.000000}
    ScalarConstructor{42.000000}
    ScalarConstructor{42.000000}
  }
})"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_Array) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
     %m3v4float = OpTypeMatrix %v4float 3
     %elem_ty = OpTypePointer Workgroup %v4float
     %var_ty = OpTypePointer Workgroup %m3v4float
     %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %float_42 = OpConstant %float 42
     %v4float_42 = OpConstantComposite %v4float %float_42 %float_42 %float_42 %float_42

     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_2
     OpStore %2 %v4float_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  ArrayAccessor{
    Identifier{myvar}
    ScalarConstructor{2}
  }
  TypeConstructor{
    __vec_4__f32
    ScalarConstructor{42.000000}
    ScalarConstructor{42.000000}
    ScalarConstructor{42.000000}
    ScalarConstructor{42.000000}
  }
})"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_Struct) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     OpMemberName %strct 1 "age"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %strct = OpTypeStruct %float %float
     %elem_ty = OpTypePointer Workgroup %float
     %var_ty = OpTypePointer Workgroup %strct
     %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1

     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_1
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  MemberAccessor{
    Identifier{myvar}
    Identifier{age}
  }
  ScalarConstructor{42.000000}
})"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_Struct_DifferOnlyMemberName) {
  // The spirv-opt internal representation will map both structs to the
  // same canonicalized type, because it doesn't care about member names.
  // But we care about member names when producing a member-access expression.
  // crbug.com/tint/213
  const std::string assembly = R"(
     OpName %1 "myvar"
     OpName %10 "myvar2"
     OpMemberName %strct 1 "age"
     OpMemberName %strct2 1 "ancientness"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %float_420 = OpConstant %float 420
     %strct = OpTypeStruct %float %float
     %strct2 = OpTypeStruct %float %float
     %elem_ty = OpTypePointer Workgroup %float
     %var_ty = OpTypePointer Workgroup %strct
     %var2_ty = OpTypePointer Workgroup %strct2
     %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1

     %1 = OpVariable %var_ty Workgroup
     %10 = OpVariable %var2_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_1
     OpStore %2 %float_42
     %20 = OpAccessChain %elem_ty %10 %uint_1
     OpStore %20 %float_420
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  MemberAccessor{
    Identifier{myvar}
    Identifier{age}
  }
  ScalarConstructor{42.000000}
}
Assignment{
  MemberAccessor{
    Identifier{myvar2}
    Identifier{ancientness}
  }
  ScalarConstructor{420.000000}
})")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_StructNonConstIndex) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     OpMemberName %55 1 "age"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %55 = OpTypeStruct %float %float
     %elem_ty = OpTypePointer Workgroup %float
     %var_ty = OpTypePointer Workgroup %55
     %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_ptr = OpTypePointer Workgroup %uint
     %uintvar = OpVariable %uint_ptr Workgroup

     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpLoad %uint %uintvar
     %2 = OpAccessChain %elem_ty %1 %10
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(), Eq("Access chain %2 index %10 is a non-constant "
                             "index into a structure %55"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_StructConstOutOfBounds) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     OpMemberName %55 1 "age"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %55 = OpTypeStruct %float %float
     %elem_ty = OpTypePointer Workgroup %float
     %var_ty = OpTypePointer Workgroup %55
     %uint = OpTypeInt 32 0
     %uint_99 = OpConstant %uint 99

     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_99
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(), Eq("Access chain %2 index value 99 is out of bounds "
                             "for structure %55 having 2 members"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_Struct_RuntimeArray) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     OpMemberName %strct 1 "age"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %float_42 = OpConstant %float 42
     %rtarr = OpTypeRuntimeArray %float
     %strct = OpTypeStruct %float %rtarr
     %elem_ty = OpTypePointer Workgroup %float
     %var_ty = OpTypePointer Workgroup %strct
     %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2

     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_1 %uint_2
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  ArrayAccessor{
    MemberAccessor{
      Identifier{myvar}
      Identifier{age}
    }
    ScalarConstructor{2}
  }
  ScalarConstructor{42.000000}
})"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_Compound_Matrix_Vector) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v4float = OpTypeVector %float 4
     %m3v4float = OpTypeMatrix %v4float 3
     %elem_ty = OpTypePointer Workgroup %float
     %var_ty = OpTypePointer Workgroup %m3v4float
     %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %float_42 = OpConstant %float 42

     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %elem_ty %1 %uint_2 %uint_3
     OpStore %2 %float_42
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody());
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  MemberAccessor{
    ArrayAccessor{
      Identifier{myvar}
      ScalarConstructor{2}
    }
    Identifier{w}
  }
  ScalarConstructor{42.000000}
})"));
}

TEST_F(SpvParserTest, EmitStatement_AccessChain_InvalidPointeeType) {
  const std::string assembly = R"(
     OpName %1 "myvar"
     %55 = OpTypeVoid
     %voidfn = OpTypeFunction %55
     %float = OpTypeFloat 32
     %60 = OpTypePointer Workgroup %55
     %var_ty = OpTypePointer Workgroup %60
     %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2

     %1 = OpVariable %var_ty Workgroup
     %100 = OpFunction %55 None %voidfn
     %entry = OpLabel
     %2 = OpAccessChain %60 %1 %uint_2
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              HasSubstr("Access chain with unknown or invalid pointee type "
                        "%60: %60 = OpTypePointer Workgroup %55"));
}

std::string OldStorageBufferPreamble() {
  return R"(
     OpName %myvar "myvar"

     OpDecorate %struct BufferBlock
     OpMemberDecorate %struct 0 Offset 0
     OpMemberDecorate %struct 1 Offset 4
     OpDecorate %arr ArrayStride 4

     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0

     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1

     %arr = OpTypeRuntimeArray %uint
     %struct = OpTypeStruct %uint %arr
     %ptr_struct = OpTypePointer Uniform %struct
     %ptr_uint = OpTypePointer Uniform %uint

     %myvar = OpVariable %ptr_struct Uniform
  )";
}

TEST_F(SpvParserTest, RemapStorageBuffer_TypesAndVarDeclarations) {
  // Enusure we get the right module-scope declaration.  This tests translation
  // of the structure type, arrays of the structure, pointers to them, and
  // OpVariable of these.
  const auto assembly = OldStorageBufferPreamble();
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << assembly << p->error();
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    myvar
    storage_buffer
    __alias_S__struct_S
  }
  RTArr -> __array__u32_stride_4
  S -> __struct_S
  [[block]] Struct{
    StructMember{[[ offset 0 ]] field0: __u32}
    StructMember{[[ offset 4 ]] field1: __alias_RTArr__array__u32_stride_4}
  })"));
}

TEST_F(SpvParserTest, RemapStorageBuffer_ThroughAccessChain_NonCascaded) {
  const auto assembly = OldStorageBufferPreamble() + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; the scalar element
  %1 = OpAccessChain %ptr_uint %myvar %uint_0
  OpStore %1 %uint_0

  ; element in the runtime array
  %2 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  MemberAccessor{
    Identifier{myvar}
    Identifier{field0}
  }
  ScalarConstructor{0}
}
Assignment{
  ArrayAccessor{
    MemberAccessor{
      Identifier{myvar}
      Identifier{field1}
    }
    ScalarConstructor{1}
  }
  ScalarConstructor{0}
})")) << ToString(fe.ast_body())
      << p->error();
}

TEST_F(SpvParserTest, RemapStorageBuffer_ThroughAccessChain_Cascaded) {
  const auto assembly = OldStorageBufferPreamble() + R"(
  %ptr_rtarr = OpTypePointer Uniform %arr
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  ; get the runtime array
  %1 = OpAccessChain %ptr_rtarr %myvar %uint_1
  ; now an element in it
  %2 = OpAccessChain %ptr_uint %1 %uint_1
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Assignment{
  ArrayAccessor{
    MemberAccessor{
      Identifier{myvar}
      Identifier{field1}
    }
    ScalarConstructor{1}
  }
  ScalarConstructor{0}
})")) << ToString(fe.ast_body())
      << p->error();
}

TEST_F(SpvParserTest, RemapStorageBuffer_ThroughCopyObject_WithoutHoisting) {
  // Generates a const declaration directly.
  // We have to do a bunch of storage class tracking for locally
  // defined values in order to get the right pointer-to-storage-buffer
  // value type for the const declration.
  const auto assembly = OldStorageBufferPreamble() + R"(
  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  %1 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  %2 = OpCopyObject %ptr_uint %1
  OpStore %2 %uint_0

  OpReturn
  OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_2
    none
    __ptr_storage_buffer__u32
    {
      ArrayAccessor{
        MemberAccessor{
          Identifier{myvar}
          Identifier{field1}
        }
        ScalarConstructor{1}
      }
    }
  }
}
Assignment{
  Identifier{x_2}
  ScalarConstructor{0}
})")) << ToString(fe.ast_body())
      << p->error();
}

TEST_F(SpvParserTest, RemapStorageBuffer_ThroughCopyObject_WithHoisting) {
  // Like the previous test, but the declaration for the copy-object
  // has its declaration hoisted.
  const auto assembly = OldStorageBufferPreamble() + R"(
  %bool = OpTypeBool
  %cond = OpConstantTrue %bool

  %100 = OpFunction %void None %voidfn

  %entry = OpLabel
  OpSelectionMerge %99 None
  OpBranchConditional %cond %20 %30

  %20 = OpLabel
  %1 = OpAccessChain %ptr_uint %myvar %uint_1 %uint_1
  ; this definintion dominates the use in %99
  %2 = OpCopyObject %ptr_uint %1
  OpBranch %99

  %30 = OpLabel
  OpReturn

  %99 = OpLabel
  OpStore %2 %uint_0
  OpReturn

  OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), Eq(R"(VariableDeclStatement{
  Variable{
    x_2
    function
    __ptr_storage_buffer__u32
  }
}
If{
  (
    ScalarConstructor{true}
  )
  {
    Assignment{
      Identifier{x_2}
      ArrayAccessor{
        MemberAccessor{
          Identifier{myvar}
          Identifier{field1}
        }
        ScalarConstructor{1}
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
  Identifier{x_2}
  ScalarConstructor{0}
}
Return{}
)")) << ToString(fe.ast_body())
     << p->error();
}

TEST_F(SpvParserTest, DISABLED_RemapStorageBuffer_ThroughFunctionCall) {
  // TODO(dneto): Blocked on OpFunctionCall support.
  // We might need this for passing pointers into atomic builtins.
}
TEST_F(SpvParserTest, DISABLED_RemapStorageBuffer_ThroughFunctionParameter) {
  // TODO(dneto): Blocked on OpFunctionCall support.
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
