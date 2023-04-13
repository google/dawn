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

#include "src/tint/writer/wgsl/test_helper.h"

#include "gmock/gmock.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest_Constructor = TestHelper;

TEST_F(WgslGeneratorImplTest_Constructor, Bool) {
    WrapInFunction(Expr(false));

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("false"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Int) {
    WrapInFunction(Expr(-12345_i));

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("-12345"));
}

TEST_F(WgslGeneratorImplTest_Constructor, UInt) {
    WrapInFunction(Expr(56779_u));

    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("56779u"));
}

TEST_F(WgslGeneratorImplTest_Constructor, F32) {
    // Use a number close to 1<<30 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f32((1 << 30) - 4)));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("1073741824.0f"));
}

TEST_F(WgslGeneratorImplTest_Constructor, F16) {
    Enable(builtin::Extension::kF16);

    // Use a number close to 1<<16 but whose decimal representation ends in 0.
    WrapInFunction(Expr(f16((1 << 15) - 8)));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("32752.0h"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_F32) {
    WrapInFunction(Call<f32>(Expr(-1.2e-5_f)));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("f32(-0.00001200000042445026f)"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_F16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Call<f16>(Expr(-1.2e-5_h)));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("f16(-0.00001198053359985352h)"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_Bool) {
    WrapInFunction(Call<bool>(true));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("bool(true)"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_Int) {
    WrapInFunction(Call<i32>(-12345_i));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("i32(-12345i)"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_Uint) {
    WrapInFunction(Call<u32>(12345_u));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("u32(12345u)"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_Vec_F32) {
    WrapInFunction(vec3<f32>(1_f, 2_f, 3_f));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("vec3<f32>(1.0f, 2.0f, 3.0f)"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_Vec_F16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(vec3<f16>(1_h, 2_h, 3_h));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("vec3<f16>(1.0h, 2.0h, 3.0h)"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_Mat_F32) {
    WrapInFunction(mat2x3<f32>(vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(3_f, 4_f, 5_f)));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("mat2x3<f32>(vec3<f32>(1.0f, 2.0f, 3.0f), "
                                        "vec3<f32>(3.0f, 4.0f, 5.0f))"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_Mat_F16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(mat2x3<f16>(vec3<f16>(1_h, 2_h, 3_h), vec3<f16>(3_h, 4_h, 5_h)));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("mat2x3<f16>(vec3<f16>(1.0h, 2.0h, 3.0h), "
                                        "vec3<f16>(3.0h, 4.0h, 5.0h))"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_Array) {
    WrapInFunction(Call(ty.array(ty.vec3<f32>(), 3_u), vec3<f32>(1_f, 2_f, 3_f),
                        vec3<f32>(4_f, 5_f, 6_f), vec3<f32>(7_f, 8_f, 9_f)));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(),
                HasSubstr("array<vec3<f32>, 3u>(vec3<f32>(1.0f, 2.0f, 3.0f), "
                          "vec3<f32>(4.0f, 5.0f, 6.0f), vec3<f32>(7.0f, 8.0f, 9.0f))"));
}

TEST_F(WgslGeneratorImplTest_Constructor, Type_ImplicitArray) {
    WrapInFunction(Call(ty.array<Infer>(), vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f),
                        vec3<f32>(7_f, 8_f, 9_f)));

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(),
                HasSubstr("array(vec3<f32>(1.0f, 2.0f, 3.0f), "
                          "vec3<f32>(4.0f, 5.0f, 6.0f), vec3<f32>(7.0f, 8.0f, 9.0f))"));
}

}  // namespace
}  // namespace tint::writer::wgsl
