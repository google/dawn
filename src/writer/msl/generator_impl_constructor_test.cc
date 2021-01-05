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
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitConstructor_Bool) {
  auto* expr = Expr(false);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "false");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Int) {
  auto* expr = Expr(-12345);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "-12345");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_UInt) {
  auto* expr = Expr(56779u);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "56779u");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Float) {
  // Use a number close to 1<<30 but whose decimal representation ends in 0.
  auto* expr = Expr(static_cast<float>((1 << 30) - 4));
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "1073741824.0f");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Float) {
  auto* expr = Construct<f32>(-1.2e-5f);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "float(-0.000012f)");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Bool) {
  auto* expr = Construct<bool>(true);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "bool(true)");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Int) {
  auto* expr = Construct<i32>(-12345);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "int(-12345)");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Uint) {
  auto* expr = Construct<u32>(12345u);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "uint(12345u)");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Vec) {
  auto* expr = vec3<f32>(1.f, 2.f, 3.f);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "float3(1.0f, 2.0f, 3.0f)");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Vec_Empty) {
  auto* expr = vec3<f32>();
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "float3(0.0f)");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Mat) {
  ast::ExpressionList mat_values;

  for (size_t i = 0; i < 2; i++) {
    mat_values.push_back(vec3<f32>(static_cast<float>(1 + (i * 2)),
                                   static_cast<float>(2 + (i * 2)),
                                   static_cast<float>(3 + (i * 2))));
  }

  auto* expr = Construct(ty.mat2x3<f32>(), mat_values);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();

  // A matrix of type T with n columns and m rows can also be constructed from
  // n vectors of type T with m components.
  EXPECT_EQ(gen.result(),
            "float2x3(float3(1.0f, 2.0f, 3.0f), float3(3.0f, 4.0f, 5.0f))");
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Array) {
  ast::type::Array ary(ty.vec3<f32>(), 3, ast::ArrayDecorationList{});

  ast::ExpressionList ary_values;

  for (size_t i = 0; i < 3; i++) {
    ary_values.push_back(vec3<f32>(static_cast<float>(1 + (i * 3)),
                                   static_cast<float>(2 + (i * 3)),
                                   static_cast<float>(3 + (i * 3))));
  }

  auto* expr = Construct(&ary, ary_values);
  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(),
            "{float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), "
            "float3(7.0f, 8.0f, 9.0f)}");
}

// TODO(dsinclair): Add struct constructor test.
TEST_F(MslGeneratorImplTest, DISABLED_EmitConstructor_Type_Struct) {}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
