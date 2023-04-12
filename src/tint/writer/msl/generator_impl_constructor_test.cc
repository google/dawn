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
#include "src/tint/writer/msl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::msl {
namespace {

using ::testing::HasSubstr;

using MslGeneratorImplTest_Constructor = TestHelper;

TEST_F(MslGeneratorImplTest_Constructor, Bool) {
    WrapInFunction(Expr(false));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("false"));
}

TEST_F(MslGeneratorImplTest_Constructor, Int) {
    WrapInFunction(Expr(-12345_i));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("-12345"));
}

TEST_F(MslGeneratorImplTest_Constructor, UInt) {
    WrapInFunction(Expr(56779_u));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("56779u"));
}

TEST_F(MslGeneratorImplTest_Constructor, Float) {
    // Use a number close to 1<<30 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f32((1 << 30) - 4)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("1073741824.0f"));
}

TEST_F(MslGeneratorImplTest_Constructor, F16) {
    Enable(builtin::Extension::kF16);

    // Use a number close to 1<<16 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f16((1 << 15) - 8)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("32752.0h"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Float) {
    WrapInFunction(Call<f32>(-1.2e-5_f));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("-0.00001200000042445026f"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_F16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Call<f16>(-1.2e-3_h));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("-0.0011997222900390625h"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Bool) {
    WrapInFunction(Call<bool>(true));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("true"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Int) {
    WrapInFunction(Call<i32>(-12345_i));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("-12345"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Uint) {
    WrapInFunction(Call<u32>(12345_u));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("12345u"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_F32) {
    WrapInFunction(vec3<f32>(1_f, 2_f, 3_f));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("float3(1.0f, 2.0f, 3.0f)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_F16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(vec3<f16>(1_h, 2_h, 3_h));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("half3(1.0h, 2.0h, 3.0h)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_Empty_F32) {
    WrapInFunction(vec3<f32>());

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("float3(0.0f)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_Empty_F16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(vec3<f16>());

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("half3(0.0h)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_SingleScalar_F32_Literal) {
    WrapInFunction(vec3<f32>(2_f));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("float3(2.0f)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_SingleScalar_F16_Literal) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(vec3<f16>(2_h));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("half3(2.0h)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_SingleScalar_F32_Var) {
    auto* var = Var("v", Expr(2_f));
    auto* cast = vec3<f32>(var);
    WrapInFunction(var, cast);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr(R"(float v = 2.0f;
  float3 const tint_symbol = float3(v);)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_SingleScalar_F16_Var) {
    Enable(builtin::Extension::kF16);

    auto* var = Var("v", Expr(2_h));
    auto* cast = vec3<f16>(var);
    WrapInFunction(var, cast);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr(R"(half v = 2.0h;
  half3 const tint_symbol = half3(v);)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_SingleScalar_Bool) {
    WrapInFunction(vec3<bool>(true));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("bool3(true)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_SingleScalar_Int) {
    WrapInFunction(vec3<i32>(2_i));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("int3(2)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Vec_SingleScalar_UInt) {
    WrapInFunction(vec3<u32>(2_u));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("uint3(2u)"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Mat_F32) {
    WrapInFunction(mat2x3<f32>(vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(3_f, 4_f, 5_f)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_THAT(gen.result(),
                HasSubstr("float2x3(float3(1.0f, 2.0f, 3.0f), float3(3.0f, 4.0f, 5.0f))"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Mat_F16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(mat2x3<f16>(vec3<f16>(1_h, 2_h, 3_h), vec3<f16>(3_h, 4_h, 5_h)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_THAT(gen.result(),
                HasSubstr("half2x3(half3(1.0h, 2.0h, 3.0h), half3(3.0h, 4.0h, 5.0h))"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Mat_Complex_F32) {
    // mat4x4<f32>(
    //     vec4<f32>(2.0f, 3.0f, 4.0f, 8.0f),
    //     vec4<f32>(),
    //     vec4<f32>(7.0f),
    //     vec4<f32>(vec4<f32>(42.0f, 21.0f, 6.0f, -5.0f)),
    //   );
    auto* vector_literal =
        vec4<f32>(Expr(f32(2.0)), Expr(f32(3.0)), Expr(f32(4.0)), Expr(f32(8.0)));
    auto* vector_zero_init = vec4<f32>();
    auto* vector_single_scalar_init = vec4<f32>(Expr(f32(7.0)));
    auto* vector_identical_init =
        vec4<f32>(vec4<f32>(Expr(f32(42.0)), Expr(f32(21.0)), Expr(f32(6.0)), Expr(f32(-5.0))));

    auto* constructor = mat4x4<f32>(vector_literal, vector_zero_init, vector_single_scalar_init,
                                    vector_identical_init);

    WrapInFunction(constructor);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_THAT(gen.result(), HasSubstr("float4x4(float4(2.0f, 3.0f, 4.0f, 8.0f), float4(0.0f), "
                                        "float4(7.0f), float4(42.0f, 21.0f, 6.0f, -5.0f))"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Mat_Complex_F16) {
    // mat4x4<f16>(
    //     vec4<f16>(2.0h, 3.0h, 4.0h, 8.0h),
    //     vec4<f16>(),
    //     vec4<f16>(7.0h),
    //     vec4<f16>(vec4<f16>(42.0h, 21.0h, 6.0h, -5.0h)),
    //   );
    Enable(builtin::Extension::kF16);

    auto* vector_literal =
        vec4<f16>(Expr(f16(2.0)), Expr(f16(3.0)), Expr(f16(4.0)), Expr(f16(8.0)));
    auto* vector_zero_init = vec4<f16>();
    auto* vector_single_scalar_init = vec4<f16>(Expr(f16(7.0)));
    auto* vector_identical_init =
        vec4<f16>(vec4<f16>(Expr(f16(42.0)), Expr(f16(21.0)), Expr(f16(6.0)), Expr(f16(-5.0))));

    auto* constructor = mat4x4<f16>(vector_literal, vector_zero_init, vector_single_scalar_init,
                                    vector_identical_init);

    WrapInFunction(constructor);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_THAT(gen.result(), HasSubstr("half4x4(half4(2.0h, 3.0h, 4.0h, 8.0h), half4(0.0h), "
                                        "half4(7.0h), half4(42.0h, 21.0h, 6.0h, -5.0h))"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Mat_Empty_F32) {
    WrapInFunction(mat2x3<f32>());

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_THAT(gen.result(),
                HasSubstr("float2x3 const tint_symbol = float2x3(float3(0.0f), float3(0.0f))"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Mat_Empty_F16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(mat2x3<f16>());

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_THAT(gen.result(),
                HasSubstr("half2x3 const tint_symbol = half2x3(half3(0.0h), half3(0.0h))"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Mat_Identity_F32) {
    // fn f() {
    //     var m_1: mat4x4<f32> = mat4x4<f32>();
    //     var m_2: mat4x4<f32> = mat4x4<f32>(m_1);
    // }

    auto* m_1 = Var("m_1", ty.mat4x4(ty.f32()), mat4x4<f32>());
    auto* m_2 = Var("m_2", ty.mat4x4(ty.f32()), mat4x4<f32>(m_1));

    WrapInFunction(m_1, m_2);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_THAT(gen.result(), HasSubstr("float4x4 m_2 = float4x4(m_1);"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Mat_Identity_F16) {
    // fn f() {
    //     var m_1: mat4x4<f16> = mat4x4<f16>();
    //     var m_2: mat4x4<f16> = mat4x4<f16>(m_1);
    // }

    Enable(builtin::Extension::kF16);

    auto* m_1 = Var("m_1", ty.mat4x4(ty.f16()), mat4x4<f16>());
    auto* m_2 = Var("m_2", ty.mat4x4(ty.f16()), mat4x4<f16>(m_1));

    WrapInFunction(m_1, m_2);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    EXPECT_THAT(gen.result(), HasSubstr("half4x4 m_2 = half4x4(m_1);"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Array) {
    WrapInFunction(Call(ty.array(ty.vec3<f32>(), 3_u), vec3<f32>(1_f, 2_f, 3_f),
                        vec3<f32>(4_f, 5_f, 6_f), vec3<f32>(7_f, 8_f, 9_f)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("{float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), "
                                        "float3(7.0f, 8.0f, 9.0f)}"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Struct) {
    auto* str = Structure("S", utils::Vector{
                                   Member("a", ty.i32()),
                                   Member("b", ty.f32()),
                                   Member("c", ty.vec3<i32>()),
                               });

    WrapInFunction(Call(ty.Of(str), 1_i, 2_f, vec3<i32>(3_i, 4_i, 5_i)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("{.a=1, .b=2.0f, .c=int3(3, 4, 5)}"));
}

TEST_F(MslGeneratorImplTest_Constructor, Type_Struct_Empty) {
    auto* str = Structure("S", utils::Vector{
                                   Member("a", ty.i32()),
                                   Member("b", ty.f32()),
                                   Member("c", ty.vec3<i32>()),
                               });

    WrapInFunction(Call(ty.Of(str)));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_THAT(gen.result(), HasSubstr("{}"));
    EXPECT_THAT(gen.result(), testing::Not(HasSubstr("{{}}")));
}

}  // namespace
}  // namespace tint::writer::msl
