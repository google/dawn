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
#include "src/tint/writer/glsl/test_helper.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using BuiltinType = sem::BuiltinType;

using GlslGeneratorImplTest_Builtin = TestHelper;

enum class ParamType {
    kF32,
    kU32,
    kBool,
};

struct BuiltinData {
    BuiltinType builtin;
    ParamType type;
    const char* glsl_name;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.glsl_name;
    switch (data.type) {
        case ParamType::kF32:
            out << "f32";
            break;
        case ParamType::kU32:
            out << "u32";
            break;
        case ParamType::kBool:
            out << "bool";
            break;
    }
    out << ">";
    return out;
}

const ast::CallExpression* GenerateCall(BuiltinType builtin,
                                        ParamType type,
                                        ProgramBuilder* builder) {
    std::string name;
    std::ostringstream str(name);
    str << builtin;
    switch (builtin) {
        case BuiltinType::kAcos:
        case BuiltinType::kAsin:
        case BuiltinType::kAtan:
        case BuiltinType::kCeil:
        case BuiltinType::kCos:
        case BuiltinType::kCosh:
        case BuiltinType::kDpdx:
        case BuiltinType::kDpdxCoarse:
        case BuiltinType::kDpdxFine:
        case BuiltinType::kDpdy:
        case BuiltinType::kDpdyCoarse:
        case BuiltinType::kDpdyFine:
        case BuiltinType::kExp:
        case BuiltinType::kExp2:
        case BuiltinType::kFloor:
        case BuiltinType::kFract:
        case BuiltinType::kFwidth:
        case BuiltinType::kFwidthCoarse:
        case BuiltinType::kFwidthFine:
        case BuiltinType::kInverseSqrt:
        case BuiltinType::kLength:
        case BuiltinType::kLog:
        case BuiltinType::kLog2:
        case BuiltinType::kNormalize:
        case BuiltinType::kRound:
        case BuiltinType::kSin:
        case BuiltinType::kSinh:
        case BuiltinType::kSqrt:
        case BuiltinType::kTan:
        case BuiltinType::kTanh:
        case BuiltinType::kTrunc:
        case BuiltinType::kSign:
            return builder->Call(str.str(), "f2");
        case BuiltinType::kLdexp:
            return builder->Call(str.str(), "f2", "i2");
        case BuiltinType::kAtan2:
        case BuiltinType::kDot:
        case BuiltinType::kDistance:
        case BuiltinType::kPow:
        case BuiltinType::kReflect:
        case BuiltinType::kStep:
            return builder->Call(str.str(), "f2", "f2");
        case BuiltinType::kCross:
            return builder->Call(str.str(), "f3", "f3");
        case BuiltinType::kFma:
        case BuiltinType::kMix:
        case BuiltinType::kFaceForward:
        case BuiltinType::kSmoothstep:
        case BuiltinType::kSmoothStep:
            return builder->Call(str.str(), "f2", "f2", "f2");
        case BuiltinType::kAll:
        case BuiltinType::kAny:
            return builder->Call(str.str(), "b2");
        case BuiltinType::kAbs:
            if (type == ParamType::kF32) {
                return builder->Call(str.str(), "f2");
            } else {
                return builder->Call(str.str(), "u2");
            }
        case BuiltinType::kCountOneBits:
        case BuiltinType::kReverseBits:
            return builder->Call(str.str(), "u2");
        case BuiltinType::kMax:
        case BuiltinType::kMin:
            if (type == ParamType::kF32) {
                return builder->Call(str.str(), "f2", "f2");
            } else {
                return builder->Call(str.str(), "u2", "u2");
            }
        case BuiltinType::kClamp:
            if (type == ParamType::kF32) {
                return builder->Call(str.str(), "f2", "f2", "f2");
            } else {
                return builder->Call(str.str(), "u2", "u2", "u2");
            }
        case BuiltinType::kSelect:
            return builder->Call(str.str(), "f2", "f2", "b2");
        case BuiltinType::kDeterminant:
            return builder->Call(str.str(), "m2x2");
        case BuiltinType::kTranspose:
            return builder->Call(str.str(), "m3x2");
        default:
            break;
    }
    return nullptr;
}
using GlslBuiltinTest = TestParamHelper<BuiltinData>;
TEST_P(GlslBuiltinTest, Emit) {
    auto param = GetParam();

    Global("f2", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    Global("f3", ty.vec3<f32>(), ast::StorageClass::kPrivate);
    Global("u2", ty.vec2<u32>(), ast::StorageClass::kPrivate);
    Global("i2", ty.vec2<i32>(), ast::StorageClass::kPrivate);
    Global("b2", ty.vec2<bool>(), ast::StorageClass::kPrivate);
    Global("m2x2", ty.mat2x2<f32>(), ast::StorageClass::kPrivate);
    Global("m3x2", ty.mat3x2<f32>(), ast::StorageClass::kPrivate);

    auto* call = GenerateCall(param.builtin, param.type, this);
    ASSERT_NE(nullptr, call) << "Unhandled builtin";
    Func("func", {}, ty.void_(), {CallStmt(call)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    auto* sem = program->Sem().Get(call);
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
    testing::Values(BuiltinData{BuiltinType::kAbs, ParamType::kF32, "abs"},
                    BuiltinData{BuiltinType::kAbs, ParamType::kU32, "abs"},
                    BuiltinData{BuiltinType::kAcos, ParamType::kF32, "acos"},
                    BuiltinData{BuiltinType::kAll, ParamType::kBool, "all"},
                    BuiltinData{BuiltinType::kAny, ParamType::kBool, "any"},
                    BuiltinData{BuiltinType::kAsin, ParamType::kF32, "asin"},
                    BuiltinData{BuiltinType::kAtan, ParamType::kF32, "atan"},
                    BuiltinData{BuiltinType::kAtan2, ParamType::kF32, "atan"},
                    BuiltinData{BuiltinType::kCeil, ParamType::kF32, "ceil"},
                    BuiltinData{BuiltinType::kClamp, ParamType::kF32, "clamp"},
                    BuiltinData{BuiltinType::kClamp, ParamType::kU32, "clamp"},
                    BuiltinData{BuiltinType::kCos, ParamType::kF32, "cos"},
                    BuiltinData{BuiltinType::kCosh, ParamType::kF32, "cosh"},
                    BuiltinData{BuiltinType::kCountOneBits, ParamType::kU32, "bitCount"},
                    BuiltinData{BuiltinType::kCross, ParamType::kF32, "cross"},
                    BuiltinData{BuiltinType::kDeterminant, ParamType::kF32, "determinant"},
                    BuiltinData{BuiltinType::kDistance, ParamType::kF32, "distance"},
                    BuiltinData{BuiltinType::kDot, ParamType::kF32, "dot"},
                    BuiltinData{BuiltinType::kDpdx, ParamType::kF32, "dFdx"},
                    BuiltinData{BuiltinType::kDpdxCoarse, ParamType::kF32, "dFdx"},
                    BuiltinData{BuiltinType::kDpdxFine, ParamType::kF32, "dFdx"},
                    BuiltinData{BuiltinType::kDpdy, ParamType::kF32, "dFdy"},
                    BuiltinData{BuiltinType::kDpdyCoarse, ParamType::kF32, "dFdy"},
                    BuiltinData{BuiltinType::kDpdyFine, ParamType::kF32, "dFdy"},
                    BuiltinData{BuiltinType::kExp, ParamType::kF32, "exp"},
                    BuiltinData{BuiltinType::kExp2, ParamType::kF32, "exp2"},
                    BuiltinData{BuiltinType::kFaceForward, ParamType::kF32, "faceforward"},
                    BuiltinData{BuiltinType::kFloor, ParamType::kF32, "floor"},
                    BuiltinData{BuiltinType::kFma, ParamType::kF32, "fma"},
                    BuiltinData{BuiltinType::kFract, ParamType::kF32, "fract"},
                    BuiltinData{BuiltinType::kFwidth, ParamType::kF32, "fwidth"},
                    BuiltinData{BuiltinType::kFwidthCoarse, ParamType::kF32, "fwidth"},
                    BuiltinData{BuiltinType::kFwidthFine, ParamType::kF32, "fwidth"},
                    BuiltinData{BuiltinType::kInverseSqrt, ParamType::kF32, "inversesqrt"},
                    BuiltinData{BuiltinType::kLdexp, ParamType::kF32, "ldexp"},
                    BuiltinData{BuiltinType::kLength, ParamType::kF32, "length"},
                    BuiltinData{BuiltinType::kLog, ParamType::kF32, "log"},
                    BuiltinData{BuiltinType::kLog2, ParamType::kF32, "log2"},
                    BuiltinData{BuiltinType::kMax, ParamType::kF32, "max"},
                    BuiltinData{BuiltinType::kMax, ParamType::kU32, "max"},
                    BuiltinData{BuiltinType::kMin, ParamType::kF32, "min"},
                    BuiltinData{BuiltinType::kMin, ParamType::kU32, "min"},
                    BuiltinData{BuiltinType::kMix, ParamType::kF32, "mix"},
                    BuiltinData{BuiltinType::kNormalize, ParamType::kF32, "normalize"},
                    BuiltinData{BuiltinType::kPow, ParamType::kF32, "pow"},
                    BuiltinData{BuiltinType::kReflect, ParamType::kF32, "reflect"},
                    BuiltinData{BuiltinType::kReverseBits, ParamType::kU32, "bitfieldReverse"},
                    BuiltinData{BuiltinType::kRound, ParamType::kU32, "round"},
                    BuiltinData{BuiltinType::kSign, ParamType::kF32, "sign"},
                    BuiltinData{BuiltinType::kSin, ParamType::kF32, "sin"},
                    BuiltinData{BuiltinType::kSinh, ParamType::kF32, "sinh"},
                    BuiltinData{BuiltinType::kSmoothstep, ParamType::kF32, "smoothstep"},
                    BuiltinData{BuiltinType::kSmoothStep, ParamType::kF32, "smoothstep"},
                    BuiltinData{BuiltinType::kSqrt, ParamType::kF32, "sqrt"},
                    BuiltinData{BuiltinType::kStep, ParamType::kF32, "step"},
                    BuiltinData{BuiltinType::kTan, ParamType::kF32, "tan"},
                    BuiltinData{BuiltinType::kTanh, ParamType::kF32, "tanh"},
                    BuiltinData{BuiltinType::kTranspose, ParamType::kF32, "transpose"},
                    BuiltinData{BuiltinType::kTrunc, ParamType::kF32, "trunc"}));

TEST_F(GlslGeneratorImplTest_Builtin, Builtin_Call) {
    auto* call = Call("dot", "param1", "param2");

    Global("param1", ty.vec3<f32>(), ast::StorageClass::kPrivate);
    Global("param2", ty.vec3<f32>(), ast::StorageClass::kPrivate);

    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "dot(param1, param2)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Select_Scalar) {
    auto* call = Call("select", 1.0f, 2.0f, true);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    gen.increment_indent();
    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "(true ? 2.0f : 1.0f)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Select_Vector) {
    auto* call = Call("select", vec2<i32>(1_i, 2_i), vec2<i32>(3_i, 4_i), vec2<bool>(true, false));
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    gen.increment_indent();
    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "mix(ivec2(1, 2), ivec2(3, 4), bvec2(true, false))");
}

TEST_F(GlslGeneratorImplTest_Builtin, Modf_Scalar) {
    auto* call = Call("modf", 1.0f);
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

struct modf_result {
  float fract;
  float whole;
};

modf_result tint_modf(float param_0) {
  modf_result result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void test_function() {
  tint_modf(1.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Modf_Vector) {
    auto* call = Call("modf", vec3<f32>());
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

struct modf_result_vec3 {
  vec3 fract;
  vec3 whole;
};

modf_result_vec3 tint_modf(vec3 param_0) {
  modf_result_vec3 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void test_function() {
  tint_modf(vec3(0.0f, 0.0f, 0.0f));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Frexp_Scalar_i32) {
    auto* call = Call("frexp", 1.0f);
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(R"(
  float sig;
  int exp;
};

frexp_result tint_frexp(float param_0) {
  frexp_result result;
  result.sig = frexp(param_0, result.exp);
  return result;
}


void test_function() {
  tint_frexp(1.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
)"));
}

TEST_F(GlslGeneratorImplTest_Builtin, Frexp_Vector_i32) {
    auto* call = Call("frexp", vec3<f32>());
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(R"(

struct frexp_result_vec3 {
  vec3 sig;
  ivec3 exp;
};

frexp_result_vec3 tint_frexp(vec3 param_0) {
  frexp_result_vec3 result;
  result.sig = frexp(param_0, result.exp);
  return result;
}


void test_function() {
  tint_frexp(vec3(0.0f, 0.0f, 0.0f));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
)"));
}

TEST_F(GlslGeneratorImplTest_Builtin, Degrees_Scalar) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

float tint_degrees(float param_0) {
  return param_0 * 57.295779513082322865;
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

TEST_F(GlslGeneratorImplTest_Builtin, Degrees_Vector) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec3 tint_degrees(vec3 param_0) {
  return param_0 * 57.295779513082322865;
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

TEST_F(GlslGeneratorImplTest_Builtin, Radians_Scalar) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

float tint_radians(float param_0) {
  return param_0 * 0.017453292519943295474;
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

TEST_F(GlslGeneratorImplTest_Builtin, Radians_Vector) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec3 tint_radians(vec3 param_0) {
  return param_0 * 0.017453292519943295474;
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

TEST_F(GlslGeneratorImplTest_Builtin, ExtractBits) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    WrapInFunction(v, offset, count, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
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

    ASSERT_TRUE(gen.Generate()) << gen.error();
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
    Global("p1", ty.vec4<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec4 p1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  packSnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Pack4x8Unorm) {
    auto* call = Call("pack4x8unorm", "p1");
    Global("p1", ty.vec4<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec4 p1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  packUnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Pack2x16Snorm) {
    auto* call = Call("pack2x16snorm", "p1");
    Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  packSnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Pack2x16Unorm) {
    auto* call = Call("pack2x16unorm", "p1");
    Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  packUnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Pack2x16Float) {
    auto* call = Call("pack2x16float", "p1");
    Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec2 p1 = vec2(0.0f, 0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  packHalf2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack4x8Snorm) {
    auto* call = Call("unpack4x8snorm", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  unpackSnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack4x8Unorm) {
    auto* call = Call("unpack4x8unorm", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  unpackUnorm4x8(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack2x16Snorm) {
    auto* call = Call("unpack2x16snorm", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  unpackSnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack2x16Unorm) {
    auto* call = Call("unpack2x16unorm", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  unpackUnorm2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, Unpack2x16Float) {
    auto* call = Call("unpack2x16float", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint p1 = 0u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  unpackHalf2x16(p1);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, StorageBarrier) {
    Func("main", {}, ty.void_(), {CallStmt(Call("storageBarrier"))},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  { barrier(); memoryBarrierBuffer(); };
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, WorkgroupBarrier) {
    Func("main", {}, ty.void_(), {CallStmt(Call("workgroupBarrier"))},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, DotI32) {
    Global("v", ty.vec3<i32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(Call("dot", "v", "v")));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

int tint_int_dot(ivec3 a, ivec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

ivec3 v = ivec3(0, 0, 0);
void test_function() {
  tint_int_dot(v, v);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  test_function();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Builtin, FMA) {
    auto* call = Call("fma", "a", "b", "c");

    Global("a", ty.vec3<f32>(), ast::StorageClass::kPrivate);
    Global("b", ty.vec3<f32>(), ast::StorageClass::kPrivate);
    Global("c", ty.vec3<f32>(), ast::StorageClass::kPrivate);

    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = Build();

    gen.increment_indent();
    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "((a) * (b) + (c))");
}

TEST_F(GlslGeneratorImplTest_Builtin, DotU32) {
    Global("v", ty.vec3<u32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(Call("dot", "v", "v")));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#version 310 es

uint tint_int_dot(uvec3 a, uvec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

uvec3 v = uvec3(0u, 0u, 0u);
void test_function() {
  tint_int_dot(v, v);
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
