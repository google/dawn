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
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/sem/call.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/glsl/test_helper.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_Builtin = TestHelper;

enum class CallParamType {
    kF32,
    kU32,
    kBool,
    kF16,
};

struct BuiltinData {
    builtin::Function builtin;
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

const ast::CallExpression* GenerateCall(builtin::Function builtin,
                                        CallParamType type,
                                        ProgramBuilder* builder) {
    std::string name;
    utils::StringStream str;
    str << name << builtin;
    switch (builtin) {
        case builtin::Function::kAcos:
        case builtin::Function::kAsin:
        case builtin::Function::kAtan:
        case builtin::Function::kCeil:
        case builtin::Function::kCos:
        case builtin::Function::kCosh:
        case builtin::Function::kDpdx:
        case builtin::Function::kDpdxCoarse:
        case builtin::Function::kDpdxFine:
        case builtin::Function::kDpdy:
        case builtin::Function::kDpdyCoarse:
        case builtin::Function::kDpdyFine:
        case builtin::Function::kExp:
        case builtin::Function::kExp2:
        case builtin::Function::kFloor:
        case builtin::Function::kFract:
        case builtin::Function::kFwidth:
        case builtin::Function::kFwidthCoarse:
        case builtin::Function::kFwidthFine:
        case builtin::Function::kInverseSqrt:
        case builtin::Function::kLength:
        case builtin::Function::kLog:
        case builtin::Function::kLog2:
        case builtin::Function::kNormalize:
        case builtin::Function::kRound:
        case builtin::Function::kSin:
        case builtin::Function::kSinh:
        case builtin::Function::kSqrt:
        case builtin::Function::kTan:
        case builtin::Function::kTanh:
        case builtin::Function::kTrunc:
        case builtin::Function::kSign:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2");
            } else {
                return builder->Call(str.str(), "f2");
            }
        case builtin::Function::kLdexp:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "i2");
            } else {
                return builder->Call(str.str(), "f2", "i2");
            }
        case builtin::Function::kAtan2:
        case builtin::Function::kDot:
        case builtin::Function::kDistance:
        case builtin::Function::kPow:
        case builtin::Function::kReflect:
        case builtin::Function::kStep:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2");
            } else {
                return builder->Call(str.str(), "f2", "f2");
            }
        case builtin::Function::kCross:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h3", "h3");
            } else {
                return builder->Call(str.str(), "f3", "f3");
            }
        case builtin::Function::kFma:
        case builtin::Function::kMix:
        case builtin::Function::kFaceForward:
        case builtin::Function::kSmoothstep:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "h2");
            } else {
                return builder->Call(str.str(), "f2", "f2", "f2");
            }
        case builtin::Function::kAll:
        case builtin::Function::kAny:
            return builder->Call(str.str(), "b2");
        case builtin::Function::kAbs:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2");
            } else {
                return builder->Call(str.str(), "u2");
            }
        case builtin::Function::kCountOneBits:
        case builtin::Function::kReverseBits:
            return builder->Call(str.str(), "u2");
        case builtin::Function::kMax:
        case builtin::Function::kMin:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2", "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2");
            } else {
                return builder->Call(str.str(), "u2", "u2");
            }
        case builtin::Function::kClamp:
            if (type == CallParamType::kF32) {
                return builder->Call(str.str(), "f2", "f2", "f2");
            } else if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "h2");
            } else {
                return builder->Call(str.str(), "u2", "u2", "u2");
            }
        case builtin::Function::kSelect:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "h2", "h2", "b2");
            } else {
                return builder->Call(str.str(), "f2", "f2", "b2");
            }
        case builtin::Function::kDeterminant:
            if (type == CallParamType::kF16) {
                return builder->Call(str.str(), "hm2x2");
            } else {
                return builder->Call(str.str(), "m2x2");
            }
        case builtin::Function::kTranspose:
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
        Enable(builtin::Extension::kF16);

        GlobalVar("h2", ty.vec2<f16>(), builtin::AddressSpace::kPrivate);
        GlobalVar("h3", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);
        GlobalVar("hm2x2", ty.mat2x2<f16>(), builtin::AddressSpace::kPrivate);
        GlobalVar("hm3x2", ty.mat3x2<f16>(), builtin::AddressSpace::kPrivate);
    }

    GlobalVar("f2", ty.vec2<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("f3", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("u2", ty.vec2<u32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("i2", ty.vec2<i32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b2", ty.vec2<bool>(), builtin::AddressSpace::kPrivate);
    GlobalVar("m2x2", ty.mat2x2<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("m3x2", ty.mat3x2<f32>(), builtin::AddressSpace::kPrivate);

    auto* call = GenerateCall(param.builtin, param.type, this);
    ASSERT_NE(nullptr, call) << "Unhandled builtin";
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(Phony(), call),
         },
         utils::Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    auto* sem = program->Sem().Get<sem::Call>(call);
    ASSERT_NE(sem, nullptr);
    auto* target = sem->Target();
    ASSERT_NE(target, nullptr);
    auto* builtin = target->As<sem::Builtin>();
    ASSERT_NE(builtin, nullptr);

    EXPECT_EQ(gen.generate_builtin_name(builtin), param.glsl_name);
}
INSTANTIATE_TEST_SUITE_P(
    GlslGeneratorImplTest_Builtin,
    GlslBuiltinTest,
    testing::
        Values(/* Logical built-in */
               BuiltinData{builtin::Function::kAll, CallParamType::kBool, "all"},
               BuiltinData{builtin::Function::kAny, CallParamType::kBool, "any"},
               /* Float built-in */
               BuiltinData{builtin::Function::kAbs, CallParamType::kF32, "abs"},
               BuiltinData{builtin::Function::kAbs, CallParamType::kF16, "abs"},
               BuiltinData{builtin::Function::kAcos, CallParamType::kF32, "acos"},
               BuiltinData{builtin::Function::kAcos, CallParamType::kF16, "acos"},
               BuiltinData{builtin::Function::kAsin, CallParamType::kF32, "asin"},
               BuiltinData{builtin::Function::kAsin, CallParamType::kF16, "asin"},
               BuiltinData{builtin::Function::kAtan, CallParamType::kF32, "atan"},
               BuiltinData{builtin::Function::kAtan, CallParamType::kF16, "atan"},
               BuiltinData{builtin::Function::kAtan2, CallParamType::kF32, "atan"},
               BuiltinData{builtin::Function::kAtan2, CallParamType::kF16, "atan"},
               BuiltinData{builtin::Function::kCeil, CallParamType::kF32, "ceil"},
               BuiltinData{builtin::Function::kCeil, CallParamType::kF16, "ceil"},
               BuiltinData{builtin::Function::kClamp, CallParamType::kF32, "clamp"},
               BuiltinData{builtin::Function::kClamp, CallParamType::kF16, "clamp"},
               BuiltinData{builtin::Function::kCos, CallParamType::kF32, "cos"},
               BuiltinData{builtin::Function::kCos, CallParamType::kF16, "cos"},
               BuiltinData{builtin::Function::kCosh, CallParamType::kF32, "cosh"},
               BuiltinData{builtin::Function::kCosh, CallParamType::kF16, "cosh"},
               BuiltinData{builtin::Function::kCross, CallParamType::kF32, "cross"},
               BuiltinData{builtin::Function::kCross, CallParamType::kF16, "cross"},
               BuiltinData{builtin::Function::kDistance, CallParamType::kF32, "distance"},
               BuiltinData{builtin::Function::kDistance, CallParamType::kF16, "distance"},
               BuiltinData{builtin::Function::kExp, CallParamType::kF32, "exp"},
               BuiltinData{builtin::Function::kExp, CallParamType::kF16, "exp"},
               BuiltinData{builtin::Function::kExp2, CallParamType::kF32, "exp2"},
               BuiltinData{builtin::Function::kExp2, CallParamType::kF16, "exp2"},
               BuiltinData{builtin::Function::kFaceForward, CallParamType::kF32, "faceforward"},
               BuiltinData{builtin::Function::kFaceForward, CallParamType::kF16, "faceforward"},
               BuiltinData{builtin::Function::kFloor, CallParamType::kF32, "floor"},
               BuiltinData{builtin::Function::kFloor, CallParamType::kF16, "floor"},
               BuiltinData{builtin::Function::kFma, CallParamType::kF32, "fma"},
               BuiltinData{builtin::Function::kFma, CallParamType::kF16, "fma"},
               BuiltinData{builtin::Function::kFract, CallParamType::kF32, "fract"},
               BuiltinData{builtin::Function::kFract, CallParamType::kF16, "fract"},
               BuiltinData{builtin::Function::kInverseSqrt, CallParamType::kF32, "inversesqrt"},
               BuiltinData{builtin::Function::kInverseSqrt, CallParamType::kF16, "inversesqrt"},
               BuiltinData{builtin::Function::kLdexp, CallParamType::kF32, "ldexp"},
               BuiltinData{builtin::Function::kLdexp, CallParamType::kF16, "ldexp"},
               BuiltinData{builtin::Function::kLength, CallParamType::kF32, "length"},
               BuiltinData{builtin::Function::kLength, CallParamType::kF16, "length"},
               BuiltinData{builtin::Function::kLog, CallParamType::kF32, "log"},
               BuiltinData{builtin::Function::kLog, CallParamType::kF16, "log"},
               BuiltinData{builtin::Function::kLog2, CallParamType::kF32, "log2"},
               BuiltinData{builtin::Function::kLog2, CallParamType::kF16, "log2"},
               BuiltinData{builtin::Function::kMax, CallParamType::kF32, "max"},
               BuiltinData{builtin::Function::kMax, CallParamType::kF16, "max"},
               BuiltinData{builtin::Function::kMin, CallParamType::kF32, "min"},
               BuiltinData{builtin::Function::kMin, CallParamType::kF16, "min"},
               BuiltinData{builtin::Function::kMix, CallParamType::kF32, "mix"},
               BuiltinData{builtin::Function::kMix, CallParamType::kF16, "mix"},
               BuiltinData{builtin::Function::kNormalize, CallParamType::kF32, "normalize"},
               BuiltinData{builtin::Function::kNormalize, CallParamType::kF16, "normalize"},
               BuiltinData{builtin::Function::kPow, CallParamType::kF32, "pow"},
               BuiltinData{builtin::Function::kPow, CallParamType::kF16, "pow"},
               BuiltinData{builtin::Function::kReflect, CallParamType::kF32, "reflect"},
               BuiltinData{builtin::Function::kReflect, CallParamType::kF16, "reflect"},
               BuiltinData{builtin::Function::kSign, CallParamType::kF32, "sign"},
               BuiltinData{builtin::Function::kSign, CallParamType::kF16, "sign"},
               BuiltinData{builtin::Function::kSin, CallParamType::kF32, "sin"},
               BuiltinData{builtin::Function::kSin, CallParamType::kF16, "sin"},
               BuiltinData{builtin::Function::kSinh, CallParamType::kF32, "sinh"},
               BuiltinData{builtin::Function::kSinh, CallParamType::kF16, "sinh"},
               BuiltinData{builtin::Function::kSmoothstep, CallParamType::kF32, "smoothstep"},
               BuiltinData{builtin::Function::kSmoothstep, CallParamType::kF16, "smoothstep"},
               BuiltinData{builtin::Function::kSqrt, CallParamType::kF32, "sqrt"},
               BuiltinData{builtin::Function::kSqrt, CallParamType::kF16, "sqrt"},
               BuiltinData{builtin::Function::kStep, CallParamType::kF32, "step"},
               BuiltinData{builtin::Function::kStep, CallParamType::kF16, "step"},
               BuiltinData{builtin::Function::kTan, CallParamType::kF32, "tan"},
               BuiltinData{builtin::Function::kTan, CallParamType::kF16, "tan"},
               BuiltinData{builtin::Function::kTanh, CallParamType::kF32, "tanh"},
               BuiltinData{builtin::Function::kTanh, CallParamType::kF16, "tanh"},
               BuiltinData{builtin::Function::kTrunc, CallParamType::kF32, "trunc"},
               BuiltinData{builtin::Function::kTrunc, CallParamType::kF16, "trunc"},
               /* Integer built-in */
               BuiltinData{builtin::Function::kAbs, CallParamType::kU32, "abs"},
               BuiltinData{builtin::Function::kClamp, CallParamType::kU32, "clamp"},
               BuiltinData{builtin::Function::kCountOneBits, CallParamType::kU32, "bitCount"},
               BuiltinData{builtin::Function::kMax, CallParamType::kU32, "max"},
               BuiltinData{builtin::Function::kMin, CallParamType::kU32, "min"},
               BuiltinData{builtin::Function::kReverseBits, CallParamType::kU32, "bitfieldReverse"},
               BuiltinData{builtin::Function::kRound, CallParamType::kU32, "round"},
               /* Matrix built-in */
               BuiltinData{builtin::Function::kDeterminant, CallParamType::kF32, "determinant"},
               BuiltinData{builtin::Function::kDeterminant, CallParamType::kF16, "determinant"},
               BuiltinData{builtin::Function::kTranspose, CallParamType::kF32, "transpose"},
               BuiltinData{builtin::Function::kTranspose, CallParamType::kF16, "transpose"},
               /* Vector built-in */
               BuiltinData{builtin::Function::kDot, CallParamType::kF32, "dot"},
               BuiltinData{builtin::Function::kDot, CallParamType::kF16, "dot"},
               /* Derivate built-in */
               BuiltinData{builtin::Function::kDpdx, CallParamType::kF32, "dFdx"},
               BuiltinData{builtin::Function::kDpdxCoarse, CallParamType::kF32, "dFdx"},
               BuiltinData{builtin::Function::kDpdxFine, CallParamType::kF32, "dFdx"},
               BuiltinData{builtin::Function::kDpdy, CallParamType::kF32, "dFdy"},
               BuiltinData{builtin::Function::kDpdyCoarse, CallParamType::kF32, "dFdy"},
               BuiltinData{builtin::Function::kDpdyFine, CallParamType::kF32, "dFdy"},
               BuiltinData{builtin::Function::kFwidth, CallParamType::kF32, "fwidth"},
               BuiltinData{builtin::Function::kFwidthCoarse, CallParamType::kF32, "fwidth"},
               BuiltinData{builtin::Function::kFwidthFine, CallParamType::kF32, "fwidth"}));

TEST_F(GlslGeneratorImplTest_Builtin, Builtin_Call) {
    auto* call = Call("dot", "param1", "param2");

    GlobalVar("param1", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("param2", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    WrapInFunction(Decl(Var("r", call)));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    utils::StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "dot(param1, param2)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Select_Scalar) {
    GlobalVar("a", Expr(1_f), builtin::AddressSpace::kPrivate);
    GlobalVar("b", Expr(2_f), builtin::AddressSpace::kPrivate);
    auto* call = Call("select", "a", "b", true);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.increment_indent();
    utils::StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(true ? b : a)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Select_Vector) {
    GlobalVar("a", vec2<i32>(1_i, 2_i), builtin::AddressSpace::kPrivate);
    GlobalVar("b", vec2<i32>(3_i, 4_i), builtin::AddressSpace::kPrivate);
    auto* call = Call("select", "a", "b", vec2<bool>(true, false));
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.increment_indent();
    utils::StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_select(a, b, bvec2(true, false))");
}

TEST_F(GlslGeneratorImplTest_Builtin, FMA_f32) {
    auto* call = Call("fma", "a", "b", "c");

    GlobalVar("a", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    WrapInFunction(Decl(Var("r", call)));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    utils::StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "((a) * (b) + (c))");
}

TEST_F(GlslGeneratorImplTest_Builtin, FMA_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("a", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);

    auto* call = Call("fma", "a", "b", "c");
    WrapInFunction(Decl(Var("r", call)));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    utils::StringStream out;
    gen.EmitExpression(out, call);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "((a) * (b) + (c))");
}

TEST_F(GlslGeneratorImplTest_Builtin, Runtime_Modf_Scalar_f32) {
    WrapInFunction(Decl(Let("f", Expr(1.5_f))),  //
                   Decl(Let("v", Call("modf", "f"))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Runtime_Modf_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Decl(Let("f", Expr(1.5_h))),  //
                   Decl(Let("v", Call("modf", "f"))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Runtime_Modf_Vector_f32) {
    WrapInFunction(Decl(Let("f", vec3<f32>(1.5_f, 2.5_f, 3.5_f))),  //
                   Decl(Let("v", Call("modf", "f"))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Runtime_Modf_Vector_f16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Decl(Let("f", vec3<f16>(1.5_h, 2.5_h, 3.5_h))),  //
                   Decl(Let("v", Call("modf", "f"))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Const_Modf_Scalar_f32) {
    WrapInFunction(Decl(Let("v", Call("modf", 1.5_f))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Const_Modf_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("modf", 1.5_h))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Const_Modf_Vector_f32) {
    WrapInFunction(Decl(Let("v", Call("modf", vec3<f32>(1.5_f, 2.5_f, 3.5_f)))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Const_Modf_Vector_f16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("modf", vec3<f16>(1.5_h, 2.5_h, 3.5_h)))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Runtime_Frexp_Scalar_f32) {
    WrapInFunction(Var("f", Expr(1_f)),  //
                   Var("v", Call("frexp", "f")));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Runtime_Frexp_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Var("f", Expr(1_h)),  //
                   Var("v", Call("frexp", "f")));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Runtime_Frexp_Vector_f32) {
    WrapInFunction(Var("f", Expr(vec3<f32>())),  //
                   Var("v", Call("frexp", "f")));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Runtime_Frexp_Vector_f16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Var("f", Expr(vec3<f16>())),  //
                   Var("v", Call("frexp", "f")));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Const_Frexp_Scalar_f32) {
    WrapInFunction(Decl(Let("v", Call("frexp", 1_f))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Const_Frexp_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("frexp", 1_h))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Const_Frexp_Vector_f32) {
    WrapInFunction(Decl(Let("v", Call("frexp", vec3<f32>()))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Const_Frexp_Vector_f16) {
    Enable(builtin::Extension::kF16);

    WrapInFunction(Decl(Let("v", Call("frexp", vec3<f16>()))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Degrees_Scalar_f32) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Degrees_Vector_f32) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Degrees_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* val = Var("val", ty.f16());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Degrees_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto* val = Var("val", ty.vec3<f16>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Radians_Scalar_f32) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Radians_Vector_f32) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Radians_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* val = Var("val", ty.f16());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, Radians_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto* val = Var("val", ty.vec3<f16>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
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

TEST_F(GlslGeneratorImplTest_Builtin, ExtractBits) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    WrapInFunction(v, offset, count, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, InsertBits) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* n = Var("n", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    WrapInFunction(v, n, offset, count, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, Pack4x8Snorm) {
    auto* call = Call("pack4x8snorm", "p1");
    GlobalVar("p1", ty.vec4<f32>(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec4 p1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packSnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Pack4x8Unorm) {
    auto* call = Call("pack4x8unorm", "p1");
    GlobalVar("p1", ty.vec4<f32>(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec4 p1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packUnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Pack2x16Snorm) {
    auto* call = Call("pack2x16snorm", "p1");
    GlobalVar("p1", ty.vec2<f32>(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packSnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Pack2x16Unorm) {
    auto* call = Call("pack2x16unorm", "p1");
    GlobalVar("p1", ty.vec2<f32>(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packUnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Pack2x16Float) {
    auto* call = Call("pack2x16float", "p1");
    GlobalVar("p1", ty.vec2<f32>(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  uint r = packHalf2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack4x8Snorm) {
    auto* call = Call("unpack4x8snorm", "p1");
    GlobalVar("p1", ty.u32(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec4 r = unpackSnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack4x8Unorm) {
    auto* call = Call("unpack4x8unorm", "p1");
    GlobalVar("p1", ty.u32(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec4 r = unpackUnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack2x16Snorm) {
    auto* call = Call("unpack2x16snorm", "p1");
    GlobalVar("p1", ty.u32(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec2 r = unpackSnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack2x16Unorm) {
    auto* call = Call("unpack2x16unorm", "p1");
    GlobalVar("p1", ty.u32(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec2 r = unpackUnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack2x16Float) {
    auto* call = Call("unpack2x16float", "p1");
    GlobalVar("p1", ty.u32(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", call)));
    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec2 r = unpackHalf2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, StorageBarrier) {
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("storageBarrier")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  { barrier(); memoryBarrierBuffer(); };
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, WorkgroupBarrier) {
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("workgroupBarrier")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, DotI32) {
    GlobalVar("v", ty.vec3<i32>(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", Call("dot", "v", "v"))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, DotU32) {
    GlobalVar("v", ty.vec3<u32>(), builtin::AddressSpace::kPrivate);
    WrapInFunction(Decl(Var("r", Call("dot", "v", "v"))));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, QuantizeToF16_Scalar) {
    GlobalVar("v", Expr(2_f), builtin::AddressSpace::kPrivate);
    WrapInFunction(Call("quantizeToF16", "v"));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, QuantizeToF16_Vec2) {
    GlobalVar("v", vec2<f32>(2_f), builtin::AddressSpace::kPrivate);
    WrapInFunction(Call("quantizeToF16", "v"));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, QuantizeToF16_Vec3) {
    GlobalVar("v", vec3<f32>(2_f), builtin::AddressSpace::kPrivate);
    WrapInFunction(Call("quantizeToF16", "v"));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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

TEST_F(GlslGeneratorImplTest_Builtin, QuantizeToF16_Vec4) {
    GlobalVar("v", vec4<f32>(2_f), builtin::AddressSpace::kPrivate);
    WrapInFunction(Call("quantizeToF16", "v"));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

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
}  // namespace tint::writer::glsl
