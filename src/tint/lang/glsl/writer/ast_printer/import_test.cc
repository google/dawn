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

#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::glsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using GlslASTPrinterTest_Import = TestHelper;

struct GlslImportData {
    const char* name;
    const char* glsl_name;
};
inline std::ostream& operator<<(std::ostream& out, GlslImportData data) {
    out << data.name;
    return out;
}

using GlslImportData_SingleParamTest = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_SingleParamTest, FloatScalar) {
    auto param = GetParam();

    auto* expr = Call(param.name, 1_f);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.glsl_name) + "(1.0f)");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_SingleParamTest,
                         testing::Values(GlslImportData{"abs", "abs"},
                                         GlslImportData{"acos", "acos"},
                                         GlslImportData{"asin", "asin"},
                                         GlslImportData{"atan", "atan"},
                                         GlslImportData{"cos", "cos"},
                                         GlslImportData{"cosh", "cosh"},
                                         GlslImportData{"ceil", "ceil"},
                                         GlslImportData{"exp", "exp"},
                                         GlslImportData{"exp2", "exp2"},
                                         GlslImportData{"floor", "floor"},
                                         GlslImportData{"fract", "fract"},
                                         GlslImportData{"inverseSqrt", "inversesqrt"},
                                         GlslImportData{"length", "length"},
                                         GlslImportData{"log", "log"},
                                         GlslImportData{"log2", "log2"},
                                         GlslImportData{"round", "round"},
                                         GlslImportData{"sign", "sign"},
                                         GlslImportData{"sin", "sin"},
                                         GlslImportData{"sinh", "sinh"},
                                         GlslImportData{"sqrt", "sqrt"},
                                         GlslImportData{"tan", "tan"},
                                         GlslImportData{"tanh", "tanh"},
                                         GlslImportData{"trunc", "trunc"}));

using GlslImportData_SingleIntParamTest = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_SingleIntParamTest, IntScalar) {
    auto param = GetParam();

    auto* expr = Call(param.name, Expr(1_i));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.glsl_name) + "(1)");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_SingleIntParamTest,
                         testing::Values(GlslImportData{"abs", "abs"}));

using GlslImportData_SingleVectorParamTest = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_SingleVectorParamTest, FloatVector) {
    auto param = GetParam();

    auto* expr = Call(param.name, Call<vec3<f32>>(0.1_f, 0.2_f, 0.3_f));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(
        out.str(),
        std::string(param.glsl_name) +
            "(vec3(0.10000000149011611938f, 0.20000000298023223877f, 0.30000001192092895508f))");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_SingleVectorParamTest,
                         testing::Values(GlslImportData{"abs", "abs"},
                                         GlslImportData{"acos", "acos"},
                                         GlslImportData{"asin", "asin"},
                                         GlslImportData{"atan", "atan"},
                                         GlslImportData{"cos", "cos"},
                                         GlslImportData{"cosh", "cosh"},
                                         GlslImportData{"ceil", "ceil"},
                                         GlslImportData{"exp", "exp"},
                                         GlslImportData{"exp2", "exp2"},
                                         GlslImportData{"floor", "floor"},
                                         GlslImportData{"fract", "fract"},
                                         GlslImportData{"inverseSqrt", "inversesqrt"},
                                         GlslImportData{"length", "length"},
                                         GlslImportData{"log", "log"},
                                         GlslImportData{"log2", "log2"},
                                         GlslImportData{"normalize", "normalize"},
                                         GlslImportData{"round", "round"},
                                         GlslImportData{"sign", "sign"},
                                         GlslImportData{"sin", "sin"},
                                         GlslImportData{"sinh", "sinh"},
                                         GlslImportData{"sqrt", "sqrt"},
                                         GlslImportData{"tan", "tan"},
                                         GlslImportData{"tanh", "tanh"},
                                         GlslImportData{"trunc", "trunc"}));

using GlslImportData_DualParam_ScalarTest = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_DualParam_ScalarTest, Float) {
    auto param = GetParam();

    auto* expr = Call(param.name, 1_f, 2_f);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.glsl_name) + "(1.0f, 2.0f)");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_DualParam_ScalarTest,
                         testing::Values(GlslImportData{"atan2", "atan"},
                                         GlslImportData{"distance", "distance"},
                                         GlslImportData{"max", "max"},
                                         GlslImportData{"min", "min"},
                                         GlslImportData{"pow", "pow"},
                                         GlslImportData{"step", "step"}));

using GlslImportData_DualParam_VectorTest = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_DualParam_VectorTest, Float) {
    auto param = GetParam();

    auto* expr = Call(param.name, Call<vec3<f32>>(1_f, 2_f, 3_f), Call<vec3<f32>>(4_f, 5_f, 6_f));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(),
              std::string(param.glsl_name) + "(vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f))");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_DualParam_VectorTest,
                         testing::Values(GlslImportData{"atan2", "atan"},
                                         GlslImportData{"cross", "cross"},
                                         GlslImportData{"distance", "distance"},
                                         GlslImportData{"max", "max"},
                                         GlslImportData{"min", "min"},
                                         GlslImportData{"pow", "pow"},
                                         GlslImportData{"reflect", "reflect"},
                                         GlslImportData{"step", "step"}));

using GlslImportData_DualParam_Int_Test = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_DualParam_Int_Test, IntScalar) {
    auto param = GetParam();

    auto* expr = Call(param.name, 1_i, 2_i);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.glsl_name) + "(1, 2)");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_DualParam_Int_Test,
                         testing::Values(GlslImportData{"max", "max"},
                                         GlslImportData{"min", "min"}));

using GlslImportData_TripleParam_ScalarTest = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_TripleParam_ScalarTest, Float) {
    auto param = GetParam();

    auto* expr = Call(param.name, 1_f, 2_f, 3_f);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.glsl_name) + "(1.0f, 2.0f, 3.0f)");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_TripleParam_ScalarTest,
                         testing::Values(GlslImportData{"mix", "mix"},
                                         GlslImportData{"clamp", "clamp"},
                                         GlslImportData{"smoothstep", "smoothstep"}));

using GlslImportData_TripleParam_VectorTest = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_TripleParam_VectorTest, Float) {
    auto param = GetParam();

    auto* expr = Call(param.name, Call<vec3<f32>>(1_f, 2_f, 3_f), Call<vec3<f32>>(4_f, 5_f, 6_f),
                      Call<vec3<f32>>(7_f, 8_f, 9_f));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(),
              std::string(param.glsl_name) +
                  R"((vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f)))");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_TripleParam_VectorTest,
                         testing::Values(GlslImportData{"faceForward", "faceforward"},
                                         GlslImportData{"clamp", "clamp"},
                                         GlslImportData{"smoothstep", "smoothstep"}));

using GlslImportData_TripleParam_Int_Test = TestParamHelper<GlslImportData>;
TEST_P(GlslImportData_TripleParam_Int_Test, IntScalar) {
    auto param = GetParam();

    auto* expr = Call(param.name, 1_i, 2_i, 3_i);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string(param.glsl_name) + "(1, 2, 3)");
}
INSTANTIATE_TEST_SUITE_P(GlslASTPrinterTest_Import,
                         GlslImportData_TripleParam_Int_Test,
                         testing::Values(GlslImportData{"clamp", "clamp"}));

TEST_F(GlslASTPrinterTest_Import, GlslImportData_Determinant) {
    GlobalVar("var", ty.mat3x3<f32>(), core::AddressSpace::kPrivate);

    auto* expr = Call("determinant", "var");
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitCall(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), std::string("determinant(var)"));
}

}  // namespace
}  // namespace tint::glsl::writer
