// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using MslASTPrinterTest = TestHelper;

struct MslImportData {
    const char* name;
    const char* msl_name;
};
inline std::ostream& operator<<(std::ostream& out, MslImportData data) {
    out << data.name;
    return out;
}
using MslImportData_SingleParamTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_SingleParamTest, FloatScalar) {
    auto param = GetParam();
    auto* call = Call(param.name, 1_f);

    // The resolver will set the builtin data for the ident
    WrapInFunction(call);

    ASTPrinter& gen = Build();

    auto* sem = program->Sem().Get<sem::Call>(call);
    ASSERT_NE(sem, nullptr);
    auto* target = sem->Target();
    ASSERT_NE(target, nullptr);
    auto* builtin = target->As<sem::BuiltinFn>();
    ASSERT_NE(builtin, nullptr);

    ASSERT_EQ(gen.generate_builtin_name(builtin), param.msl_name);
}
INSTANTIATE_TEST_SUITE_P(MslASTPrinterTest,
                         MslImportData_SingleParamTest,
                         testing::Values(MslImportData{"abs", "fabs"},
                                         MslImportData{"acos", "acos"},
                                         MslImportData{"asin", "asin"},
                                         MslImportData{"atan", "atan"},
                                         MslImportData{"ceil", "ceil"},
                                         MslImportData{"cos", "cos"},
                                         MslImportData{"cosh", "cosh"},
                                         MslImportData{"exp", "exp"},
                                         MslImportData{"exp2", "exp2"},
                                         MslImportData{"floor", "floor"},
                                         MslImportData{"fract", "fract"},
                                         MslImportData{"inverseSqrt", "rsqrt"},
                                         MslImportData{"length", "length"},
                                         MslImportData{"log", "log"},
                                         MslImportData{"log2", "log2"},
                                         MslImportData{"round", "rint"},
                                         MslImportData{"sign", "sign"},
                                         MslImportData{"sin", "sin"},
                                         MslImportData{"sinh", "sinh"},
                                         MslImportData{"sqrt", "sqrt"},
                                         MslImportData{"tan", "tan"},
                                         MslImportData{"tanh", "tanh"},
                                         MslImportData{"trunc", "trunc"}));

TEST_F(MslASTPrinterTest, MslImportData_SingleParamTest_IntScalar) {
    auto* expr = Call("abs", 1_i);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), R"(abs(1))");
}

TEST_F(MslASTPrinterTest, MslImportData_SingleParamTest_ScalarLength) {
    auto* expr = Call("length", 2_f);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), R"(fabs(2.0f))");
}

using MslImportData_DualParam_ScalarTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParam_ScalarTest, Float) {
    auto param = GetParam();
    auto* expr = Call(param.name, 1_f, 2_f);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), std::string(param.msl_name) + "(1.0f, 2.0f)");
}
INSTANTIATE_TEST_SUITE_P(MslASTPrinterTest,
                         MslImportData_DualParam_ScalarTest,
                         testing::Values(MslImportData{"atan2", "atan2"},
                                         MslImportData{"max", "fmax"},
                                         MslImportData{"min", "fmin"},
                                         MslImportData{"pow", "powr"},
                                         MslImportData{"step", "step"}));

TEST_F(MslASTPrinterTest, MslImportData_DualParam_ScalarDistance) {
    auto* expr = Call("distance", 2_f, 3_f);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), R"(fabs(2.0f - 3.0f))");
}

using MslImportData_DualParam_VectorTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParam_VectorTest, Float) {
    auto param = GetParam();

    auto* expr = Call(param.name, Call<vec3<f32>>(1_f, 2_f, 3_f), Call<vec3<f32>>(4_f, 5_f, 6_f));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), std::string(param.msl_name) +
                             R"((float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f)))");
}
INSTANTIATE_TEST_SUITE_P(MslASTPrinterTest,
                         MslImportData_DualParam_VectorTest,
                         testing::Values(MslImportData{"atan2", "atan2"},
                                         MslImportData{"cross", "cross"},
                                         MslImportData{"distance", "distance"},
                                         MslImportData{"max", "fmax"},
                                         MslImportData{"min", "fmin"},
                                         MslImportData{"pow", "powr"},
                                         MslImportData{"reflect", "reflect"},
                                         MslImportData{"step", "step"}));

using MslImportData_DualParam_Int_Test = TestParamHelper<MslImportData>;
TEST_P(MslImportData_DualParam_Int_Test, IntScalar) {
    auto param = GetParam();

    auto* expr = Call(param.name, 1_i, 2_i);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), std::string(param.msl_name) + "(1, 2)");
}
INSTANTIATE_TEST_SUITE_P(MslASTPrinterTest,
                         MslImportData_DualParam_Int_Test,
                         testing::Values(MslImportData{"max", "max"}, MslImportData{"min", "min"}));

using MslImportData_TripleParam_ScalarTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_TripleParam_ScalarTest, Float) {
    auto param = GetParam();

    auto* expr = Call(param.name, 1_f, 2_f, 3_f);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), std::string(param.msl_name) + "(1.0f, 2.0f, 3.0f)");
}
INSTANTIATE_TEST_SUITE_P(MslASTPrinterTest,
                         MslImportData_TripleParam_ScalarTest,
                         testing::Values(MslImportData{"fma", "fma"},
                                         MslImportData{"mix", "mix"},
                                         MslImportData{"clamp", "clamp"},
                                         MslImportData{"smoothstep", "smoothstep"}));

using MslImportData_TripleParam_VectorTest = TestParamHelper<MslImportData>;
TEST_P(MslImportData_TripleParam_VectorTest, Float) {
    auto param = GetParam();

    auto* expr = Call(param.name, Call<vec3<f32>>(1_f, 2_f, 3_f), Call<vec3<f32>>(4_f, 5_f, 6_f),
                      Call<vec3<f32>>(7_f, 8_f, 9_f));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(
        out.str(),
        std::string(param.msl_name) +
            R"((float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)))");
}
INSTANTIATE_TEST_SUITE_P(MslASTPrinterTest,
                         MslImportData_TripleParam_VectorTest,
                         testing::Values(MslImportData{"faceForward", "faceforward"},
                                         MslImportData{"fma", "fma"},
                                         MslImportData{"clamp", "clamp"},
                                         MslImportData{"smoothstep", "smoothstep"}));

using MslImportData_TripleParam_Int_Test = TestParamHelper<MslImportData>;
TEST_P(MslImportData_TripleParam_Int_Test, IntScalar) {
    auto param = GetParam();

    auto* expr = Call(param.name, 1_i, 2_i, 3_i);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), std::string(param.msl_name) + "(1, 2, 3)");
}
INSTANTIATE_TEST_SUITE_P(MslASTPrinterTest,
                         MslImportData_TripleParam_Int_Test,
                         testing::Values(MslImportData{"clamp", "clamp"},
                                         MslImportData{"clamp", "clamp"}));

TEST_F(MslASTPrinterTest, MslImportData_Determinant) {
    GlobalVar("var", ty.mat3x3<f32>(), core::AddressSpace::kPrivate);

    auto* expr = Call("determinant", "var");

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), std::string("determinant(var)"));
}

TEST_F(MslASTPrinterTest, MslImportData_QuantizeToF16_Scalar) {
    GlobalVar("v", Expr(2_f), core::AddressSpace::kPrivate);

    auto* expr = Call("quantizeToF16", "v");
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float(half(v))");
}

TEST_F(MslASTPrinterTest, MslImportData_QuantizeToF16_Vector) {
    GlobalVar("v", Call<vec3<f32>>(2_f), core::AddressSpace::kPrivate);

    auto* expr = Call("quantizeToF16", "v");
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitCall(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float3(half3(v))");
}

}  // namespace
}  // namespace tint::msl::writer
