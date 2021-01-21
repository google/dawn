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
#include "src/ast/uint_literal.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitConstructor_Bool) {
  ASSERT_TRUE(gen.EmitConstructor(Expr(false))) << gen.error();
  EXPECT_EQ(gen.result(), "false");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Int) {
  ASSERT_TRUE(gen.EmitConstructor(Expr(-12345))) << gen.error();
  EXPECT_EQ(gen.result(), "-12345");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_UInt) {
  ASSERT_TRUE(gen.EmitConstructor(Expr(56779u))) << gen.error();
  EXPECT_EQ(gen.result(), "56779u");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Float) {
  // Use a number close to 1<<30 but whose decimal representation ends in 0.
  ASSERT_TRUE(gen.EmitConstructor(Expr(static_cast<float>((1 << 30) - 4))))
      << gen.error();
  EXPECT_EQ(gen.result(), "1073741824.0");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Float) {
  ASSERT_TRUE(gen.EmitConstructor(Construct<f32>(Expr(-1.2e-5f))))
      << gen.error();
  EXPECT_EQ(gen.result(), "f32(-0.000012)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Bool) {
  ASSERT_TRUE(gen.EmitConstructor(Construct<bool>(true))) << gen.error();
  EXPECT_EQ(gen.result(), "bool(true)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Int) {
  ASSERT_TRUE(gen.EmitConstructor(Construct<i32>(-12345))) << gen.error();
  EXPECT_EQ(gen.result(), "i32(-12345)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Uint) {
  ASSERT_TRUE(gen.EmitConstructor(Construct<u32>(12345u))) << gen.error();
  EXPECT_EQ(gen.result(), "u32(12345u)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Vec) {
  ASSERT_TRUE(gen.EmitConstructor(vec3<f32>(1.f, 2.f, 3.f))) << gen.error();
  EXPECT_EQ(gen.result(), "vec3<f32>(1.0, 2.0, 3.0)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Mat) {
  ASSERT_TRUE(gen.EmitConstructor(
      Construct(ty.mat2x3<f32>(), vec2<f32>(1.0f, 2.0f), vec2<f32>(3.0f, 4.0f),
                vec2<f32>(5.0f, 6.0f))))
      << gen.error();
  EXPECT_EQ(gen.result(),
            "mat2x3<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0), "
            "vec2<f32>(5.0, 6.0))");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Array) {
  ASSERT_TRUE(gen.EmitConstructor(
      array(ty.vec3<f32>(), 3, vec3<f32>(1.f, 2.f, 3.f),
            vec3<f32>(4.f, 5.f, 6.f), vec3<f32>(7.f, 8.f, 9.f))))
      << gen.error();
  EXPECT_EQ(gen.result(),
            "array<vec3<f32>, 3>(vec3<f32>(1.0, 2.0, 3.0), "
            "vec3<f32>(4.0, 5.0, 6.0), vec3<f32>(7.0, 8.0, 9.0))");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
