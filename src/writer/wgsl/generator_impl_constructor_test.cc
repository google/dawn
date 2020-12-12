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
#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/uint_literal.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitConstructor_Bool) {
  ast::type::Bool bool_type;
  auto* lit = create<ast::BoolLiteral>(Source{}, &bool_type, false);
  ast::ScalarConstructorExpression expr(lit);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "false");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Int) {
  ast::type::I32 i32;
  auto* lit = create<ast::SintLiteral>(Source{}, &i32, -12345);
  ast::ScalarConstructorExpression expr(lit);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "-12345");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_UInt) {
  ast::type::U32 u32;
  auto* lit = create<ast::UintLiteral>(Source{}, &u32, 56779);
  ast::ScalarConstructorExpression expr(lit);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "56779u");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Float) {
  ast::type::F32 f32;
  // Use a number close to 1<<30 but whose decimal representation ends in 0.
  auto* lit = create<ast::FloatLiteral>(Source{}, &f32,
                                        static_cast<float>((1 << 30) - 4));
  ast::ScalarConstructorExpression expr(lit);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "1073741824.0");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Float) {
  ast::type::F32 f32;

  auto* lit = create<ast::FloatLiteral>(Source{}, &f32, -1.2e-5);
  ast::ExpressionList values;
  values.push_back(create<ast::ScalarConstructorExpression>(lit));

  ast::TypeConstructorExpression expr(&f32, values);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "f32(-0.000012)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Bool) {
  ast::type::Bool b;

  auto* lit = create<ast::BoolLiteral>(Source{}, &b, true);
  ast::ExpressionList values;
  values.push_back(create<ast::ScalarConstructorExpression>(lit));

  ast::TypeConstructorExpression expr(&b, values);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "bool(true)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Int) {
  ast::type::I32 i32;

  auto* lit = create<ast::SintLiteral>(Source{}, &i32, -12345);
  ast::ExpressionList values;
  values.push_back(create<ast::ScalarConstructorExpression>(lit));

  ast::TypeConstructorExpression expr(&i32, values);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "i32(-12345)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Uint) {
  ast::type::U32 u32;

  auto* lit = create<ast::UintLiteral>(Source{}, &u32, 12345);
  ast::ExpressionList values;
  values.push_back(create<ast::ScalarConstructorExpression>(lit));

  ast::TypeConstructorExpression expr(&u32, values);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "u32(12345u)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Vec) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  auto* lit1 = create<ast::FloatLiteral>(Source{}, &f32, 1.f);
  auto* lit2 = create<ast::FloatLiteral>(Source{}, &f32, 2.f);
  auto* lit3 = create<ast::FloatLiteral>(Source{}, &f32, 3.f);
  ast::ExpressionList values;
  values.push_back(create<ast::ScalarConstructorExpression>(lit1));
  values.push_back(create<ast::ScalarConstructorExpression>(lit2));
  values.push_back(create<ast::ScalarConstructorExpression>(lit3));

  ast::TypeConstructorExpression expr(&vec, values);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), "vec3<f32>(1.0, 2.0, 3.0)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Mat) {
  ast::type::F32 f32;
  ast::type::Matrix mat(&f32, 3, 2);

  ast::type::Vector vec(&f32, 2);

  ast::ExpressionList mat_values;

  for (size_t i = 0; i < 3; i++) {
    auto* lit1 = create<ast::FloatLiteral>(Source{}, &f32,
                                           static_cast<float>(1 + (i * 2)));
    auto* lit2 = create<ast::FloatLiteral>(Source{}, &f32,
                                           static_cast<float>(2 + (i * 2)));

    ast::ExpressionList values;
    values.push_back(create<ast::ScalarConstructorExpression>(lit1));
    values.push_back(create<ast::ScalarConstructorExpression>(lit2));

    mat_values.push_back(create<ast::TypeConstructorExpression>(&vec, values));
  }

  ast::TypeConstructorExpression expr(&mat, mat_values);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(), std::string("mat2x3<f32>(vec2<f32>(1.0, 2.0), ") +
                              "vec2<f32>(3.0, 4.0), " + "vec2<f32>(5.0, 6.0))");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Array) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);
  ast::type::Array ary(&vec, 3, ast::ArrayDecorationList{});

  ast::ExpressionList ary_values;

  for (size_t i = 0; i < 3; i++) {
    auto* lit1 = create<ast::FloatLiteral>(Source{}, &f32,
                                           static_cast<float>(1 + (i * 3)));
    auto* lit2 = create<ast::FloatLiteral>(Source{}, &f32,
                                           static_cast<float>(2 + (i * 3)));
    auto* lit3 = create<ast::FloatLiteral>(Source{}, &f32,
                                           static_cast<float>(3 + (i * 3)));

    ast::ExpressionList values;
    values.push_back(create<ast::ScalarConstructorExpression>(lit1));
    values.push_back(create<ast::ScalarConstructorExpression>(lit2));
    values.push_back(create<ast::ScalarConstructorExpression>(lit3));

    ary_values.push_back(create<ast::TypeConstructorExpression>(&vec, values));
  }

  ast::TypeConstructorExpression expr(&ary, ary_values);

  ASSERT_TRUE(gen.EmitConstructor(&expr)) << gen.error();
  EXPECT_EQ(gen.result(),
            std::string("array<vec3<f32>, 3>(") + "vec3<f32>(1.0, 2.0, 3.0), " +
                "vec3<f32>(4.0, 5.0, 6.0), " + "vec3<f32>(7.0, 8.0, 9.0))");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
