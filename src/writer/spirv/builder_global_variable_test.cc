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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bool_literal.h"
#include "src/ast/builtin.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/location_decoration.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/storage_class.h"
#include "src/ast/struct.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, GlobalVar_NoStorageClass) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kNone, &f32);

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithStorageClass) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kOutput, &f32);

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithStorageClass_Input) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kInput, &f32);

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
)");
}

TEST_F(BuilderTest, GlobalVar_WithConstructor) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  auto* init = create<ast::TypeConstructorExpression>(&vec, std::move(vals));

  EXPECT_TRUE(td.DetermineResultType(init)) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  td.RegisterVariableForTesting(&v);

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %6 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
%7 = OpTypePointer Output %2
%6 = OpVariable %7 Output %5
)");
}

TEST_F(BuilderTest, GlobalVar_Const) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  auto* init = create<ast::TypeConstructorExpression>(&vec, std::move(vals));

  EXPECT_TRUE(td.DetermineResultType(init)) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);
  td.RegisterVariableForTesting(&v);

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %5 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

TEST_F(BuilderTest, GlobalVar_Complex_Constructor) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));
  auto* init = create<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  EXPECT_TRUE(td.DetermineResultType(init)) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);
  td.RegisterVariableForTesting(&v);

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
)");
}

TEST_F(BuilderTest, GlobalVar_Complex_ConstructorWithExtract) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec2(&f32, 2);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.0f)));
  auto* first = create<ast::TypeConstructorExpression>(&vec2, std::move(vals));

  vals.push_back(std::move(first));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  auto* init = create<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  EXPECT_TRUE(td.DetermineResultType(init)) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);
  td.RegisterVariableForTesting(&v);

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpTypeVector %2 2
%4 = OpConstant %2 1
%5 = OpConstant %2 2
%6 = OpConstantComposite %3 %4 %5
%8 = OpTypeInt 32 0
%9 = OpConstant %8 0
%7 = OpSpecConstantOp %2 CompositeExtract %6 9
%11 = OpConstant %8 1
%10 = OpSpecConstantOp %2 CompositeExtract %6 11
%12 = OpConstant %2 3
%13 = OpSpecConstantComposite %1 %7 %10 %12
)");
}

TEST_F(BuilderTest, GlobalVar_WithLocation) {
  ast::type::F32Type f32;
  auto* v = create<ast::Variable>("var", ast::StorageClass::kOutput, &f32);
  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(5, Source{}));

  ast::DecoratedVariable dv(std::move(v));
  dv.set_decorations(std::move(decos));

  EXPECT_TRUE(b.GenerateGlobalVariable(&dv)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Location 5
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithBindingAndSet) {
  ast::type::F32Type f32;
  auto* v = create<ast::Variable>("var", ast::StorageClass::kOutput, &f32);
  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(2, Source{}));
  decos.push_back(create<ast::SetDecoration>(3, Source{}));

  ast::DecoratedVariable dv(std::move(v));
  dv.set_decorations(std::move(decos));

  EXPECT_TRUE(b.GenerateGlobalVariable(&dv)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Binding 2
OpDecorate %1 DescriptorSet 3
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithBuiltin) {
  ast::type::F32Type f32;
  auto* v = create<ast::Variable>("var", ast::StorageClass::kOutput, &f32);
  ast::VariableDecorationList decos;
  decos.push_back(
      create<ast::BuiltinDecoration>(ast::Builtin::kPosition, Source{}));

  ast::DecoratedVariable dv(std::move(v));
  dv.set_decorations(std::move(decos));

  EXPECT_TRUE(b.GenerateGlobalVariable(&dv)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 BuiltIn Position
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_ConstantId_Bool) {
  ast::type::BoolType bool_type;

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::ConstantIdDecoration>(1200, Source{}));

  ast::DecoratedVariable v(
      create<ast::Variable>("var", ast::StorageClass::kNone, &bool_type));
  v.set_decorations(std::move(decos));
  v.set_constructor(create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true)));

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 1200
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpSpecConstantTrue %1
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
)");
}

TEST_F(BuilderTest, GlobalVar_ConstantId_Bool_NoConstructor) {
  ast::type::BoolType bool_type;

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::ConstantIdDecoration>(1200, Source{}));

  ast::DecoratedVariable v(
      create<ast::Variable>("var", ast::StorageClass::kNone, &bool_type));
  v.set_decorations(std::move(decos));

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %4 SpecId 1200
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeBool
%2 = OpTypePointer Private %3
%4 = OpSpecConstantFalse %3
%1 = OpVariable %2 Private %4
)");
}

TEST_F(BuilderTest, GlobalVar_ConstantId_Scalar) {
  ast::type::F32Type f32;

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::ConstantIdDecoration>(0, Source{}));

  ast::DecoratedVariable v(
      create<ast::Variable>("var", ast::StorageClass::kNone, &f32));
  v.set_decorations(std::move(decos));
  v.set_constructor(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.0)));

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 0
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpSpecConstant %1 2
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
)");
}

TEST_F(BuilderTest, GlobalVar_ConstantId_Scalar_F32_NoConstructor) {
  ast::type::F32Type f32;

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::ConstantIdDecoration>(0, Source{}));

  ast::DecoratedVariable v(
      create<ast::Variable>("var", ast::StorageClass::kNone, &f32));
  v.set_decorations(std::move(decos));

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %4 SpecId 0
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpSpecConstant %3 0
%1 = OpVariable %2 Private %4
)");
}

TEST_F(BuilderTest, GlobalVar_ConstantId_Scalar_I32_NoConstructor) {
  ast::type::I32Type i32;

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::ConstantIdDecoration>(0, Source{}));

  ast::DecoratedVariable v(
      create<ast::Variable>("var", ast::StorageClass::kNone, &i32));
  v.set_decorations(std::move(decos));

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %4 SpecId 0
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpSpecConstant %3 0
%1 = OpVariable %2 Private %4
)");
}

TEST_F(BuilderTest, GlobalVar_ConstantId_Scalar_U32_NoConstructor) {
  ast::type::U32Type u32;

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::ConstantIdDecoration>(0, Source{}));

  ast::DecoratedVariable v(
      create<ast::Variable>("var", ast::StorageClass::kNone, &u32));
  v.set_decorations(std::move(decos));

  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "tint_766172"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %4 SpecId 0
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 0
%2 = OpTypePointer Private %3
%4 = OpSpecConstant %3 0
%1 = OpVariable %2 Private %4
)");
}

struct BuiltinData {
  ast::Builtin builtin;
  SpvBuiltIn result;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
  out << data.builtin;
  return out;
}
using BuiltinDataTest = TestParamHelper<BuiltinData>;
TEST_P(BuiltinDataTest, Convert) {
  auto params = GetParam();
  EXPECT_EQ(b.ConvertBuiltin(params.builtin), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest_Type,
    BuiltinDataTest,
    testing::Values(
        BuiltinData{ast::Builtin::kNone, SpvBuiltInMax},
        BuiltinData{ast::Builtin::kPosition, SpvBuiltInPosition},
        BuiltinData{
            ast::Builtin::kVertexIdx,
            SpvBuiltInVertexIndex,
        },
        BuiltinData{ast::Builtin::kInstanceIdx, SpvBuiltInInstanceIndex},
        BuiltinData{ast::Builtin::kFrontFacing, SpvBuiltInFrontFacing},
        BuiltinData{ast::Builtin::kFragCoord, SpvBuiltInFragCoord},
        BuiltinData{ast::Builtin::kFragDepth, SpvBuiltInFragDepth},
        BuiltinData{ast::Builtin::kLocalInvocationId,
                    SpvBuiltInLocalInvocationId},
        BuiltinData{ast::Builtin::kLocalInvocationIdx,
                    SpvBuiltInLocalInvocationIndex},
        BuiltinData{ast::Builtin::kGlobalInvocationId,
                    SpvBuiltInGlobalInvocationId}));

TEST_F(BuilderTest, GlobalVar_DeclReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // var b : [[access(read)]] A

  ast::type::I32Type i32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("a", &i32, std::move(decos)));
  members.push_back(create<ast::StructMember>("b", &i32, std::move(decos)));

  ast::type::StructType A("A", create<ast::Struct>(std::move(members)));
  ast::type::AccessControlType ac{ast::AccessControl::kReadOnly, &A};

  ast::Variable var("b", ast::StorageClass::kStorageBuffer, &ac);

  EXPECT_TRUE(b.GenerateGlobalVariable(&var)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %3 0 NonWritable
OpMemberDecorate %3 1 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "tint_41"
OpMemberName %3 0 "tint_61"
OpMemberName %3 1 "tint_62"
OpName %1 "tint_62"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
)");
}

TEST_F(BuilderTest, GlobalVar_TypeAliasDeclReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // type B = A;
  // var b : [[access(read)]] B

  ast::type::I32Type i32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("a", &i32, std::move(decos)));

  ast::type::StructType A("A", create<ast::Struct>(std::move(members)));
  ast::type::AliasType B("B", &A);
  ast::type::AccessControlType ac{ast::AccessControl::kReadOnly, &B};

  ast::Variable var("b", ast::StorageClass::kStorageBuffer, &ac);

  EXPECT_TRUE(b.GenerateGlobalVariable(&var)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %3 0 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "tint_41"
OpMemberName %3 0 "tint_61"
OpName %1 "tint_62"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
)");
}

TEST_F(BuilderTest, GlobalVar_TypeAliasAssignReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // type B = [[access(read)]] A;
  // var b : B

  ast::type::I32Type i32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("a", &i32, std::move(decos)));

  ast::type::StructType A("A", create<ast::Struct>(std::move(members)));
  ast::type::AccessControlType ac{ast::AccessControl::kReadOnly, &A};
  ast::type::AliasType B("B", &ac);

  ast::Variable var("b", ast::StorageClass::kStorageBuffer, &B);

  EXPECT_TRUE(b.GenerateGlobalVariable(&var)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %3 0 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "tint_41"
OpMemberName %3 0 "tint_61"
OpName %1 "tint_62"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
)");
}

TEST_F(BuilderTest, GlobalVar_TwoVarDeclReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // var b : [[access(read)]] A
  // var c : [[access(read_write)]] A

  ast::type::I32Type i32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("a", &i32, std::move(decos)));

  ast::type::StructType A("A", create<ast::Struct>(std::move(members)));
  ast::type::AccessControlType read{ast::AccessControl::kReadOnly, &A};
  ast::type::AccessControlType rw{ast::AccessControl::kReadWrite, &A};

  ast::Variable var_b("b", ast::StorageClass::kStorageBuffer, &read);
  ast::Variable var_c("c", ast::StorageClass::kStorageBuffer, &rw);

  EXPECT_TRUE(b.GenerateGlobalVariable(&var_b)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(&var_c)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %3 0 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "tint_41"
OpMemberName %3 0 "tint_61"
OpName %1 "tint_62"
OpName %7 "tint_41"
OpMemberName %7 0 "tint_61"
OpName %5 "tint_63"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeStruct %4
%6 = OpTypePointer StorageBuffer %7
%5 = OpVariable %6 StorageBuffer
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
