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

#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitConstructor_Bool) {
  auto* expr = Expr(false);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "false");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Int) {
  auto* expr = Expr(-12345);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "-12345");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_UInt) {
  auto* expr = Expr(56779u);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "56779u");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Float) {
  // Use a number close to 1<<30 but whose decimal representation ends in 0.
  auto* expr = Expr(static_cast<float>((1 << 30) - 4));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "1073741824.0");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Float) {
  auto* expr = Construct<f32>(Expr(-1.2e-5f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "f32(-0.000012)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Bool) {
  auto* expr = Construct<bool>(true);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "bool(true)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Int) {
  auto* expr = Construct<i32>(-12345);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "i32(-12345)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Uint) {
  auto* expr = Construct<u32>(12345u);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "u32(12345u)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Vec) {
  auto* expr = vec3<f32>(1.f, 2.f, 3.f);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(), "vec3<f32>(1.0, 2.0, 3.0)");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Mat) {
  auto* expr = Construct(ty.mat2x3<f32>(), vec2<f32>(1.0f, 2.0f),
                         vec2<f32>(3.0f, 4.0f), vec2<f32>(5.0f, 6.0f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(),
            "mat2x3<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0), "
            "vec2<f32>(5.0, 6.0))");
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Array) {
  auto* expr = array(ty.vec3<f32>(), 3, vec3<f32>(1.f, 2.f, 3.f),
                     vec3<f32>(4.f, 5.f, 6.f), vec3<f32>(7.f, 8.f, 9.f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(expr)) << gen.error();
  EXPECT_EQ(gen.result(),
            "array<vec3<f32>, 3>(vec3<f32>(1.0, 2.0, 3.0), "
            "vec3<f32>(4.0, 5.0, 6.0), vec3<f32>(7.0, 8.0, 9.0))");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
