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
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using ::testing::HasSubstr;

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitConstructor_Bool) {
  WrapInFunction(Expr(false));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("false"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Int) {
  WrapInFunction(Expr(-12345));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("-12345"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_UInt) {
  WrapInFunction(Expr(56779u));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("56779u"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Float) {
  // Use a number close to 1<<30 but whose decimal representation ends in 0.
  WrapInFunction(Expr(static_cast<float>((1 << 30) - 4)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("1073741824.0f"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Float) {
  WrapInFunction(Construct<f32>(-1.2e-5f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("float(-0.000012f)"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Bool) {
  WrapInFunction(Construct<bool>(true));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("bool(true)"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Int) {
  WrapInFunction(Construct<i32>(-12345));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("int(-12345)"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Uint) {
  WrapInFunction(Construct<u32>(12345u));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("uint(12345u)"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Vec) {
  WrapInFunction(vec3<f32>(1.f, 2.f, 3.f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("float3(1.0f, 2.0f, 3.0f)"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Vec_Empty) {
  WrapInFunction(vec3<f32>());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("float3()"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Mat) {
  WrapInFunction(Construct(ty.mat2x3<f32>(), vec3<f32>(1.0f, 2.0f, 3.0f),
                           vec3<f32>(3.0f, 4.0f, 5.0f)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();

  // A matrix of type T with n columns and m rows can also be constructed from
  // n vectors of type T with m components.
  EXPECT_THAT(
      gen.result(),
      HasSubstr(
          "float2x3(float3(1.0f, 2.0f, 3.0f), float3(3.0f, 4.0f, 5.0f))"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Mat_Empty) {
  WrapInFunction(mat4x4<f32>());

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("float4x4()"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Array) {
  WrapInFunction(
      Construct(ty.array(ty.vec3<f32>(), 3), vec3<f32>(1.0f, 2.0f, 3.0f),
                vec3<f32>(4.0f, 5.0f, 6.0f), vec3<f32>(7.0f, 8.0f, 9.0f)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(),
              HasSubstr("{float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), "
                        "float3(7.0f, 8.0f, 9.0f)}"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Struct) {
  auto* str = Structure("S", {
                                 Member("a", ty.i32()),
                                 Member("b", ty.f32()),
                                 Member("c", ty.vec3<i32>()),
                             });

  WrapInFunction(Construct(str, 1, 2.0f, vec3<i32>(3, 4, 5)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("{1, 2.0f, int3(3, 4, 5)}"));
}

TEST_F(MslGeneratorImplTest, EmitConstructor_Type_Struct_Empty) {
  auto* str = Structure("S", {});

  WrapInFunction(Construct(str));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("{}"));
  EXPECT_THAT(gen.result(), Not(HasSubstr("{{}}")));
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
