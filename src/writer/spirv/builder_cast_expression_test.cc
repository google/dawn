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

#include "gtest/gtest.h"
#include "src/ast/cast_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
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

TEST_F(BuilderTest, Cast_FloatToU32) {
  ast::type::U32Type u32;
  ast::type::F32Type f32;

  ast::CastExpression cast(&u32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::FloatLiteral>(&f32, 2.4)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.4000001
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertFToU %2 %4
)");
}

TEST_F(BuilderTest, Cast_FloatToI32) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::CastExpression cast(&i32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::FloatLiteral>(&f32, 2.4)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.4000001
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertFToS %2 %4
)");
}

TEST_F(BuilderTest, Cast_I32ToFloat) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::CastExpression cast(&f32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::SintLiteral>(&i32, 2)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 1
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertSToF %2 %4
)");
}

TEST_F(BuilderTest, Cast_U32ToFloat) {
  ast::type::U32Type u32;
  ast::type::F32Type f32;

  ast::CastExpression cast(&f32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::UintLiteral>(&u32, 2)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertUToF %2 %4
)");
}

TEST_F(BuilderTest, Cast_WithLoad) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  // var i : i32 = 1;
  // cast<f32>(i);
  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &i32);

  ast::CastExpression cast(&f32,
                           std::make_unique<ast::IdentifierExpression>("i"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCastExpression(&cast), 5u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeFloat 32
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = OpConvertSToF %6 %7
)");
}

TEST_F(BuilderTest, Cast_WithAlias) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  // type Int = i32
  // cast<Int>(1.f)

  ast::type::AliasType alias("Int", &i32);

  ast::CastExpression cast(&alias,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::FloatLiteral>(&f32, 2.3)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeFloat 32
%4 = OpConstant %3 2.29999995
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpConvertFToS %2 %4
)");
}

TEST_F(BuilderTest, Cast_I32ToU32) {
  ast::type::U32Type u32;
  ast::type::I32Type i32;

  ast::CastExpression cast(&u32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::SintLiteral>(&i32, 2)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpTypeInt 32 1
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpBitcast %2 %4
)");
}

TEST_F(BuilderTest, Cast_U32ToI32) {
  ast::type::U32Type u32;
  ast::type::I32Type i32;

  ast::CastExpression cast(&i32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::UintLiteral>(&u32, 2)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpTypeInt 32 0
%4 = OpConstant %3 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpBitcast %2 %4
)");
}

TEST_F(BuilderTest, Cast_I32ToI32) {
  ast::type::I32Type i32;

  ast::CastExpression cast(&i32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::SintLiteral>(&i32, 2)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpCopyObject %2 %3
)");
}

TEST_F(BuilderTest, Cast_U32ToU32) {
  ast::type::U32Type u32;

  ast::CastExpression cast(&u32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::UintLiteral>(&u32, 2)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 0
%3 = OpConstant %2 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpCopyObject %2 %3
)");
}

TEST_F(BuilderTest, Cast_F32ToF32) {
  ast::type::F32Type f32;

  ast::CastExpression cast(&f32,
                           std::make_unique<ast::ScalarConstructorExpression>(
                               std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateCastExpression(&cast), 1u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%3 = OpConstant %2 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%1 = OpCopyObject %2 %3
)");
}

TEST_F(BuilderTest, Cast_Vectors_I32_to_F32) {
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::F32Type f32;
  ast::type::VectorType fvec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &ivec3);

  ast::CastExpression cast(&fvec3,
                           std::make_unique<ast::IdentifierExpression>("i"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCastExpression(&cast), 6u) << b.error();

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

TEST_F(BuilderTest, Cast_Vectors_U32_to_F32) {
  ast::type::U32Type u32;
  ast::type::VectorType uvec3(&u32, 3);
  ast::type::F32Type f32;
  ast::type::VectorType fvec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &uvec3);

  ast::CastExpression cast(&fvec3,
                           std::make_unique<ast::IdentifierExpression>("i"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCastExpression(&cast), 6u) << b.error();

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

TEST_F(BuilderTest, Cast_Vectors_F32_to_I32) {
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);
  ast::type::F32Type f32;
  ast::type::VectorType fvec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &fvec3);

  ast::CastExpression cast(&ivec3,
                           std::make_unique<ast::IdentifierExpression>("i"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCastExpression(&cast), 6u) << b.error();

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

TEST_F(BuilderTest, Cast_Vectors_F32_to_U32) {
  ast::type::U32Type u32;
  ast::type::VectorType uvec3(&u32, 3);
  ast::type::F32Type f32;
  ast::type::VectorType fvec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &fvec3);

  ast::CastExpression cast(&uvec3,
                           std::make_unique<ast::IdentifierExpression>("i"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCastExpression(&cast), 6u) << b.error();

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

TEST_F(BuilderTest, Cast_Vectors_U32_to_U32) {
  ast::type::U32Type u32;
  ast::type::VectorType uvec3(&u32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &uvec3);

  ast::CastExpression cast(&uvec3,
                           std::make_unique<ast::IdentifierExpression>("i"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCastExpression(&cast), 6u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = OpCopyObject %3 %7
)");
}

TEST_F(BuilderTest, Cast_Vectors_I32_to_U32) {
  ast::type::U32Type u32;
  ast::type::VectorType uvec3(&u32, 3);
  ast::type::I32Type i32;
  ast::type::VectorType ivec3(&i32, 3);

  auto var =
      std::make_unique<ast::Variable>("i", ast::StorageClass::kPrivate, &ivec3);

  ast::CastExpression cast(&uvec3,
                           std::make_unique<ast::IdentifierExpression>("i"));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  ASSERT_TRUE(td.DetermineResultType(&cast)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCastExpression(&cast), 6u) << b.error();

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

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
