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
  WrapInFunction(Expr(false));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("false"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Int) {
  WrapInFunction(Expr(-12345));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("-12345"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_UInt) {
  WrapInFunction(Expr(56779u));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("56779u"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Float) {
  // Use a number close to 1<<30 but whose decimal representation ends in 0.
  WrapInFunction(Expr(static_cast<float>((1 << 30) - 4)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("1073741824.0f"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Float) {
  WrapInFunction(Construct<f32>(-1.2e-5f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("float(-0.000012f)"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Bool) {
  WrapInFunction(Construct<bool>(true));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("bool(true)"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Int) {
  WrapInFunction(Construct<i32>(-12345));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("int(-12345)"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Uint) {
  WrapInFunction(Construct<u32>(12345u));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("uint(12345u)"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec) {
  WrapInFunction(vec3<f32>(1.f, 2.f, 3.f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("float3(1.0f, 2.0f, 3.0f)"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec_Empty) {
  WrapInFunction(vec3<f32>());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("float3(0.0f, 0.0f, 0.0f)"));

  Validate();
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

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Mat_Empty) {
  WrapInFunction(mat2x3<f32>());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();

  EXPECT_THAT(result(),
              HasSubstr("float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Array) {
  WrapInFunction(Construct(ty.array(ty.vec3<f32>(), 3),
                           vec3<f32>(1.f, 2.f, 3.f), vec3<f32>(4.f, 5.f, 6.f),
                           vec3<f32>(7.f, 8.f, 9.f)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr("{float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f),"
                        " float3(7.0f, 8.0f, 9.0f)}"));

  Validate();
}

// TODO(bclayton): Zero-init arrays
TEST_F(HlslGeneratorImplTest_Constructor,
       DISABLED_EmitConstructor_Type_Array_Empty) {
  WrapInFunction(Construct(ty.array(ty.vec3<f32>(), 3)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr("{float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f),"
                        " float3(0.0f, 0.0f, 0.0f)}"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Struct) {
  auto* str = Structure("S", {
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                                 Member("c", ty.vec3<i32>()),
                             });

  WrapInFunction(Construct(str, 1, 2.0f, vec3<i32>(3, 4, 5)));

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("{1, 2.0f, int3(3, 4, 5)}"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_Constructor, EmitConstructor_Type_Struct_Empty) {
  auto* str = Structure("S", {
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                                 Member("c", ty.vec3<i32>()),
                             });

  WrapInFunction(Construct(str));

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("{0, 0.0f, int3(0, 0, 0)}"));

  Validate();
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
