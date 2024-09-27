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
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::glsl::writer {
namespace {

using ::testing::HasSubstr;
using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using GlslASTPrinterTest_Builtin = TestHelper;

enum class CallParamType {
    kF32,
    kU32,
    kBool,
    kF16,
};

struct BuiltinData {
    wgsl::BuiltinFn builtin;
    CallParamType type;
    const char* glsl_name;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.glsl_name << "<";
    switch (data.type) {
        case CallParamType::kF32:
            out << "f32";
            break;
        case CallParamType::kU32:
            out << "u32";
            break;
        case CallParamType::kBool:
            out << "bool";
            break;
        case CallParamType::kF16:
            out << "f16";
            break;
    }
    out << ">";
    return out;
}

const ast::CallExpression* GenerateCall(wgsl::BuiltinFn builtin,
                                        CallParamType type,
                                        ProgramBuilder* builder) {
    std::string name;
    StringStream str;
    str << name << builtin;
    switch (builtin) {
        case wgsl::BuiltinFn::kAcos:
        case wgsl::BuiltinFn::kAsin:
        case wgsl::BuiltinFn::kAtan:
        case wgsl::BuiltinFn::kCeil:
        case wgsl::BuiltinFn::kCos:
        case wgsl::BuiltinFn::kCosh:
        case wgsl::BuiltinFn::kDpdx:
        case wgsl::BuiltinFn::kDpdxCoarse:
        case wgsl::BuiltinFn::kDpdxFine:
        case wgsl::BuiltinFn::kDpdy:
        case wgsl::BuiltinFn::kDpdyCoarse:
        case wgsl::BuiltinFn::kDpdyFine:
        case wgsl::BuiltinFn::kExp:
        case wgsl::BuiltinFn::kExp2:
        case wgsl::BuiltinFn::kFloor:
        case wgsl::BuiltinFn::kFract:
        case wgsl::BuiltinFn::kFwidth:
        case wgsl::BuiltinFn::kFwidthCoarse:
        case wgsl::BuiltinFn::kFwidthFine:
        case wgsl::BuiltinFn::kInverseSqrt:
        case wgsl::BuiltinFn::kLength:
        case wgsl::BuiltinFn::kLog:
        case wgsl::BuiltinFn::kLog2:
        case wgsl::BuiltinFn::kNormalize:
        case wgsl::BuiltinFn::kRound:
        case wgsl::BuiltinFn::kSin:
        case wgsl::BuiltinFn::kSinh:
        case wgsl::BuiltinFn::kSqrt:
        case wgsl::BuiltinFn::kTan:
        case wgsl::BuiltinFn::kTanh:
        case wgsl::BuiltinFn::kTrunc:
        case wgsl::BuiltinFn::kSign:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2");
            } else {
                return builder->Call(str.str(), "f2");
            }
        case wgsl::BuiltinFn::kLdexp:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "i2");
            } else {
                return builder->Call(str.str(), "f2", "i2");
            }
        case wgsl::BuiltinFn::kAtan2:
        case wgsl::BuiltinFn::kDot:
        case wgsl::BuiltinFn::kDistance:
        case wgsl::BuiltinFn::kPow:
        case wgsl::BuiltinFn::kReflect:
        case wgsl::BuiltinFn::kStep:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2");
            } else {
                return builder->Call(str.str(), "f2", "f2");
            }
        case wgsl::BuiltinFn::kCross:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h3", "h3");
            } else {
                return builder->Call(str.str(), "f3", "f3");
            }
        case wgsl::BuiltinFn::kFma:
        case wgsl::BuiltinFn::kMix:
        case wgsl::BuiltinFn::kFaceForward:
        case wgsl::BuiltinFn::kSmoothstep:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "h2");
            } else {
                return builder->Call(str.str(), "f2", "f2", "f2");
            }
        case wgsl::BuiltinFn::kAll:
        case wgsl::BuiltinFn::kAny:
            return builder->Call(str.str(), "b2");
        case wgsl::BuiltinFn::kAbs:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2");
            } else {
                return builder->Call(str.str(), "u2");
            }
        case wgsl::BuiltinFn::kCountOneBits:
        case wgsl::BuiltinFn::kReverseBits:
            return builder->Call(str.str(), "u2");
        case wgsl::BuiltinFn::kMax:
        case wgsl::BuiltinFn::kMin:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2", "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2");
            } else {
                return builder->Call(str.str(), "u2", "u2");
            }
        case wgsl::BuiltinFn::kClamp:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2", "f2", "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "h2");
            } else {
                return builder->Call(str.str(), "u2", "u2", "u2");
            }
        case wgsl::BuiltinFn::kSelect:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "b2");
            } else {
                return builder->Call(str.str(), "f2", "f2", "b2");
            }
        case wgsl::BuiltinFn::kDeterminant:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "hm2x2");
            } else {
                return builder->Call(str.str(), "m2x2");
            }
        case wgsl::BuiltinFn::kTranspose:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "hm3x2");
            } else {
                return builder->Call(str.str(), "m3x2");
            }
        default:
            break;
    }
    return nullptr;
}
using GlslBuiltinTest = TestParamHelper<BuiltinData>;
TEST_P(GlslBuiltinTest, Emit) {
    auto param = GetParam();

    if (param.type == CallParamType::kF16) {
        Enable(wgsl::Extension::kF16);

        GlobalVar("h2", ty.vec2<f16>(), core::AddressSpace::kPrivate);
        GlobalVar("h3", ty.vec3<f16>(), core::AddressSpace::kPrivate);
        GlobalVar("hm2x2", ty.mat2x2<f16>(), core::AddressSpace::kPrivate);
        GlobalVar("hm3x2", ty.mat3x2<f16>(), core::AddressSpace::kPrivate);
    }

    GlobalVar("f2", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("f3", ty.vec3<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("u2", ty.vec2<u32>(), core::AddressSpace::kPrivate);
    GlobalVar("i2", ty.vec2<i32>(), core::AddressSpace::kPrivate);
    GlobalVar("b2", ty.vec2<bool>(), core::AddressSpace::kPrivate);
    GlobalVar("m2x2", ty.mat2x2<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("m3x2", ty.mat3x2<f32>(), core::AddressSpace::kPrivate);

    auto* call = GenerateCall(param.builtin, param.type, this);
    ASSERT_NE(nullptr, call) << "Unhandled builtin";
    Func("func", tint::Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         },
         Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    auto* sem = program->Sem().Get<sem::Call>(call);
    ASSERT_NE(sem, nullptr);
    auto* target = sem->Target();
    ASSERT_NE(target, nullptr);
    auto* builtin = target->As<sem::BuiltinFn>();
    ASSERT_NE(builtin, nullptr);

    EXPECT_EQ(gen.generate_builtin_name(builtin), param.glsl_name);
}
INSTANTIATE_TEST_SUITE_P(
    GlslASTPrinterTest_Builtin,
    GlslBuiltinTest,
    testing::Values(/* Logical built-in */
                    BuiltinData{wgsl::BuiltinFn::kAll, CallParamType::kBool, "all"},
                    BuiltinData{wgsl::BuiltinFn::kAny, CallParamType::kBool, "any"},
                    /* Float built-in */
                    BuiltinData{wgsl::BuiltinFn::kAbs, CallParamType::kF32, "abs"},
                    BuiltinData{wgsl::BuiltinFn::kAbs, CallParamType::kF16, "abs"},
                    BuiltinData{wgsl::BuiltinFn::kAcos, CallParamType::kF32, "acos"},
                    BuiltinData{wgsl::BuiltinFn::kAcos, CallParamType::kF16, "acos"},
                    BuiltinData{wgsl::BuiltinFn::kAsin, CallParamType::kF32, "asin"},
                    BuiltinData{wgsl::BuiltinFn::kAsin, CallParamType::kF16, "asin"},
                    BuiltinData{wgsl::BuiltinFn::kAtan, CallParamType::kF32, "atan"},
                    BuiltinData{wgsl::BuiltinFn::kAtan, CallParamType::kF16, "atan"},
                    BuiltinData{wgsl::BuiltinFn::kAtan2, CallParamType::kF32, "atan"},
                    BuiltinData{wgsl::BuiltinFn::kAtan2, CallParamType::kF16, "atan"},
                    BuiltinData{wgsl::BuiltinFn::kCeil, CallParamType::kF32, "ceil"},
                    BuiltinData{wgsl::BuiltinFn::kCeil, CallParamType::kF16, "ceil"},
                    BuiltinData{wgsl::BuiltinFn::kClamp, CallParamType::kF32, "clamp"},
                    BuiltinData{wgsl::BuiltinFn::kClamp, CallParamType::kF16, "clamp"},
                    BuiltinData{wgsl::BuiltinFn::kCos, CallParamType::kF32, "cos"},
                    BuiltinData{wgsl::BuiltinFn::kCos, CallParamType::kF16, "cos"},
                    BuiltinData{wgsl::BuiltinFn::kCosh, CallParamType::kF32, "cosh"},
                    BuiltinData{wgsl::BuiltinFn::kCosh, CallParamType::kF16, "cosh"},
                    BuiltinData{wgsl::BuiltinFn::kCross, CallParamType::kF32, "cross"},
                    BuiltinData{wgsl::BuiltinFn::kCross, CallParamType::kF16, "cross"},
                    BuiltinData{wgsl::BuiltinFn::kDistance, CallParamType::kF32, "distance"},
                    BuiltinData{wgsl::BuiltinFn::kDistance, CallParamType::kF16, "distance"},
                    BuiltinData{wgsl::BuiltinFn::kExp, CallParamType::kF32, "exp"},
                    BuiltinData{wgsl::BuiltinFn::kExp, CallParamType::kF16, "exp"},
                    BuiltinData{wgsl::BuiltinFn::kExp2, CallParamType::kF32, "exp2"},
                    BuiltinData{wgsl::BuiltinFn::kExp2, CallParamType::kF16, "exp2"},
                    BuiltinData{wgsl::BuiltinFn::kFaceForward, CallParamType::kF32, "faceforward"},
                    BuiltinData{wgsl::BuiltinFn::kFaceForward, CallParamType::kF16, "faceforward"},
                    BuiltinData{wgsl::BuiltinFn::kFloor, CallParamType::kF32, "floor"},
                    BuiltinData{wgsl::BuiltinFn::kFloor, CallParamType::kF16, "floor"},
                    BuiltinData{wgsl::BuiltinFn::kFma, CallParamType::kF32, "fma"},
                    BuiltinData{wgsl::BuiltinFn::kFma, CallParamType::kF16, "fma"},
                    BuiltinData{wgsl::BuiltinFn::kFract, CallParamType::kF32, "fract"},
                    BuiltinData{wgsl::BuiltinFn::kFract, CallParamType::kF16, "fract"},
                    BuiltinData{wgsl::BuiltinFn::kInverseSqrt, CallParamType::kF32, "inversesqrt"},
                    BuiltinData{wgsl::BuiltinFn::kInverseSqrt, CallParamType::kF16, "inversesqrt"},
                    BuiltinData{wgsl::BuiltinFn::kLdexp, CallParamType::kF32, "ldexp"},
                    BuiltinData{wgsl::BuiltinFn::kLdexp, CallParamType::kF16, "ldexp"},
                    BuiltinData{wgsl::BuiltinFn::kLength, CallParamType::kF32, "length"},
                    BuiltinData{wgsl::BuiltinFn::kLength, CallParamType::kF16, "length"},
                    BuiltinData{wgsl::BuiltinFn::kLog, CallParamType::kF32, "log"},
                    BuiltinData{wgsl::BuiltinFn::kLog, CallParamType::kF16, "log"},
                    BuiltinData{wgsl::BuiltinFn::kLog2, CallParamType::kF32, "log2"},
                    BuiltinData{wgsl::BuiltinFn::kLog2, CallParamType::kF16, "log2"},
                    BuiltinData{wgsl::BuiltinFn::kMax, CallParamType::kF32, "max"},
                    BuiltinData{wgsl::BuiltinFn::kMax, CallParamType::kF16, "max"},
                    BuiltinData{wgsl::BuiltinFn::kMin, CallParamType::kF32, "min"},
                    BuiltinData{wgsl::BuiltinFn::kMin, CallParamType::kF16, "min"},
                    BuiltinData{wgsl::BuiltinFn::kMix, CallParamType::kF32, "mix"},
                    BuiltinData{wgsl::BuiltinFn::kMix, CallParamType::kF16, "mix"},
                    BuiltinData{wgsl::BuiltinFn::kNormalize, CallParamType::kF32, "normalize"},
                    BuiltinData{wgsl::BuiltinFn::kNormalize, CallParamType::kF16, "normalize"},
                    BuiltinData{wgsl::BuiltinFn::kPow, CallParamType::kF32, "pow"},
                    BuiltinData{wgsl::BuiltinFn::kPow, CallParamType::kF16, "pow"},
                    BuiltinData{wgsl::BuiltinFn::kReflect, CallParamType::kF32, "reflect"},
                    BuiltinData{wgsl::BuiltinFn::kReflect, CallParamType::kF16, "reflect"},
                    BuiltinData{wgsl::BuiltinFn::kSign, CallParamType::kF32, "sign"},
                    BuiltinData{wgsl::BuiltinFn::kSign, CallParamType::kF16, "sign"},
                    BuiltinData{wgsl::BuiltinFn::kSin, CallParamType::kF32, "sin"},
                    BuiltinData{wgsl::BuiltinFn::kSin, CallParamType::kF16, "sin"},
                    BuiltinData{wgsl::BuiltinFn::kSinh, CallParamType::kF32, "sinh"},
                    BuiltinData{wgsl::BuiltinFn::kSinh, CallParamType::kF16, "sinh"},
                    BuiltinData{wgsl::BuiltinFn::kSmoothstep, CallParamType::kF32, "smoothstep"},
                    BuiltinData{wgsl::BuiltinFn::kSmoothstep, CallParamType::kF16, "smoothstep"},
                    BuiltinData{wgsl::BuiltinFn::kSqrt, CallParamType::kF32, "sqrt"},
                    BuiltinData{wgsl::BuiltinFn::kSqrt, CallParamType::kF16, "sqrt"},
                    BuiltinData{wgsl::BuiltinFn::kStep, CallParamType::kF32, "step"},
                    BuiltinData{wgsl::BuiltinFn::kStep, CallParamType::kF16, "step"},
                    BuiltinData{wgsl::BuiltinFn::kTan, CallParamType::kF32, "tan"},
                    BuiltinData{wgsl::BuiltinFn::kTan, CallParamType::kF16, "tan"},
                    BuiltinData{wgsl::BuiltinFn::kTanh, CallParamType::kF32, "tanh"},
                    BuiltinData{wgsl::BuiltinFn::kTanh, CallParamType::kF16, "tanh"},
                    BuiltinData{wgsl::BuiltinFn::kTrunc, CallParamType::kF32, "trunc"},
                    BuiltinData{wgsl::BuiltinFn::kTrunc, CallParamType::kF16, "trunc"},
                    /* Integer built-in */
                    BuiltinData{wgsl::BuiltinFn::kAbs, CallParamType::kU32, "abs"},
                    BuiltinData{wgsl::BuiltinFn::kClamp, CallParamType::kU32, "clamp"},
                    BuiltinData{wgsl::BuiltinFn::kCountOneBits, CallParamType::kU32, "bitCount"},
                    BuiltinData{wgsl::BuiltinFn::kMax, CallParamType::kU32, "max"},
                    BuiltinData{wgsl::BuiltinFn::kMin, CallParamType::kU32, "min"},
                    BuiltinData{wgsl::BuiltinFn::kReverseBits, CallParamType::kU32,
                                "bitfieldReverse"},
                    BuiltinData{wgsl::BuiltinFn::kRound, CallParamType::kU32, "round"},
                    /* Matrix built-in */
                    BuiltinData{wgsl::BuiltinFn::kDeterminant, CallParamType::kF32, "determinant"},
                    BuiltinData{wgsl::BuiltinFn::kDeterminant, CallParamType::kF16, "determinant"},
                    BuiltinData{wgsl::BuiltinFn::kTranspose, CallParamType::kF32, "transpose"},
                    BuiltinData{wgsl::BuiltinFn::kTranspose, CallParamType::kF16, "transpose"},
                    /* Vector built-in */
                    BuiltinData{wgsl::BuiltinFn::kDot, CallParamType::kF32, "dot"},
                    BuiltinData{wgsl::BuiltinFn::kDot, CallParamType::kF16, "dot"},
                    /* Derivate built-in */
                    BuiltinData{wgsl::BuiltinFn::kDpdx, CallParamType::kF32, "dFdx"},
                    BuiltinData{wgsl::BuiltinFn::kDpdxCoarse, CallParamType::kF32, "dFdx"},
                    BuiltinData{wgsl::BuiltinFn::kDpdxFine, CallParamType::kF32, "dFdx"},
                    BuiltinData{wgsl::BuiltinFn::kDpdy, CallParamType::kF32, "dFdy"},
                    BuiltinData{wgsl::BuiltinFn::kDpdyCoarse, CallParamType::kF32, "dFdy"},
                    BuiltinData{wgsl::BuiltinFn::kDpdyFine, CallParamType::kF32, "dFdy"},
                    BuiltinData{wgsl::BuiltinFn::kFwidth, CallParamType::kF32, "fwidth"},
                    BuiltinData{wgsl::BuiltinFn::kFwidthCoarse, CallParamType::kF32, "fwidth"},
                    BuiltinData{wgsl::BuiltinFn::kFwidthFine, CallParamType::kF32, "fwidth"}));

TEST_F(GlslASTPrinterTest_Builtin, Builtin_Call) {
    auto* call = Call("dot", "param1", "param2");

    GlobalVar("param1", ty.vec3<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("param2", ty.vec3<f32>(), core::AddressSpace::kPrivate);

    WrapInFunction(Decl(Var("r", call)));

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "dot(param1, param2)");
}

TEST_F(GlslASTPrinterTest_Builtin, Select_Scalar) {
    GlobalVar("a", Expr(1_f), core::AddressSpace::kPrivate);
    GlobalVar("b", Expr(2_f), core::AddressSpace::kPrivate);
    auto* call = Call("select", "a", "b", true);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "mix(a, b, true)");
}

TEST_F(GlslASTPrinterTest_Builtin, Select_Vector) {
    GlobalVar("a", Call<vec2<i32>>(1_i, 2_i), core::AddressSpace::kPrivate);
    GlobalVar("b", Call<vec2<i32>>(3_i, 4_i), core::AddressSpace::kPrivate);
    auto* call = Call("select", "a", "b", Call<vec2<bool>>(true, false));
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "mix(a, b, bvec2(true, false))");
}

TEST_F(GlslASTPrinterTest_Builtin, FMA_f32) {
    auto* call = Call("fma", "a", "b", "c");

    GlobalVar("a", ty.vec3<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("c", ty.vec3<f32>(), core::AddressSpace::kPrivate);

    WrapInFunction(Decl(Var("r", call)));

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "((a) * (b) + (c))");
}

TEST_F(GlslASTPrinterTest_Builtin, FMA_f16) {
    Enable(wgsl::Extension::kF16);

    GlobalVar("a", ty.vec3<f16>(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f16>(), core::AddressSpace::kPrivate);
    GlobalVar("c", ty.vec3<f16>(), core::AddressSpace::kPrivate);

    auto* call = Call("fma", "a", "b", "c");
    WrapInFunction(Decl(Var("r", call)));

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "((a) * (b) + (c))");
}

TEST_F(GlslASTPrinterTest_Builtin, Runtime_Modf_Scalar_f32) {
    WrapInFunction(Decl(Let("f", Expr(1.5_f))),  //
                   Decl(Let("v", Call("modf", "f"))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct modf_result_f32 {
  float fract;
  float whole;
};

modf_result_f32 tint_modf(float param_0) {
  modf_result_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void test_function() {
  float f = 1.5f;
  modf_result_f32 v = tint_modf(f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Runtime_Modf_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("f", Expr(1.5_h))),  //
                   Decl(Let("v", Call("modf", "f"))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};

modf_result_f16 tint_modf(float16_t param_0) {
  modf_result_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void test_function() {
  float16_t f = 1.5hf;
  modf_result_f16 v = tint_modf(f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Runtime_Modf_Vector_f32) {
    WrapInFunction(Decl(Let("f", Call<vec3<f32>>(1.5_f, 2.5_f, 3.5_f))),  //
                   Decl(Let("v", Call("modf", "f"))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct modf_result_vec3_f32 {
  vec3 fract;
  vec3 whole;
};

modf_result_vec3_f32 tint_modf(vec3 param_0) {
  modf_result_vec3_f32 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void test_function() {
  vec3 f = vec3(1.5f, 2.5f, 3.5f);
  modf_result_vec3_f32 v = tint_modf(f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Runtime_Modf_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("f", Call<vec3<f16>>(1.5_h, 2.5_h, 3.5_h))),  //
                   Decl(Let("v", Call("modf", "f"))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_vec3_f16 {
  f16vec3 fract;
  f16vec3 whole;
};

modf_result_vec3_f16 tint_modf(f16vec3 param_0) {
  modf_result_vec3_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void test_function() {
  f16vec3 f = f16vec3(1.5hf, 2.5hf, 3.5hf);
  modf_result_vec3_f16 v = tint_modf(f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Const_Modf_Scalar_f32) {
    WrapInFunction(Decl(Let("v", Call("modf", 1.5_f))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct modf_result_f32 {
  float fract;
  float whole;
};


void test_function() {
  modf_result_f32 v = modf_result_f32(0.5f, 1.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Const_Modf_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("modf", 1.5_h))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};


void test_function() {
  modf_result_f16 v = modf_result_f16(0.5hf, 1.0hf);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Const_Modf_Vector_f32) {
    WrapInFunction(Decl(Let("v", Call("modf", Call<vec3<f32>>(1.5_f, 2.5_f, 3.5_f)))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct modf_result_vec3_f32 {
  vec3 fract;
  vec3 whole;
};


void test_function() {
  modf_result_vec3_f32 v = modf_result_vec3_f32(vec3(0.5f), vec3(1.0f, 2.0f, 3.0f));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Const_Modf_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("modf", Call<vec3<f16>>(1.5_h, 2.5_h, 3.5_h)))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_vec3_f16 {
  f16vec3 fract;
  f16vec3 whole;
};


void test_function() {
  modf_result_vec3_f16 v = modf_result_vec3_f16(f16vec3(0.5hf), f16vec3(1.0hf, 2.0hf, 3.0hf));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Runtime_Frexp_Scalar_f32) {
    WrapInFunction(Var("f", Expr(1_f)),  //
                   Var("v", Call("frexp", "f")));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};

frexp_result_f32 tint_frexp(float param_0) {
  frexp_result_f32 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void test_function() {
  float f = 1.0f;
  frexp_result_f32 v = tint_frexp(f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Runtime_Frexp_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Var("f", Expr(1_h)),  //
                   Var("v", Call("frexp", "f")));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct frexp_result_f16 {
  float16_t fract;
  int exp;
};

frexp_result_f16 tint_frexp(float16_t param_0) {
  frexp_result_f16 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void test_function() {
  float16_t f = 1.0hf;
  frexp_result_f16 v = tint_frexp(f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Runtime_Frexp_Vector_f32) {
    WrapInFunction(Var("f", Call<vec3<f32>>()),  //
                   Var("v", Call("frexp", "f")));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct frexp_result_vec3_f32 {
  vec3 fract;
  ivec3 exp;
};

frexp_result_vec3_f32 tint_frexp(vec3 param_0) {
  frexp_result_vec3_f32 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void test_function() {
  vec3 f = vec3(0.0f);
  frexp_result_vec3_f32 v = tint_frexp(f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Runtime_Frexp_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Var("f", Call<vec3<f16>>()),  //
                   Var("v", Call("frexp", "f")));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct frexp_result_vec3_f16 {
  f16vec3 fract;
  ivec3 exp;
};

frexp_result_vec3_f16 tint_frexp(f16vec3 param_0) {
  frexp_result_vec3_f16 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void test_function() {
  f16vec3 f = f16vec3(0.0hf);
  frexp_result_vec3_f16 v = tint_frexp(f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Const_Frexp_Scalar_f32) {
    WrapInFunction(Decl(Let("v", Call("frexp", 1_f))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};


void test_function() {
  frexp_result_f32 v = frexp_result_f32(0.5f, 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Const_Frexp_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("frexp", 1_h))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct frexp_result_f16 {
  float16_t fract;
  int exp;
};


void test_function() {
  frexp_result_f16 v = frexp_result_f16(0.5hf, 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Const_Frexp_Vector_f32) {
    WrapInFunction(Decl(Let("v", Call("frexp", Call<vec3<f32>>()))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

struct frexp_result_vec3_f32 {
  vec3 fract;
  ivec3 exp;
};


void test_function() {
  frexp_result_vec3_f32 v = frexp_result_vec3_f32(vec3(0.0f), ivec3(0));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Const_Frexp_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("frexp", Call<vec3<f16>>()))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct frexp_result_vec3_f16 {
  f16vec3 fract;
  ivec3 exp;
};


void test_function() {
  frexp_result_vec3_f16 v = frexp_result_vec3_f16(f16vec3(0.0hf), ivec3(0));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Degrees_Scalar_f32) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

float tint_degrees(float param_0) {
  return param_0 * 57.29577951308232286465f;
}


void test_function() {
  float val = 0.0f;
  float tint_symbol = tint_degrees(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Degrees_Vector_f32) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec3 tint_degrees(vec3 param_0) {
  return param_0 * 57.29577951308232286465f;
}


void test_function() {
  vec3 val = vec3(0.0f, 0.0f, 0.0f);
  vec3 tint_symbol = tint_degrees(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Degrees_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    auto* val = Var("val", ty.f16());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_degrees(float16_t param_0) {
  return param_0 * 57.29577951308232286465hf;
}


void test_function() {
  float16_t val = 0.0hf;
  float16_t tint_symbol = tint_degrees(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Degrees_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    auto* val = Var("val", ty.vec3<f16>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec3 tint_degrees(f16vec3 param_0) {
  return param_0 * 57.29577951308232286465hf;
}


void test_function() {
  f16vec3 val = f16vec3(0.0hf, 0.0hf, 0.0hf);
  f16vec3 tint_symbol = tint_degrees(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Radians_Scalar_f32) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

float tint_radians(float param_0) {
  return param_0 * 0.01745329251994329547f;
}


void test_function() {
  float val = 0.0f;
  float tint_symbol = tint_radians(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Radians_Vector_f32) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec3 tint_radians(vec3 param_0) {
  return param_0 * 0.01745329251994329547f;
}


void test_function() {
  vec3 val = vec3(0.0f, 0.0f, 0.0f);
  vec3 tint_symbol = tint_radians(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Radians_Scalar_f16) {
    Enable(wgsl::Extension::kF16);

    auto* val = Var("val", ty.f16());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_radians(float16_t param_0) {
  return param_0 * 0.01745329251994329547hf;
}


void test_function() {
  float16_t val = 0.0hf;
  float16_t tint_symbol = tint_radians(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Radians_Vector_f16) {
    Enable(wgsl::Extension::kF16);

    auto* val = Var("val", ty.vec3<f16>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec3 tint_radians(f16vec3 param_0) {
  return param_0 * 0.01745329251994329547hf;
}


void test_function() {
  f16vec3 val = f16vec3(0.0hf, 0.0hf, 0.0hf);
  f16vec3 tint_symbol = tint_radians(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, ExtractBits) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    WrapInFunction(v, offset, count, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

uvec3 tint_extract_bits(uvec3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

void test_function() {
  uvec3 v = uvec3(0u, 0u, 0u);
  uint offset = 0u;
  uint count = 0u;
  uvec3 tint_symbol = tint_extract_bits(v, offset, count);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, InsertBits) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* n = Var("n", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    WrapInFunction(v, n, offset, count, call);

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

uvec3 tint_insert_bits(uvec3 v, uvec3 n, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldInsert(v, n, int(s), int((e - s)));
}

void test_function() {
  uvec3 v = uvec3(0u, 0u, 0u);
  uvec3 n = uvec3(0u, 0u, 0u);
  uint offset = 0u;
  uint count = 0u;
  uvec3 tint_symbol = tint_insert_bits(v, n, offset, count);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Pack4x8Snorm) {
    auto* call = Call("pack4x8snorm", "p1");
    GlobalVar("p1", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec4 p1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packSnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Pack4x8Unorm) {
    auto* call = Call("pack4x8unorm", "p1");
    GlobalVar("p1", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec4 p1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packUnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Pack2x16Snorm) {
    auto* call = Call("pack2x16snorm", "p1");
    GlobalVar("p1", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packSnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Pack2x16Unorm) {
    auto* call = Call("pack2x16unorm", "p1");
    GlobalVar("p1", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packUnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Pack2x16Float) {
    auto* call = Call("pack2x16float", "p1");
    GlobalVar("p1", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packHalf2x16(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Unpack4x8Snorm) {
    auto* call = Call("unpack4x8snorm", "p1");
    GlobalVar("p1", ty.u32(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec4 r = unpackSnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Unpack4x8Unorm) {
    auto* call = Call("unpack4x8unorm", "p1");
    GlobalVar("p1", ty.u32(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec4 r = unpackUnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Unpack2x16Snorm) {
    auto* call = Call("unpack2x16snorm", "p1");
    GlobalVar("p1", ty.u32(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec2 r = unpackSnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Unpack2x16Unorm) {
    auto* call = Call("unpack2x16unorm", "p1");
    GlobalVar("p1", ty.u32(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec2 r = unpackUnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, Unpack2x16Float) {
    auto* call = Call("unpack2x16float", "p1");
    GlobalVar("p1", ty.u32(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec2 r = unpackHalf2x16(p1);
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, StorageBarrier) {
    Func("main", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("storageBarrier")),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  { memoryBarrierBuffer(); barrier(); };
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, WorkgroupBarrier) {
    Func("main", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("workgroupBarrier")),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, DotI32) {
    GlobalVar("v", ty.vec3<i32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", Call("dot", "v", "v"))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

int tint_int_dot(ivec3 a, ivec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

ivec3 v = ivec3(0, 0, 0);
void test_function() {
  int r = tint_int_dot(v, v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, DotU32) {
    GlobalVar("v", ty.vec3<u32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", Call("dot", "v", "v"))));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

uint tint_int_dot(uvec3 a, uvec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

uvec3 v = uvec3(0u, 0u, 0u);
void test_function() {
  uint r = tint_int_dot(v, v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, QuantizeToF16_Scalar) {
    GlobalVar("v", Expr(2_f), core::AddressSpace::kPrivate);
    WrapInFunction(Call("quantizeToF16", "v"));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

float tint_quantizeToF16(float param_0) {
  return unpackHalf2x16(packHalf2x16(vec2(param_0))).x;
}


float v = 2.0f;
void test_function() {
  float tint_symbol = tint_quantizeToF16(v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, QuantizeToF16_Vec2) {
    GlobalVar("v", Call<vec2<f32>>(2_f), core::AddressSpace::kPrivate);
    WrapInFunction(Call("quantizeToF16", "v"));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec2 tint_quantizeToF16(vec2 param_0) {
  return unpackHalf2x16(packHalf2x16(param_0));
}


vec2 v = vec2(2.0f);
void test_function() {
  vec2 tint_symbol = tint_quantizeToF16(v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, QuantizeToF16_Vec3) {
    GlobalVar("v", Call<vec3<f32>>(2_f), core::AddressSpace::kPrivate);
    WrapInFunction(Call("quantizeToF16", "v"));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec3 tint_quantizeToF16(vec3 param_0) {
  return vec3(
    unpackHalf2x16(packHalf2x16(param_0.xy)),
    unpackHalf2x16(packHalf2x16(param_0.zz)).x);
}


vec3 v = vec3(2.0f);
void test_function() {
  vec3 tint_symbol = tint_quantizeToF16(v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslASTPrinterTest_Builtin, QuantizeToF16_Vec4) {
    GlobalVar("v", Call<vec4<f32>>(2_f), core::AddressSpace::kPrivate);
    WrapInFunction(Call("quantizeToF16", "v"));

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(#version 310 es

vec4 tint_quantizeToF16(vec4 param_0) {
  return vec4(
    unpackHalf2x16(packHalf2x16(param_0.xy)),
    unpackHalf2x16(packHalf2x16(param_0.zw)));
}


vec4 v = vec4(2.0f);
void test_function() {
  vec4 tint_symbol = tint_quantizeToF16(v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer
