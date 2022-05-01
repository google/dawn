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
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/sem/call.h"
#include "src/tint/writer/hlsl/test_helper.h"

namespace tint::writer::hlsl {
namespace {

using BuiltinType = sem::BuiltinType;

using ::testing::HasSubstr;

using HlslGeneratorImplTest_Builtin = TestHelper;

enum class ParamType {
    kF32,
    kU32,
    kBool,
};

struct BuiltinData {
    BuiltinType builtin;
    ParamType type;
    const char* hlsl_name;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.hlsl_name;
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
using HlslBuiltinTest = TestParamHelper<BuiltinData>;
TEST_P(HlslBuiltinTest, Emit) {
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

    EXPECT_EQ(gen.generate_builtin_name(builtin), param.hlsl_name);
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Builtin,
    HlslBuiltinTest,
    testing::Values(BuiltinData{BuiltinType::kAbs, ParamType::kF32, "abs"},
                    BuiltinData{BuiltinType::kAbs, ParamType::kU32, "abs"},
                    BuiltinData{BuiltinType::kAcos, ParamType::kF32, "acos"},
                    BuiltinData{BuiltinType::kAll, ParamType::kBool, "all"},
                    BuiltinData{BuiltinType::kAny, ParamType::kBool, "any"},
                    BuiltinData{BuiltinType::kAsin, ParamType::kF32, "asin"},
                    BuiltinData{BuiltinType::kAtan, ParamType::kF32, "atan"},
                    BuiltinData{BuiltinType::kAtan2, ParamType::kF32, "atan2"},
                    BuiltinData{BuiltinType::kCeil, ParamType::kF32, "ceil"},
                    BuiltinData{BuiltinType::kClamp, ParamType::kF32, "clamp"},
                    BuiltinData{BuiltinType::kClamp, ParamType::kU32, "clamp"},
                    BuiltinData{BuiltinType::kCos, ParamType::kF32, "cos"},
                    BuiltinData{BuiltinType::kCosh, ParamType::kF32, "cosh"},
                    BuiltinData{BuiltinType::kCountOneBits, ParamType::kU32, "countbits"},
                    BuiltinData{BuiltinType::kCross, ParamType::kF32, "cross"},
                    BuiltinData{BuiltinType::kDeterminant, ParamType::kF32, "determinant"},
                    BuiltinData{BuiltinType::kDistance, ParamType::kF32, "distance"},
                    BuiltinData{BuiltinType::kDot, ParamType::kF32, "dot"},
                    BuiltinData{BuiltinType::kDpdx, ParamType::kF32, "ddx"},
                    BuiltinData{BuiltinType::kDpdxCoarse, ParamType::kF32, "ddx_coarse"},
                    BuiltinData{BuiltinType::kDpdxFine, ParamType::kF32, "ddx_fine"},
                    BuiltinData{BuiltinType::kDpdy, ParamType::kF32, "ddy"},
                    BuiltinData{BuiltinType::kDpdyCoarse, ParamType::kF32, "ddy_coarse"},
                    BuiltinData{BuiltinType::kDpdyFine, ParamType::kF32, "ddy_fine"},
                    BuiltinData{BuiltinType::kExp, ParamType::kF32, "exp"},
                    BuiltinData{BuiltinType::kExp2, ParamType::kF32, "exp2"},
                    BuiltinData{BuiltinType::kFaceForward, ParamType::kF32, "faceforward"},
                    BuiltinData{BuiltinType::kFloor, ParamType::kF32, "floor"},
                    BuiltinData{BuiltinType::kFma, ParamType::kF32, "mad"},
                    BuiltinData{BuiltinType::kFract, ParamType::kF32, "frac"},
                    BuiltinData{BuiltinType::kFwidth, ParamType::kF32, "fwidth"},
                    BuiltinData{BuiltinType::kFwidthCoarse, ParamType::kF32, "fwidth"},
                    BuiltinData{BuiltinType::kFwidthFine, ParamType::kF32, "fwidth"},
                    BuiltinData{BuiltinType::kInverseSqrt, ParamType::kF32, "rsqrt"},
                    BuiltinData{BuiltinType::kLdexp, ParamType::kF32, "ldexp"},
                    BuiltinData{BuiltinType::kLength, ParamType::kF32, "length"},
                    BuiltinData{BuiltinType::kLog, ParamType::kF32, "log"},
                    BuiltinData{BuiltinType::kLog2, ParamType::kF32, "log2"},
                    BuiltinData{BuiltinType::kMax, ParamType::kF32, "max"},
                    BuiltinData{BuiltinType::kMax, ParamType::kU32, "max"},
                    BuiltinData{BuiltinType::kMin, ParamType::kF32, "min"},
                    BuiltinData{BuiltinType::kMin, ParamType::kU32, "min"},
                    BuiltinData{BuiltinType::kMix, ParamType::kF32, "lerp"},
                    BuiltinData{BuiltinType::kNormalize, ParamType::kF32, "normalize"},
                    BuiltinData{BuiltinType::kPow, ParamType::kF32, "pow"},
                    BuiltinData{BuiltinType::kReflect, ParamType::kF32, "reflect"},
                    BuiltinData{BuiltinType::kReverseBits, ParamType::kU32, "reversebits"},
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

TEST_F(HlslGeneratorImplTest_Builtin, Builtin_Call) {
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

TEST_F(HlslGeneratorImplTest_Builtin, Select_Scalar) {
    auto* call = Call("select", 1.0f, 2.0f, true);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    gen.increment_indent();
    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "(true ? 2.0f : 1.0f)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Select_Vector) {
    auto* call = Call("select", vec2<i32>(1, 2), vec2<i32>(3, 4), vec2<bool>(true, false));
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    gen.increment_indent();
    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "(bool2(true, false) ? int2(3, 4) : int2(1, 2))");
}

TEST_F(HlslGeneratorImplTest_Builtin, Modf_Scalar) {
    auto* call = Call("modf", 1.0f);
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(struct modf_result {
  float fract;
  float whole;
};
modf_result tint_modf(float param_0) {
  float whole;
  float fract = modf(param_0, whole);
  modf_result result = {fract, whole};
  return result;
}

[numthreads(1, 1, 1)]
void test_function() {
  tint_modf(1.0f);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Modf_Vector) {
    auto* call = Call("modf", vec3<f32>());
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(struct modf_result_vec3 {
  float3 fract;
  float3 whole;
};
modf_result_vec3 tint_modf(float3 param_0) {
  float3 whole;
  float3 fract = modf(param_0, whole);
  modf_result_vec3 result = {fract, whole};
  return result;
}

[numthreads(1, 1, 1)]
void test_function() {
  tint_modf(float3(0.0f, 0.0f, 0.0f));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Frexp_Scalar_i32) {
    auto* call = Call("frexp", 1.0f);
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(struct frexp_result {
  float sig;
  int exp;
};
frexp_result tint_frexp(float param_0) {
  float exp;
  float sig = frexp(param_0, exp);
  frexp_result result = {sig, int(exp)};
  return result;
}

[numthreads(1, 1, 1)]
void test_function() {
  tint_frexp(1.0f);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Frexp_Vector_i32) {
    auto* call = Call("frexp", vec3<f32>());
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(struct frexp_result_vec3 {
  float3 sig;
  int3 exp;
};
frexp_result_vec3 tint_frexp(float3 param_0) {
  float3 exp;
  float3 sig = frexp(param_0, exp);
  frexp_result_vec3 result = {sig, int3(exp)};
  return result;
}

[numthreads(1, 1, 1)]
void test_function() {
  tint_frexp(float3(0.0f, 0.0f, 0.0f));
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Degrees_Scalar) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float tint_degrees(float param_0) {
  return param_0 * 57.295779513082322865;
}

[numthreads(1, 1, 1)]
void test_function() {
  float val = 0.0f;
  const float tint_symbol = tint_degrees(val);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Degrees_Vector) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float3 tint_degrees(float3 param_0) {
  return param_0 * 57.295779513082322865;
}

[numthreads(1, 1, 1)]
void test_function() {
  float3 val = float3(0.0f, 0.0f, 0.0f);
  const float3 tint_symbol = tint_degrees(val);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Radians_Scalar) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float tint_radians(float param_0) {
  return param_0 * 0.017453292519943295474;
}

[numthreads(1, 1, 1)]
void test_function() {
  float val = 0.0f;
  const float tint_symbol = tint_radians(val);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Radians_Vector) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float3 tint_radians(float3 param_0) {
  return param_0 * 0.017453292519943295474;
}

[numthreads(1, 1, 1)]
void test_function() {
  float3 val = float3(0.0f, 0.0f, 0.0f);
  const float3 tint_symbol = tint_radians(val);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Pack4x8Snorm) {
    auto* call = Call("pack4x8snorm", "p1");
    Global("p1", ty.vec4<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(uint tint_pack4x8snorm(float4 param_0) {
  int4 i = int4(round(clamp(param_0, -1.0, 1.0) * 127.0)) & 0xff;
  return asuint(i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

static float4 p1 = float4(0.0f, 0.0f, 0.0f, 0.0f);

[numthreads(1, 1, 1)]
void test_function() {
  tint_pack4x8snorm(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Pack4x8Unorm) {
    auto* call = Call("pack4x8unorm", "p1");
    Global("p1", ty.vec4<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(uint tint_pack4x8unorm(float4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

static float4 p1 = float4(0.0f, 0.0f, 0.0f, 0.0f);

[numthreads(1, 1, 1)]
void test_function() {
  tint_pack4x8unorm(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Pack2x16Snorm) {
    auto* call = Call("pack2x16snorm", "p1");
    Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(uint tint_pack2x16snorm(float2 param_0) {
  int2 i = int2(round(clamp(param_0, -1.0, 1.0) * 32767.0)) & 0xffff;
  return asuint(i.x | i.y << 16);
}

static float2 p1 = float2(0.0f, 0.0f);

[numthreads(1, 1, 1)]
void test_function() {
  tint_pack2x16snorm(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Pack2x16Unorm) {
    auto* call = Call("pack2x16unorm", "p1");
    Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(uint tint_pack2x16unorm(float2 param_0) {
  uint2 i = uint2(round(clamp(param_0, 0.0, 1.0) * 65535.0));
  return (i.x | i.y << 16);
}

static float2 p1 = float2(0.0f, 0.0f);

[numthreads(1, 1, 1)]
void test_function() {
  tint_pack2x16unorm(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Pack2x16Float) {
    auto* call = Call("pack2x16float", "p1");
    Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(uint tint_pack2x16float(float2 param_0) {
  uint2 i = f32tof16(param_0);
  return i.x | (i.y << 16);
}

static float2 p1 = float2(0.0f, 0.0f);

[numthreads(1, 1, 1)]
void test_function() {
  tint_pack2x16float(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Unpack4x8Snorm) {
    auto* call = Call("unpack4x8snorm", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float4 tint_unpack4x8snorm(uint param_0) {
  int j = int(param_0);
  int4 i = int4(j << 24, j << 16, j << 8, j) >> 24;
  return clamp(float4(i) / 127.0, -1.0, 1.0);
}

static uint p1 = 0u;

[numthreads(1, 1, 1)]
void test_function() {
  tint_unpack4x8snorm(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Unpack4x8Unorm) {
    auto* call = Call("unpack4x8unorm", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}

static uint p1 = 0u;

[numthreads(1, 1, 1)]
void test_function() {
  tint_unpack4x8unorm(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Unpack2x16Snorm) {
    auto* call = Call("unpack2x16snorm", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}

static uint p1 = 0u;

[numthreads(1, 1, 1)]
void test_function() {
  tint_unpack2x16snorm(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Unpack2x16Unorm) {
    auto* call = Call("unpack2x16unorm", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float2 tint_unpack2x16unorm(uint param_0) {
  uint j = param_0;
  uint2 i = uint2(j & 0xffff, j >> 16);
  return float2(i) / 65535.0;
}

static uint p1 = 0u;

[numthreads(1, 1, 1)]
void test_function() {
  tint_unpack2x16unorm(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, Unpack2x16Float) {
    auto* call = Call("unpack2x16float", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(float2 tint_unpack2x16float(uint param_0) {
  uint i = param_0;
  return f16tof32(uint2(i & 0xffff, i >> 16));
}

static uint p1 = 0u;

[numthreads(1, 1, 1)]
void test_function() {
  tint_unpack2x16float(p1);
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, StorageBarrier) {
    Func("main", {}, ty.void_(), {CallStmt(Call("storageBarrier"))},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"([numthreads(1, 1, 1)]
void main() {
  DeviceMemoryBarrierWithGroupSync();
  return;
}
)");
}

TEST_F(HlslGeneratorImplTest_Builtin, WorkgroupBarrier) {
    Func("main", {}, ty.void_(), {CallStmt(Call("workgroupBarrier"))},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"([numthreads(1, 1, 1)]
void main() {
  GroupMemoryBarrierWithGroupSync();
  return;
}
)");
}

}  // namespace
}  // namespace tint::writer::hlsl
