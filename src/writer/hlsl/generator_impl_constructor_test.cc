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

#include "gmock/gmock.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using ::testing::HasSubstr;

using HlslGeneratorImplTest_Constructor = TestHelper;

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Bool) {
  auto* expr = Expr(false);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "false");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Int) {
  auto* expr = Expr(-12345);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "-12345");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_UInt) {
  auto* expr = Expr(56779u);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "56779u");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Float) {
  // Use a number close to 1<<30 but whose decimal representation ends in 0.
  auto* expr = Expr(static_cast<float>((1 << 30) - 4));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "1073741824.0f");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Float) {
  auto* expr = Construct<f32>(-1.2e-5f);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "float(-0.000012f)");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Bool) {
  auto* expr = Construct<bool>(true);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "bool(true)");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Int) {
  auto* expr = Construct<i32>(-12345);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "int(-12345)");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Uint) {
  auto* expr = Construct<u32>(12345u);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "uint(12345u)");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec) {
  auto* expr = vec3<f32>(1.f, 2.f, 3.f);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "float3(1.0f, 2.0f, 3.0f)");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec_Empty) {
  auto* expr = vec3<f32>();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(), "float3(0.0f)");
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Mat) {
  WrapInFunction(
      mat2x3<f32>(vec3<f32>(1.f, 2.f, 3.f), vec3<f32>(3.f, 4.f, 5.f)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();

  EXPECT_THAT(
      result(),
      HasSubstr(
          "float2x3(float3(1.0f, 2.0f, 3.0f), float3(3.0f, 4.0f, 5.0f))"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Array) {
  auto* expr = Construct(ty.array(ty.vec3<f32>(), 3), vec3<f32>(1.f, 2.f, 3.f),
                         vec3<f32>(4.f, 5.f, 6.f), vec3<f32>(7.f, 8.f, 9.f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitConstructor(pre, out, expr)) << gen.error();
  EXPECT_EQ(result(),
            "{float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f),"
            " float3(7.0f, 8.0f, 9.0f)}");
}

// TODO(dsinclair): Add struct constructor test.
TEST_F(HlslGeneratorImplTest_Constructor,
       DISABLED_EmitConstructor_Type_Struct) {}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
