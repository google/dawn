// Copyright 2021 The Tint Authors.
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
#include "src/tint/writer/glsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using ::testing::HasSubstr;

using GlslGeneratorImplTest_Constructor = TestHelper;

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Bool) {
    WrapInFunction(Expr(false));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("false"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Int) {
    WrapInFunction(Expr(-12345_i));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("-12345"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_UInt) {
    WrapInFunction(Expr(56779_u));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("56779u"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Float) {
    // Use a number close to 1<<30 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f32((1 << 30) - 4)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("1073741824.0f"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Float) {
    WrapInFunction(Construct<f32>(-1.2e-5f));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("float(-0.000012f)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Bool) {
    WrapInFunction(Construct<bool>(true));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("bool(true)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Int) {
    WrapInFunction(Construct<i32>(-12345_i));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("int(-12345)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Uint) {
    WrapInFunction(Construct<u32>(12345_u));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("uint(12345u)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec) {
    WrapInFunction(vec3<f32>(1.f, 2.f, 3.f));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("vec3(1.0f, 2.0f, 3.0f)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec_Empty) {
    WrapInFunction(vec3<f32>());

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("vec3(0.0f, 0.0f, 0.0f)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec_SingleScalar_Float) {
    WrapInFunction(vec3<f32>(2.0f));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("vec3(2.0f)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec_SingleScalar_Bool) {
    WrapInFunction(vec3<bool>(true));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("bvec3(true)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec_SingleScalar_Int) {
    WrapInFunction(vec3<i32>(2_i));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("ivec3(2)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Vec_SingleScalar_UInt) {
    WrapInFunction(vec3<u32>(2_u));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("uvec3(2u)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Mat) {
    WrapInFunction(mat2x3<f32>(vec3<f32>(1.f, 2.f, 3.f), vec3<f32>(3.f, 4.f, 5.f)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_THAT(gen.result(), HasSubstr("mat2x3(vec3(1.0f, 2.0f, 3.0f), vec3(3.0f, 4.0f, 5.0f))"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Mat_Empty) {
    WrapInFunction(mat2x3<f32>());

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_THAT(gen.result(), HasSubstr("mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Array) {
    WrapInFunction(Construct(ty.array(ty.vec3<f32>(), 3_u), vec3<f32>(1.f, 2.f, 3.f),
                             vec3<f32>(4.f, 5.f, 6.f), vec3<f32>(7.f, 8.f, 9.f)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("vec3[3](vec3(1.0f, 2.0f, 3.0f), "
                                        "vec3(4.0f, 5.0f, 6.0f), "
                                        "vec3(7.0f, 8.0f, 9.0f))"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Array_Empty) {
    WrapInFunction(Construct(ty.array(ty.vec3<f32>(), 3_u)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("vec3[3](vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),"
                                        " vec3(0.0f, 0.0f, 0.0f))"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Struct) {
    auto* str = Structure("S", {
                                   Member("a", ty.i32()),
                                   Member("b", ty.f32()),
                                   Member("c", ty.vec3<i32>()),
                               });

    WrapInFunction(Construct(ty.Of(str), 1_i, 2.0f, vec3<i32>(3_i, 4_i, 5_i)));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("S(1, 2.0f, ivec3(3, 4, 5))"));
}

TEST_F(GlslGeneratorImplTest_Constructor, EmitConstructor_Type_Struct_Empty) {
    auto* str = Structure("S", {
                                   Member("a", ty.i32()),
                                   Member("b", ty.f32()),
                                   Member("c", ty.vec3<i32>()),
                               });

    WrapInFunction(Construct(ty.Of(str)));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("S(0"));
}

}  // namespace
}  // namespace tint::writer::glsl
