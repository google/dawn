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
})"));
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
                             "for structure %55 having 2 elements"));
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
              HasSubstr("Access chain with unknown pointee type %60 void"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
