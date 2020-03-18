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
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/bool_literal.h"
#include "src/ast/const_initializer_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/int_literal.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/uint_literal.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using GeneratorImplTest = testing::Test;

TEST_F(GeneratorImplTest, EmitInitializer_Bool) {
  auto lit = std::make_unique<ast::BoolLiteral>(false);
  ast::ConstInitializerExpression expr(std::move(lit));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "false");
}

TEST_F(GeneratorImplTest, EmitInitializer_Int) {
  auto lit = std::make_unique<ast::IntLiteral>(-12345);
  ast::ConstInitializerExpression expr(std::move(lit));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "-12345");
}

TEST_F(GeneratorImplTest, EmitInitializer_UInt) {
  auto lit = std::make_unique<ast::UintLiteral>(56779);
  ast::ConstInitializerExpression expr(std::move(lit));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "56779u");
}

TEST_F(GeneratorImplTest, EmitInitializer_Float) {
  auto lit = std::make_unique<ast::FloatLiteral>(1.5e27);
  ast::ConstInitializerExpression expr(std::move(lit));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "1.49999995e+27");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Float) {
  ast::type::F32Type f32;

  auto lit = std::make_unique<ast::FloatLiteral>(-1.2e-5);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit)));

  ast::TypeInitializerExpression expr(&f32, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "f32(-1.20000004e-05)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Bool) {
  ast::type::BoolType b;

  auto lit = std::make_unique<ast::BoolLiteral>(true);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit)));

  ast::TypeInitializerExpression expr(&b, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "bool(true)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Int) {
  ast::type::I32Type i32;

  auto lit = std::make_unique<ast::IntLiteral>(-12345);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit)));

  ast::TypeInitializerExpression expr(&i32, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "i32(-12345)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Uint) {
  ast::type::U32Type u32;

  auto lit = std::make_unique<ast::UintLiteral>(12345);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit)));

  ast::TypeInitializerExpression expr(&u32, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "u32(12345u)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Vec) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  auto lit1 = std::make_unique<ast::FloatLiteral>(1.f);
  auto lit2 = std::make_unique<ast::FloatLiteral>(2.f);
  auto lit3 = std::make_unique<ast::FloatLiteral>(3.f);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit1)));
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit2)));
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit3)));

  ast::TypeInitializerExpression expr(&vec, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "vec3<f32>(1.00000000, 2.00000000, 3.00000000)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Mat) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 2);

  ast::type::VectorType vec(&f32, 2);

  std::vector<std::unique_ptr<ast::Expression>> mat_values;

  for (size_t i = 0; i < 3; i++) {
    auto lit1 = std::make_unique<ast::FloatLiteral>(1.f + (i * 2));
    auto lit2 = std::make_unique<ast::FloatLiteral>(2.f + (i * 2));

    std::vector<std::unique_ptr<ast::Expression>> values;
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit1)));
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit2)));

    mat_values.push_back(std::make_unique<ast::TypeInitializerExpression>(
        &vec, std::move(values)));
  }

  ast::TypeInitializerExpression expr(&mat, std::move(mat_values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(),
            std::string("mat2x3<f32>(vec2<f32>(1.00000000, 2.00000000), ") +
                "vec2<f32>(3.00000000, 4.00000000), " +
                "vec2<f32>(5.00000000, 6.00000000))");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Array) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::ArrayType ary(&vec, 3);

  std::vector<std::unique_ptr<ast::Expression>> ary_values;

  for (size_t i = 0; i < 3; i++) {
    auto lit1 = std::make_unique<ast::FloatLiteral>(1.f + (i * 3));
    auto lit2 = std::make_unique<ast::FloatLiteral>(2.f + (i * 3));
    auto lit3 = std::make_unique<ast::FloatLiteral>(3.f + (i * 3));

    std::vector<std::unique_ptr<ast::Expression>> values;
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit1)));
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit2)));
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit3)));

    ary_values.push_back(std::make_unique<ast::TypeInitializerExpression>(
        &vec, std::move(values)));
  }

  ast::TypeInitializerExpression expr(&ary, std::move(ary_values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), std::string("array<vec3<f32>, 3>(") +
                            "vec3<f32>(1.00000000, 2.00000000, 3.00000000), " +
                            "vec3<f32>(4.00000000, 5.00000000, 6.00000000), " +
                            "vec3<f32>(7.00000000, 8.00000000, 9.00000000))");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
