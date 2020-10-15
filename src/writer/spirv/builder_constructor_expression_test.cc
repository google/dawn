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

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/ast/binary_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Constructor_Const) {
  ast::type::F32Type f32;
  auto fl = std::make_unique<ast::FloatLiteral>(&f32, 42.2f);
  ast::ScalarConstructorExpression c(std::move(fl));

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &c, true), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 42.2000008
)");
}

TEST_F(BuilderTest, Constructor_Type) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &t, true), 5u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_WithCasts) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType vec(&f32, 2);

  ast::ExpressionList type_vals;
  type_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(type_vals)));

  type_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(type_vals)));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 7u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%4 = OpTypeInt 32 1
%5 = OpConstant %4 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%3 = OpConvertSToF %2 %5
%6 = OpConvertSToF %2 %5
%7 = OpCompositeConstruct %1 %3 %6
)");
}

TEST_F(BuilderTest, Constructor_Type_WithAlias) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  // type Int = i32
  // cast<Int>(1.f)

  ast::type::AliasType alias("Int", &i32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3)));

  ast::TypeConstructorExpression cast(&alias, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.29999995
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertFToS %2 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_IdentifierExpression_Param) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);

  auto var = std::make_unique<ast::Variable>(
      "ident", ast::StorageClass::kFunction, &f32);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::IdentifierExpression>("ident"));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateExpression(&t), 8u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
%5 = OpTypeVector %3 2
%6 = OpConstant %3 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %4
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpCompositeConstruct %5 %6 %7
)");
}

TEST_F(BuilderTest, Constructor_Vector_Bitcast_Params) {
  ast::type::I32Type i32;
  ast::type::U32Type u32;
  ast::type::VectorType vec(&u32, 2);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 7u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 2
%3 = OpTypeInt 32 1
%4 = OpConstant %3 1
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpBitcast %2 %4
%6 = OpBitcast %2 %4
%7 = OpCompositeConstruct %1 %5 %6
)");
}

TEST_F(BuilderTest, Constructor_Type_NonConst_Value_Fails) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  auto rel = std::make_unique<ast::BinaryExpression>(
      ast::BinaryOp::kAdd,
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 3.0f)),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::move(rel));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &t, true), 0u);
  EXPECT_TRUE(b.has_error());
  EXPECT_EQ(b.error(), R"(constructor must be a constant expression)");
}

TEST_F(BuilderTest, Constructor_Type_Bool_With_Bool) {
  ast::type::BoolType bool_type;

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::BoolLiteral>(&bool_type, true)));

  ast::TypeConstructorExpression t(&bool_type, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 1u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeBool
%3 = OpConstantTrue %2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpCopyObject %2 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_I32_With_I32) {
  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::TypeConstructorExpression cast(&i32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpCopyObject %2 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_U32_With_U32) {
  ast::type::U32Type u32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 2)));

  ast::TypeConstructorExpression cast(&u32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpConstant %2 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpCopyObject %2 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_F32_With_F32) {
  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&f32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpConstant %2 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpCopyObject %2 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec2_With_F32_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 4u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec3_With_F32_F32_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 4u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec3_With_F32_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));

  ast::TypeConstructorExpression cast(&vec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %3 %6 %7
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec3_With_Vec2_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %6 %7 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec4_With_F32_F32_F32_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 4);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 4u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec4_With_F32_F32_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %3 %3 %6 %7
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec4_With_F32_Vec2_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %3 %6 %7 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec4_With_Vec2_F32_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeConstruct %1 %6 %7 %4 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec4_With_Vec2_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec2_params)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 10u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeExtract %2 %5 0
%9 = OpCompositeExtract %2 %5 1
%10 = OpCompositeConstruct %1 %6 %7 %8 %9
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec4_With_F32_Vec3) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vec_params)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 3
%5 = OpConstantComposite %4 %3 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeExtract %2 %5 2
%9 = OpCompositeConstruct %1 %3 %6 %7 %8
)");
}

TEST_F(BuilderTest, Constructor_Type_Vec4_With_Vec3_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vec_params)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 3
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4 %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpCompositeExtract %2 %5 0
%7 = OpCompositeExtract %2 %5 1
%8 = OpCompositeExtract %2 %5 2
%9 = OpCompositeConstruct %1 %6 %7 %8 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_ModuleScope_Vec3_With_F32_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));

  ast::TypeConstructorExpression cast(&vec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &cast, true), 11u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %3 %6 %9
)");
}

TEST_F(BuilderTest, Constructor_Type_ModuleScope_Vec3_With_Vec2_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &cast, true), 11u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %6 %9 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_ModuleScope_Vec4_With_F32_F32_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &cast, true), 11u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %3 %3 %6 %9
)");
}

TEST_F(BuilderTest, Constructor_Type_ModuleScope_Vec4_With_F32_Vec2_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &cast, true), 11u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 2
%5 = OpConstantComposite %4 %3 %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %3 %6 %9 %3
)");
}

TEST_F(BuilderTest, Constructor_Type_ModuleScope_Vec4_With_Vec2_F32_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &cast, true), 11u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantComposite %1 %6 %9 %4 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_ModuleScope_Vec4_With_Vec2_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vec2_params)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &cast, true), 13u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 2
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%11 = OpSpecConstantOp %2 CompositeExtract %5 8
%12 = OpSpecConstantOp %2 CompositeExtract %5 10
%13 = OpSpecConstantComposite %1 %6 %9 %11 %12
)");
}

TEST_F(BuilderTest, Constructor_Type_ModuleScope_Vec4_With_F32_Vec3) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vec_params)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &cast, true), 13u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpConstant %2 2
%4 = OpTypeVector %2 3
%5 = OpConstantComposite %4 %3 %3 %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%12 = OpConstant %7 2
%11 = OpSpecConstantOp %2 CompositeExtract %5 12
%13 = OpSpecConstantComposite %1 %3 %6 %9 %11
)");
}

TEST_F(BuilderTest, Constructor_Type_ModuleScope_Vec4_With_Vec3_F32) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec4(&f32, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vec_params)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&vec4, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateConstructorExpression(nullptr, &cast, true), 13u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 4
%3 = OpTypeVector %2 3
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4 %4
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%6 = OpSpecConstantOp %2 CompositeExtract %5 8
%10 = OpConstant %7 1
%9 = OpSpecConstantOp %2 CompositeExtract %5 10
%12 = OpConstant %7 2
%11 = OpSpecConstantOp %2 CompositeExtract %5 12
%13 = OpSpecConstantComposite %1 %6 %9 %11 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat2x2_With_Vec2_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::MatrixType mat(&f32, 2, 2);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat3x2_With_Vec2_Vec2_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::MatrixType mat(&f32, 2, 3);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec3_params;
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec3_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat4x2_With_Vec2_Vec2_Vec2_Vec2) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::MatrixType mat(&f32, 2, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec3_params;
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec4_params;
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec3_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec4_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat2x3_With_Vec3_Vec3) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::MatrixType mat(&f32, 3, 2);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat3x3_With_Vec3_Vec3_Vec3) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::MatrixType mat(&f32, 3, 3);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec3_params;
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec3_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat4x3_With_Vec3_Vec3_Vec3_Vec3) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::MatrixType mat(&f32, 3, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec3_params;
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec4_params;
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec3_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec4_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat2x4_With_Vec4_Vec4) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 4);
  ast::type::MatrixType mat(&f32, 4, 2);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 2
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat3x4_With_Vec4_Vec4_Vec4) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 4);
  ast::type::MatrixType mat(&f32, 4, 3);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec3_params;
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec3_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 3
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Mat4x4_With_Vec4_Vec4_Vec4_Vec4) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 4);
  ast::type::MatrixType mat(&f32, 4, 4);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec3_params;
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec3_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec4_params;
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec4_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec3_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec4_params)));

  ast::TypeConstructorExpression cast(&mat, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 4
%1 = OpTypeMatrix %2 4
%4 = OpConstant %3 2
%5 = OpConstantComposite %2 %4 %4 %4 %4
%6 = OpConstantComposite %1 %5 %5 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Array_5_F32) {
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 5);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::TypeConstructorExpression cast(&ary, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 6u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 0
%4 = OpConstant %3 5
%1 = OpTypeArray %2 %4
%5 = OpConstant %2 2
%6 = OpConstantComposite %1 %5 %5 %5 %5 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_Array_2_Vec3) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::ArrayType ary(&vec, 2);

  ast::ExpressionList vec_params;
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList vec2_params;
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec2_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_params)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec2_params)));

  ast::TypeConstructorExpression cast(&ary, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%4 = OpTypeInt 32 0
%5 = OpConstant %4 2
%1 = OpTypeArray %2 %5
%6 = OpConstant %3 2
%7 = OpConstantComposite %2 %6 %6 %6
%8 = OpConstantComposite %1 %7 %7
)");
}

TEST_F(BuilderTest, Constructor_Type_Struct) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &vec, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(std::move(members));
  ast::type::StructType s_type("my_struct", std::move(s));

  ast::ExpressionList vec_vals;
  vec_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));
  vec_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));
  vec_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));
  vals.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_vals)));

  ast::TypeConstructorExpression t(&s_type, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 6u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeVector %2 3
%1 = OpTypeStruct %2 %3
%4 = OpConstant %2 2
%5 = OpConstantComposite %3 %4 %4 %4
%6 = OpConstantComposite %1 %4 %5
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit_F32) {
  ast::type::F32Type f32;

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&f32, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit_I32) {
  ast::type::I32Type i32;

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&i32, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit_U32) {
  ast::type::U32Type u32;

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&u32, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit_Bool) {
  ast::type::BoolType bool_type;

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&bool_type, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 2u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit_Vector) {
  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 2);

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 3u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 2
%3 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 2, 4);

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&mat, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 4u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%1 = OpTypeMatrix %2 4
%4 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit_Array) {
  ast::type::I32Type i32;
  ast::type::ArrayType ary(&i32, 2);

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&ary, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 5u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
%1 = OpTypeArray %2 %4
%5 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_ZeroInit_Struct) {
  ast::type::F32Type f32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(std::move(members));
  ast::type::StructType s_type("my_struct", std::move(s));

  ast::ExpressionList vals;
  ast::TypeConstructorExpression t(&s_type, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});

  EXPECT_EQ(b.GenerateExpression(&t), 3u);
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2
%3 = OpConstantNull %1
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_U32_To_I32) {
  ast::type::U32Type u32;
  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 2)));

  ast::TypeConstructorExpression cast(&i32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpBitcast %2 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_I32_To_U32) {
  ast::type::U32Type u32;
  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::TypeConstructorExpression cast(&u32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpTypeInt 32 1
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpBitcast %2 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_F32_To_I32) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.4)));

  ast::TypeConstructorExpression cast(&i32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.4000001
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertFToS %2 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_F32_To_U32) {
  ast::type::U32Type u32;
  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.4)));

  ast::TypeConstructorExpression cast(&u32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.4000001
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertFToU %2 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_I32_To_F32) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::TypeConstructorExpression cast(&f32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 1
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertSToF %2 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_U32_To_F32) {
  ast::type::U32Type u32;
  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 2)));

  ast::TypeConstructorExpression cast(&f32, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertUToF %2 %4
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_Vectors_U32_to_I32) {
  ast::type::U32Type u32;
  ast::type::VectorType uvec3(&u32, 3);
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &uvec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("i"));

  ast::TypeConstructorExpression cast(&ivec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateExpression(&cast), 6u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = OpBitcast %7 %9
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_Vectors_F32_to_I32) {
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::F32Type f32;
  ast::type::VectorType fvec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &fvec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("i"));

  ast::TypeConstructorExpression cast(&ivec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateExpression(&cast), 6u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = OpConvertFToS %7 %9
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_Vectors_I32_to_U32) {
  ast::type::U32Type u32;
  ast::type::VectorType uvec3(&u32, 3);
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &ivec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("i"));

  ast::TypeConstructorExpression cast(&uvec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateExpression(&cast), 6u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = OpBitcast %7 %9
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_Vectors_F32_to_U32) {
  ast::type::U32Type u32;
  ast::type::VectorType uvec3(&u32, 3);
  ast::type::F32Type f32;
  ast::type::VectorType fvec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &fvec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("i"));

  ast::TypeConstructorExpression cast(&uvec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateExpression(&cast), 6u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = OpConvertFToU %7 %9
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_Vectors_I32_to_F32) {
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::F32Type f32;
  ast::type::VectorType fvec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &ivec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("i"));

  ast::TypeConstructorExpression cast(&fvec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateExpression(&cast), 6u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = OpConvertSToF %7 %9
)");
}

TEST_F(BuilderTest, Constructor_Type_Convert_Vectors_U32_to_F32) {
  ast::type::U32Type u32;
  ast::type::VectorType uvec3(&u32, 3);
  ast::type::F32Type f32;
  ast::type::VectorType fvec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &uvec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("i"));

  ast::TypeConstructorExpression cast(&fvec3, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateExpression(&cast), 6u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = OpConvertUToF %7 %9
)");
}

TEST_F(BuilderTest, IsConstructorConst_GlobalVectorWithAllConstConstructors) {
  // vec3<f32>(1.0, 2.0, 3.0)  -> true
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));
  ast::TypeConstructorExpression t(&vec, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_TRUE(b.is_constructor_const(&t, true));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_GlobalVector_WithIdent) {
  // vec3<f32>(a, b, c)  -> false -- ERROR
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("a"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("b"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("c"));
  ast::TypeConstructorExpression t(&vec, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::Variable var_a("a", ast::StorageClass::kPrivate, &f32);
  ast::Variable var_b("b", ast::StorageClass::kPrivate, &f32);
  ast::Variable var_c("c", ast::StorageClass::kPrivate, &f32);
  td.RegisterVariableForTesting(&var_a);
  td.RegisterVariableForTesting(&var_b);
  td.RegisterVariableForTesting(&var_c);

  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_FALSE(b.is_constructor_const(&t, true));
  EXPECT_TRUE(b.has_error());
  EXPECT_EQ(b.error(), "constructor must be a constant expression");
}

TEST_F(BuilderTest, IsConstructorConst_GlobalArrayWithAllConstConstructors) {
  // array<vec3<f32>, 2>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(1.0, 2.0, 3.0))
  //   -> true
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::ArrayType ary(&vec, 2);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));
  auto first =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(params));

  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));
  auto second =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(params));

  ast::ExpressionList ary_params;
  ary_params.push_back(std::move(first));
  ary_params.push_back(std::move(second));
  ast::TypeConstructorExpression t(&ary, std::move(ary_params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_TRUE(b.is_constructor_const(&t, true));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest,
       IsConstructorConst_GlobalVectorWithMatchingTypeConstructors) {
  // vec3<f32>(f32(1.0), f32(2.0))  -> false
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);

  ast::ExpressionList vec_params;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vec_params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(params)));

  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));
  vec_params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(params)));

  ast::TypeConstructorExpression t(&vec, std::move(vec_params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_FALSE(b.is_constructor_const(&t, true));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_GlobalWithTypeCastConstructor) {
  // vec3<f32>(f32(1), f32(2)) -> false
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vec_params;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vec_params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(params)));

  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  vec_params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(params)));

  ast::TypeConstructorExpression t(&vec, std::move(vec_params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_FALSE(b.is_constructor_const(&t, true));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_VectorWithAllConstConstructors) {
  // vec3<f32>(1.0, 2.0, 3.0)  -> true
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));
  ast::TypeConstructorExpression t(&vec, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_TRUE(b.is_constructor_const(&t, false));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_Vector_WithIdent) {
  // vec3<f32>(a, b, c)  -> false
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("a"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("b"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("c"));
  ast::TypeConstructorExpression t(&vec, std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::Variable var_a("a", ast::StorageClass::kPrivate, &f32);
  ast::Variable var_b("b", ast::StorageClass::kPrivate, &f32);
  ast::Variable var_c("c", ast::StorageClass::kPrivate, &f32);
  td.RegisterVariableForTesting(&var_a);
  td.RegisterVariableForTesting(&var_b);
  td.RegisterVariableForTesting(&var_c);

  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_FALSE(b.is_constructor_const(&t, false));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_ArrayWithAllConstConstructors) {
  // array<vec3<f32>, 2>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(1.0, 2.0, 3.0))
  //   -> true
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::ArrayType ary(&vec, 2);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));
  auto first =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(params));

  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));
  auto second =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(params));

  ast::ExpressionList ary_params;
  ary_params.push_back(std::move(first));
  ary_params.push_back(std::move(second));
  ast::TypeConstructorExpression t(&ary, std::move(ary_params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_TRUE(b.is_constructor_const(&t, false));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_VectorWith_TypeCastConstConstructors) {
  // vec2<f32>(f32(1.0), f32(2.0))  -> false
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType vec(&f32, 2);

  ast::ExpressionList vec_params;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vec_params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(params)));

  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  vec_params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(params)));

  ast::TypeConstructorExpression t(&vec, std::move(vec_params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_FALSE(b.is_constructor_const(&t, false));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_WithTypeCastConstructor) {
  // vec3<f32>(f32(1), f32(2)) -> false
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vec_params;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vec_params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(params)));

  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  vec_params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &f32, std::move(params)));

  ast::TypeConstructorExpression t(&vec, std::move(vec_params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_FALSE(b.is_constructor_const(&t, false));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_BitCastScalars) {
  ast::type::I32Type i32;
  ast::type::U32Type u32;
  ast::type::VectorType vec(&u32, 2);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::TypeConstructorExpression t(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_FALSE(b.is_constructor_const(&t, false));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_Struct) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &vec, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(std::move(members));
  ast::type::StructType s_type("my_struct", std::move(s));

  ast::ExpressionList vec_vals;
  vec_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));
  vec_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));
  vec_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));
  vals.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_vals)));

  ast::TypeConstructorExpression t(&s_type, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_TRUE(b.is_constructor_const(&t, false));
  EXPECT_FALSE(b.has_error());
}

TEST_F(BuilderTest, IsConstructorConst_Struct_WithIdentSubExpression) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &f32, std::move(decos)));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &vec, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(std::move(members));
  ast::type::StructType s_type("my_struct", std::move(s));

  ast::ExpressionList vec_vals;
  vec_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));
  vec_vals.push_back(std::make_unique<ast::IdentifierExpression>("a"));
  vec_vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2)));
  vals.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vec_vals)));

  ast::TypeConstructorExpression t(&s_type, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ast::Variable var_a("a", ast::StorageClass::kPrivate, &f32);
  ast::Variable var_b("b", ast::StorageClass::kPrivate, &f32);
  td.RegisterVariableForTesting(&var_a);
  td.RegisterVariableForTesting(&var_b);

  ASSERT_TRUE(td.DetermineResultType(&t)) << td.error();

  Builder b(&mod);
  EXPECT_FALSE(b.is_constructor_const(&t, false));
  EXPECT_FALSE(b.has_error());
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
