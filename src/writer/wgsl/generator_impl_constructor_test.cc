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
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using ::testing::HasSubstr;

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitConstructor_Bool) {
  WrapInFunction(Expr(false));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("false"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Int) {
  WrapInFunction(Expr(-12345));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("-12345"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_UInt) {
  WrapInFunction(Expr(56779u));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("56779u"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Float) {
  // Use a number close to 1<<30 but whose decimal representation ends in 0.
  WrapInFunction(Expr(static_cast<float>((1 << 30) - 4)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("1073741824.0"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Float) {
  WrapInFunction(Construct<f32>(Expr(-1.2e-5f)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("f32(-0.000012)"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Bool) {
  WrapInFunction(Construct<bool>(true));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("bool(true)"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Int) {
  WrapInFunction(Construct<i32>(-12345));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("i32(-12345)"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Uint) {
  WrapInFunction(Construct<u32>(12345u));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("u32(12345u)"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Vec) {
  WrapInFunction(vec3<f32>(1.f, 2.f, 3.f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("vec3<f32>(1.0, 2.0, 3.0)"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Mat) {
  WrapInFunction(
      mat2x3<f32>(vec3<f32>(1.f, 2.f, 3.f), vec3<f32>(3.f, 4.f, 5.f)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("mat2x3<f32>(vec3<f32>(1.0, 2.0, 3.0), "
                                      "vec3<f32>(3.0, 4.0, 5.0))"));
}

TEST_F(WgslGeneratorImplTest, EmitConstructor_Type_Array) {
  WrapInFunction(
      Construct(ty.array(ty.vec3<f32>(), 3), vec3<f32>(1.0f, 2.0f, 3.0f),
                vec3<f32>(4.0f, 5.0f, 6.0f), vec3<f32>(7.0f, 8.0f, 9.0f)));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(),
              HasSubstr("array<vec3<f32>, 3>(vec3<f32>(1.0, 2.0, 3.0), "
                        "vec3<f32>(4.0, 5.0, 6.0), vec3<f32>(7.0, 8.0, 9.0))"));
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
