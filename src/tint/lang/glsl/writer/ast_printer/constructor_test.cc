// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gmock/gmock.h"
#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"

namespace tint::glsl::writer {
namespace {

using ::testing::HasSubstr;
using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using GlslASTPrinterTest_Constructor = TestHelper;

TEST_F(GlslASTPrinterTest_Constructor, Bool) {
    WrapInFunction(Expr(false));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("false"));
}

TEST_F(GlslASTPrinterTest_Constructor, Int) {
    WrapInFunction(Expr(-12345_i));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("-12345"));
}

TEST_F(GlslASTPrinterTest_Constructor, UInt) {
    WrapInFunction(Expr(56779_u));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("56779u"));
}

TEST_F(GlslASTPrinterTest_Constructor, Float) {
    // Use a number close to 1<<30 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f32((1 << 30) - 4)));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("1073741824.0f"));
}

TEST_F(GlslASTPrinterTest_Constructor, F16) {
    Enable(wgsl::Extension::kF16);

    // Use a number close to 1<<16 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f16((1 << 15) - 8)));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("32752.0hf"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Float) {
    WrapInFunction(Call<f32>(-1.2e-5_f));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("-0.00001200000042445026f"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_F16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<f16>(-1.2e-3_h));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("-0.0011997222900390625hf"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Bool) {
    WrapInFunction(Call<bool>(true));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("true"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Int) {
    WrapInFunction(Call<i32>(-12345_i));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("-12345"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Uint) {
    WrapInFunction(Call<u32>(12345_u));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("12345u"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_F32) {
    WrapInFunction(Call<vec3<f32>>(1_f, 2_f, 3_f));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("vec3(1.0f, 2.0f, 3.0f)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_F16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<vec3<f16>>(1_h, 2_h, 3_h));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("f16vec3(1.0hf, 2.0hf, 3.0hf)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_Empty_F32) {
    WrapInFunction(Call<vec3<f32>>());

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("vec3(0.0f)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_Empty_F16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<vec3<f16>>());

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("f16vec3(0.0hf)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_SingleScalar_F32_Literal) {
    WrapInFunction(Call<vec3<f32>>(2_f));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("vec3(2.0f)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_SingleScalar_F16_Literal) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<vec3<f16>>(2_h));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("f16vec3(2.0hf)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_SingleScalar_F32_Var) {
    auto* var = Var("v", Expr(2_f));
    auto* cast = Call<vec3<f32>>(var);
    WrapInFunction(var, cast);

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr(R"(float v = 2.0f;
  vec3 tint_symbol = vec3(v);)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_SingleScalar_F16_Var) {
    Enable(wgsl::Extension::kF16);

    auto* var = Var("v", Expr(2_h));
    auto* cast = Call<vec3<f16>>(var);
    WrapInFunction(var, cast);

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr(R"(float16_t v = 2.0hf;
  f16vec3 tint_symbol = f16vec3(v);)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_SingleScalar_Bool) {
    WrapInFunction(Call<vec3<bool>>(true));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("bvec3(true)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_SingleScalar_Int) {
    WrapInFunction(Call<vec3<i32>>(2_i));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("ivec3(2)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Vec_SingleScalar_UInt) {
    WrapInFunction(Call<vec3<u32>>(2_u));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("uvec3(2u)"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Mat_F32) {
    WrapInFunction(
        Call<mat2x3<f32>>(Call<vec3<f32>>(1_f, 2_f, 3_f), Call<vec3<f32>>(3_f, 4_f, 5_f)));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("mat2x3(vec3(1.0f, 2.0f, 3.0f), vec3(3.0f, 4.0f, 5.0f))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Mat_F16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(
        Call<mat2x3<f16>>(Call<vec3<f16>>(1_h, 2_h, 3_h), Call<vec3<f16>>(3_h, 4_h, 5_h)));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(),
                HasSubstr("f16mat2x3(f16vec3(1.0hf, 2.0hf, 3.0hf), f16vec3(3.0hf, 4.0hf, 5.0hf))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Mat_Complex_F32) {
    // mat4x4<f32>(
    //     vec4<f32>(2.0f, 3.0f, 4.0f, 8.0f),
    //     vec4<f32>(),
    //     vec4<f32>(7.0f),
    //     vec4<f32>(vec4<f32>(42.0f, 21.0f, 6.0f, -5.0f)),
    //   );
    auto* vector_literal =
        Call<vec4<f32>>(Expr(f32(2.0)), Expr(f32(3.0)), Expr(f32(4.0)), Expr(f32(8.0)));
    auto* vector_zero_init = Call<vec4<f32>>();
    auto* vector_single_scalar_init = Call<vec4<f32>>(Expr(f32(7.0)));
    auto* vector_identical_init = Call<vec4<f32>>(
        Call<vec4<f32>>(Expr(f32(42.0)), Expr(f32(21.0)), Expr(f32(6.0)), Expr(f32(-5.0))));

    auto* ctor = Call<mat4x4<f32>>(vector_literal, vector_zero_init, vector_single_scalar_init,
                                   vector_identical_init);

    WrapInFunction(ctor);

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("mat4(vec4(2.0f, 3.0f, 4.0f, 8.0f), vec4(0.0f), "
                                        "vec4(7.0f), vec4(42.0f, 21.0f, 6.0f, -5.0f))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Mat_Complex_F16) {
    // mat4x4<f16>(
    //     vec4<f16>(2.0h, 3.0h, 4.0h, 8.0h),
    //     vec4<f16>(),
    //     vec4<f16>(7.0h),
    //     vec4<f16>(vec4<f16>(42.0h, 21.0h, 6.0h, -5.0h)),
    //   );
    Enable(wgsl::Extension::kF16);

    auto* vector_literal =
        Call<vec4<f16>>(Expr(f16(2.0)), Expr(f16(3.0)), Expr(f16(4.0)), Expr(f16(8.0)));
    auto* vector_zero_init = Call<vec4<f16>>();
    auto* vector_single_scalar_init = Call<vec4<f16>>(Expr(f16(7.0)));
    auto* vector_identical_init = Call<vec4<f16>>(
        Call<vec4<f16>>(Expr(f16(42.0)), Expr(f16(21.0)), Expr(f16(6.0)), Expr(f16(-5.0))));

    auto* ctor = Call<mat4x4<f16>>(vector_literal, vector_zero_init, vector_single_scalar_init,
                                   vector_identical_init);

    WrapInFunction(ctor);

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(),
                HasSubstr("f16mat4(f16vec4(2.0hf, 3.0hf, 4.0hf, 8.0hf), f16vec4(0.0hf), "
                          "f16vec4(7.0hf), f16vec4(42.0hf, 21.0hf, 6.0hf, -5.0hf))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Mat_Empty_F32) {
    WrapInFunction(Call<mat2x3<f32>>());

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("mat2x3 tint_symbol = mat2x3(vec3(0.0f), vec3(0.0f))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Mat_Empty_F16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Call<mat2x3<f16>>());

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(),
                HasSubstr("f16mat2x3 tint_symbol = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Mat_Identity_F32) {
    // fn f() {
    //     var m_1: mat4x4<f32> = mat4x4<f32>();
    //     var m_2: mat4x4<f32> = mat4x4<f32>(m_1);
    // }

    auto* m_1 = Var("m_1", ty.mat4x4(ty.f32()), Call<mat4x4<f32>>());
    auto* m_2 = Var("m_2", ty.mat4x4(ty.f32()), Call<mat4x4<f32>>(m_1));

    WrapInFunction(m_1, m_2);

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("mat4 m_2 = mat4(m_1);"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Mat_Identity_F16) {
    // fn f() {
    //     var m_1: mat4x4<f16> = mat4x4<f16>();
    //     var m_2: mat4x4<f16> = mat4x4<f16>(m_1);
    // }

    Enable(wgsl::Extension::kF16);

    auto* m_1 = Var("m_1", ty.mat4x4(ty.f16()), Call<mat4x4<f16>>());
    auto* m_2 = Var("m_2", ty.mat4x4(ty.f16()), Call<mat4x4<f16>>(m_1));

    WrapInFunction(m_1, m_2);

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("f16mat4 m_2 = f16mat4(m_1);"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Array) {
    WrapInFunction(Call<array<vec3<f32>, 3>>(Call<vec3<f32>>(1_f, 2_f, 3_f),
                                             Call<vec3<f32>>(4_f, 5_f, 6_f),
                                             Call<vec3<f32>>(7_f, 8_f, 9_f)));

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("vec3[3](vec3(1.0f, 2.0f, 3.0f), "
                                        "vec3(4.0f, 5.0f, 6.0f), "
                                        "vec3(7.0f, 8.0f, 9.0f))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Array_Empty) {
    WrapInFunction(Call<array<vec3<f32>, 3>>());

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("vec3[3](vec3(0.0f), vec3(0.0f), vec3(0.0f))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Struct) {
    auto* str = Structure("S", Vector{
                                   Member("a", ty.i32()),
                                   Member("b", ty.f32()),
                                   Member("c", ty.vec3<i32>()),
                               });

    WrapInFunction(Call(ty.Of(str), 1_i, 2_f, Call<vec3<i32>>(3_i, 4_i, 5_i)));

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("S(1, 2.0f, ivec3(3, 4, 5))"));
}

TEST_F(GlslASTPrinterTest_Constructor, Type_Struct_Empty) {
    auto* str = Structure("S", Vector{
                                   Member("a", ty.i32()),
                                   Member("b", ty.f32()),
                                   Member("c", ty.vec3<i32>()),
                               });

    WrapInFunction(Call(ty.Of(str)));

    ASTPrinter& gen = SanitizeAndBuild();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.Result(), HasSubstr("S(0"));
}

}  // namespace
}  // namespace tint::glsl::writer
