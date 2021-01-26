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
#include "src/ast/float_literal.h"
#include "src/ast/group_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/storage_class.h"
#include "src/ast/struct.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/program.h"
#include "src/type/access_control_type.h"
#include "src/type/bool_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/struct_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
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
  auto* v = Var("var", ast::StorageClass::kNone, ty.f32);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithStorageClass) {
  auto* v = Var("var", ast::StorageClass::kOutput, ty.f32);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithStorageClass_Input) {
  auto* v = Var("var", ast::StorageClass::kInput, ty.f32);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
)");
}

TEST_F(BuilderTest, GlobalVar_WithConstructor) {
  auto* init = vec3<f32>(1.f, 1.f, 3.f);
  EXPECT_TRUE(td.DetermineResultType(init)) << td.error();

  auto* v = Var("var", ast::StorageClass::kOutput, ty.f32, init,
                ast::VariableDecorationList{});
  td.RegisterVariableForTesting(v);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %6 "var"
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
  auto* init = vec3<f32>(1.f, 1.f, 3.f);
  EXPECT_TRUE(td.DetermineResultType(init)) << td.error();

  auto* v = Const("var", ast::StorageClass::kOutput, ty.f32, init,
                  ast::VariableDecorationList{});
  td.RegisterVariableForTesting(v);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %5 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

TEST_F(BuilderTest, GlobalVar_Complex_Constructor) {
  auto* init = vec3<f32>(ast::ExpressionList{Expr(1.f), Expr(2.f), Expr(3.f)});
  EXPECT_TRUE(td.DetermineResultType(init)) << td.error();

  auto* v = Const("var", ast::StorageClass::kOutput, ty.f32, init,
                  ast::VariableDecorationList{});
  td.RegisterVariableForTesting(v);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
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
  auto* init = vec3<f32>(vec2<f32>(1.f, 2.f), 3.f);

  EXPECT_TRUE(td.DetermineResultType(init)) << td.error();

  auto* v = Const("var", ast::StorageClass::kOutput, ty.f32, init,
                  ast::VariableDecorationList{});
  td.RegisterVariableForTesting(v);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
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
  auto* v = Var("var", ast::StorageClass::kOutput, ty.f32, nullptr,
                ast::VariableDecorationList{
                    create<ast::LocationDecoration>(5),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Location 5
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithBindingAndGroup) {
  auto* v = Var("var", ast::StorageClass::kOutput, ty.f32, nullptr,
                ast::VariableDecorationList{
                    create<ast::BindingDecoration>(2),
                    create<ast::GroupDecoration>(3),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
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
  auto* v = Var("var", ast::StorageClass::kOutput, ty.f32, nullptr,
                ast::VariableDecorationList{
                    create<ast::BuiltinDecoration>(ast::Builtin::kPosition),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
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
  auto* v = Var("var", ast::StorageClass::kNone, ty.bool_, Expr(true),
                ast::VariableDecorationList{
                    create<ast::ConstantIdDecoration>(1200),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "var"
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
  auto* v = Var("var", ast::StorageClass::kNone, ty.bool_, nullptr,
                ast::VariableDecorationList{
                    create<ast::ConstantIdDecoration>(1200),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
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
  auto* v = Var("var", ast::StorageClass::kNone, ty.f32, Expr(2.f),
                ast::VariableDecorationList{
                    create<ast::ConstantIdDecoration>(0),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "var"
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
  auto* v = Var("var", ast::StorageClass::kNone, ty.f32, nullptr,
                ast::VariableDecorationList{
                    create<ast::ConstantIdDecoration>(0),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
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
  auto* v = Var("var", ast::StorageClass::kNone, ty.i32, nullptr,
                ast::VariableDecorationList{
                    create<ast::ConstantIdDecoration>(0),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
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
  auto* v = Var("var", ast::StorageClass::kNone, ty.u32, nullptr,
                ast::VariableDecorationList{
                    create<ast::ConstantIdDecoration>(0),
                });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
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
  spirv::Builder& b = Build();

  EXPECT_EQ(b.ConvertBuiltin(params.builtin), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest_Type,
    BuiltinDataTest,
    testing::Values(
        BuiltinData{ast::Builtin::kNone, SpvBuiltInMax},
        BuiltinData{ast::Builtin::kPosition, SpvBuiltInPosition},
        BuiltinData{
            ast::Builtin::kVertexIndex,
            SpvBuiltInVertexIndex,
        },
        BuiltinData{ast::Builtin::kInstanceIndex, SpvBuiltInInstanceIndex},
        BuiltinData{ast::Builtin::kFrontFacing, SpvBuiltInFrontFacing},
        BuiltinData{ast::Builtin::kFragCoord, SpvBuiltInFragCoord},
        BuiltinData{ast::Builtin::kFragDepth, SpvBuiltInFragDepth},
        BuiltinData{ast::Builtin::kLocalInvocationId,
                    SpvBuiltInLocalInvocationId},
        BuiltinData{ast::Builtin::kLocalInvocationIndex,
                    SpvBuiltInLocalInvocationIndex},
        BuiltinData{ast::Builtin::kGlobalInvocationId,
                    SpvBuiltInGlobalInvocationId}));

TEST_F(BuilderTest, GlobalVar_DeclReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // var b : [[access(read)]] A

  auto* A = ty.struct_(
      "A", create<ast::Struct>(
               ast::StructMemberList{Member("a", ty.i32), Member("b", ty.i32)},
               ast::StructDecorationList{}));
  type::AccessControl ac{ast::AccessControl::kReadOnly, A};

  auto* var = Var("b", ast::StorageClass::kStorage, &ac);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %3 0 NonWritable
OpMemberDecorate %3 1 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpMemberName %3 1 "b"
OpName %1 "b"
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

  auto* A = ty.struct_(
      "A", create<ast::Struct>(ast::StructMemberList{Member("a", ty.i32)},
                               ast::StructDecorationList{}));
  auto* B = ty.alias("B", A);
  type::AccessControl ac{ast::AccessControl::kReadOnly, B};
  auto* var = Var("b", ast::StorageClass::kStorage, &ac);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %3 0 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpName %1 "b"
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

  auto* A = ty.struct_(
      "A", create<ast::Struct>(ast::StructMemberList{Member("a", ty.i32)},
                               ast::StructDecorationList{}));
  type::AccessControl ac{ast::AccessControl::kReadOnly, A};
  auto* B = ty.alias("B", &ac);
  auto* var = Var("b", ast::StorageClass::kStorage, B);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpMemberDecorate %3 0 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpName %1 "b"
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

  auto* A = ty.struct_(
      "A", create<ast::Struct>(ast::StructMemberList{Member("a", ty.i32)},
                               ast::StructDecorationList{}));
  type::AccessControl read{ast::AccessControl::kReadOnly, A};
  type::AccessControl rw{ast::AccessControl::kReadWrite, A};

  auto* var_b = Var("b", ast::StorageClass::kStorage, &read);
  auto* var_c = Var("c", ast::StorageClass::kStorage, &rw);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var_b)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(var_c)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()),
            R"(OpMemberDecorate %3 0 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpName %1 "b"
OpName %7 "A"
OpMemberName %7 0 "a"
OpName %5 "c"
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

TEST_F(BuilderTest, GlobalVar_TextureStorageReadOnly) {
  // var<uniform_constant> a : [[access(read)]] texture_storage_2d<r32uint>;

  type::StorageTexture type(type::TextureDimension::k2d,
                            type::ImageFormat::kR32Uint);
  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&type)) << td.error();

  type::AccessControl ac(ast::AccessControl::kReadOnly, &type);

  auto* var_a = Var("a", ast::StorageClass::kUniformConstant, &ac);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 NonWritable
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 R32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
)");
}

TEST_F(BuilderTest, GlobalVar_TextureStorageWriteOnly) {
  // var<uniform_constant> a : [[access(write)]] texture_storage_2d<r32uint>;

  type::StorageTexture type(type::TextureDimension::k2d,
                            type::ImageFormat::kR32Uint);
  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&type)) << td.error();

  type::AccessControl ac(ast::AccessControl::kWriteOnly, &type);

  auto* var_a = Var("a", ast::StorageClass::kUniformConstant, &ac);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 NonReadable
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 R32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
)");
}

// Check that multiple texture_storage types with different access modifiers
// only produces a single OpTypeImage.
TEST_F(BuilderTest, GlobalVar_TextureStorageWithDifferentAccess) {
  // var<uniform_constant> a : [[access(read)]] texture_storage_2d<r32uint>;
  // var<uniform_constant> b : [[access(write)]] texture_storage_2d<r32uint>;

  type::StorageTexture st(type::TextureDimension::k2d,
                          type::ImageFormat::kR32Uint);
  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&st)) << td.error();

  type::AccessControl type_a(ast::AccessControl::kReadOnly, &st);
  auto* var_a = Var("a", ast::StorageClass::kUniformConstant, &type_a);

  type::AccessControl type_b(ast::AccessControl::kWriteOnly, &st);
  auto* var_b = Var("b", ast::StorageClass::kUniformConstant, &type_b);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(var_b)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 NonWritable
OpDecorate %5 NonReadable
)");
  // There must only be one OpTypeImage declaration with the same
  // arguments
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 R32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%6 = OpTypePointer UniformConstant %3
%5 = OpVariable %6 UniformConstant
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
