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

#include "src/tint/ast/call_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/writer/msl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::msl {
namespace {

using BuiltinType = sem::BuiltinType;

using MslGeneratorImplTest = TestHelper;

enum class ParamType {
    kF32,
    kU32,
    kBool,
};

struct BuiltinData {
    BuiltinType builtin;
    ParamType type;
    const char* msl_name;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.msl_name << "<";
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
        case BuiltinType::kStorageBarrier:
            return builder->Call(str.str());
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
        case BuiltinType::kCountLeadingZeros:
        case BuiltinType::kCountOneBits:
        case BuiltinType::kCountTrailingZeros:
        case BuiltinType::kReverseBits:
            return builder->Call(str.str(), "u2");
        case BuiltinType::kExtractBits:
            return builder->Call(str.str(), "u2", "u1", "u1");
        case BuiltinType::kInsertBits:
            return builder->Call(str.str(), "u2", "u2", "u1", "u1");
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
        case BuiltinType::kPack2x16snorm:
        case BuiltinType::kPack2x16unorm:
            return builder->Call(str.str(), "f2");
        case BuiltinType::kPack4x8snorm:
        case BuiltinType::kPack4x8unorm:
            return builder->Call(str.str(), "f4");
        case BuiltinType::kUnpack4x8snorm:
        case BuiltinType::kUnpack4x8unorm:
        case BuiltinType::kUnpack2x16snorm:
        case BuiltinType::kUnpack2x16unorm:
            return builder->Call(str.str(), "u1");
        case BuiltinType::kWorkgroupBarrier:
            return builder->Call(str.str());
        case BuiltinType::kTranspose:
            return builder->Call(str.str(), "m3x2");
        default:
            break;
    }
    return nullptr;
}

using MslBuiltinTest = TestParamHelper<BuiltinData>;
TEST_P(MslBuiltinTest, Emit) {
    auto param = GetParam();

    Global("f2", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    Global("f3", ty.vec3<f32>(), ast::StorageClass::kPrivate);
    Global("f4", ty.vec4<f32>(), ast::StorageClass::kPrivate);
    Global("u1", ty.u32(), ast::StorageClass::kPrivate);
    Global("u2", ty.vec2<u32>(), ast::StorageClass::kPrivate);
    Global("i2", ty.vec2<i32>(), ast::StorageClass::kPrivate);
    Global("b2", ty.vec2<bool>(), ast::StorageClass::kPrivate);
    Global("m2x2", ty.mat2x2<f32>(), ast::StorageClass::kPrivate);
    Global("m3x2", ty.mat3x2<f32>(), ast::StorageClass::kPrivate);

    auto* call = GenerateCall(param.builtin, param.type, this);
    ASSERT_NE(nullptr, call) << "Unhandled builtin";
    Func("func", {}, ty.void_(), {Ignore(call)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    GeneratorImpl& gen = Build();

    auto* sem = program->Sem().Get(call);
    ASSERT_NE(sem, nullptr);
    auto* target = sem->Target();
    ASSERT_NE(target, nullptr);
    auto* builtin = target->As<sem::Builtin>();
    ASSERT_NE(builtin, nullptr);

    EXPECT_EQ(gen.generate_builtin_name(builtin), param.msl_name);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslBuiltinTest,
    testing::Values(
        BuiltinData{BuiltinType::kAbs, ParamType::kF32, "fabs"},
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
        BuiltinData{BuiltinType::kCountLeadingZeros, ParamType::kU32, "clz"},
        BuiltinData{BuiltinType::kCountOneBits, ParamType::kU32, "popcount"},
        BuiltinData{BuiltinType::kCountTrailingZeros, ParamType::kU32, "ctz"},
        BuiltinData{BuiltinType::kCross, ParamType::kF32, "cross"},
        BuiltinData{BuiltinType::kDeterminant, ParamType::kF32, "determinant"},
        BuiltinData{BuiltinType::kDistance, ParamType::kF32, "distance"},
        BuiltinData{BuiltinType::kDot, ParamType::kF32, "dot"},
        BuiltinData{BuiltinType::kDpdx, ParamType::kF32, "dfdx"},
        BuiltinData{BuiltinType::kDpdxCoarse, ParamType::kF32, "dfdx"},
        BuiltinData{BuiltinType::kDpdxFine, ParamType::kF32, "dfdx"},
        BuiltinData{BuiltinType::kDpdy, ParamType::kF32, "dfdy"},
        BuiltinData{BuiltinType::kDpdyCoarse, ParamType::kF32, "dfdy"},
        BuiltinData{BuiltinType::kDpdyFine, ParamType::kF32, "dfdy"},
        BuiltinData{BuiltinType::kExp, ParamType::kF32, "exp"},
        BuiltinData{BuiltinType::kExp2, ParamType::kF32, "exp2"},
        BuiltinData{BuiltinType::kExtractBits, ParamType::kU32, "extract_bits"},
        BuiltinData{BuiltinType::kFaceForward, ParamType::kF32, "faceforward"},
        BuiltinData{BuiltinType::kFloor, ParamType::kF32, "floor"},
        BuiltinData{BuiltinType::kFma, ParamType::kF32, "fma"},
        BuiltinData{BuiltinType::kFract, ParamType::kF32, "fract"},
        BuiltinData{BuiltinType::kFwidth, ParamType::kF32, "fwidth"},
        BuiltinData{BuiltinType::kFwidthCoarse, ParamType::kF32, "fwidth"},
        BuiltinData{BuiltinType::kFwidthFine, ParamType::kF32, "fwidth"},
        BuiltinData{BuiltinType::kInsertBits, ParamType::kU32, "insert_bits"},
        BuiltinData{BuiltinType::kInverseSqrt, ParamType::kF32, "rsqrt"},
        BuiltinData{BuiltinType::kLdexp, ParamType::kF32, "ldexp"},
        BuiltinData{BuiltinType::kLength, ParamType::kF32, "length"},
        BuiltinData{BuiltinType::kLog, ParamType::kF32, "log"},
        BuiltinData{BuiltinType::kLog2, ParamType::kF32, "log2"},
        BuiltinData{BuiltinType::kMax, ParamType::kF32, "fmax"},
        BuiltinData{BuiltinType::kMax, ParamType::kU32, "max"},
        BuiltinData{BuiltinType::kMin, ParamType::kF32, "fmin"},
        BuiltinData{BuiltinType::kMin, ParamType::kU32, "min"},
        BuiltinData{BuiltinType::kNormalize, ParamType::kF32, "normalize"},
        BuiltinData{BuiltinType::kPack4x8snorm, ParamType::kF32, "pack_float_to_snorm4x8"},
        BuiltinData{BuiltinType::kPack4x8unorm, ParamType::kF32, "pack_float_to_unorm4x8"},
        BuiltinData{BuiltinType::kPack2x16snorm, ParamType::kF32, "pack_float_to_snorm2x16"},
        BuiltinData{BuiltinType::kPack2x16unorm, ParamType::kF32, "pack_float_to_unorm2x16"},
        BuiltinData{BuiltinType::kPow, ParamType::kF32, "pow"},
        BuiltinData{BuiltinType::kReflect, ParamType::kF32, "reflect"},
        BuiltinData{BuiltinType::kReverseBits, ParamType::kU32, "reverse_bits"},
        BuiltinData{BuiltinType::kRound, ParamType::kU32, "rint"},
        BuiltinData{BuiltinType::kSelect, ParamType::kF32, "select"},
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
        BuiltinData{BuiltinType::kTrunc, ParamType::kF32, "trunc"},
        BuiltinData{BuiltinType::kUnpack4x8snorm, ParamType::kU32, "unpack_snorm4x8_to_float"},
        BuiltinData{BuiltinType::kUnpack4x8unorm, ParamType::kU32, "unpack_unorm4x8_to_float"},
        BuiltinData{BuiltinType::kUnpack2x16snorm, ParamType::kU32, "unpack_snorm2x16_to_float"},
        BuiltinData{BuiltinType::kUnpack2x16unorm, ParamType::kU32, "unpack_unorm2x16_to_float"}));

TEST_F(MslGeneratorImplTest, Builtin_Call) {
    Global("param1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    Global("param2", ty.vec2<f32>(), ast::StorageClass::kPrivate);

    auto* call = Call("dot", "param1", "param2");
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "dot(param1, param2)");
}

TEST_F(MslGeneratorImplTest, StorageBarrier) {
    auto* call = Call("storageBarrier");
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "threadgroup_barrier(mem_flags::mem_device)");
}

TEST_F(MslGeneratorImplTest, WorkgroupBarrier) {
    auto* call = Call("workgroupBarrier");
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "threadgroup_barrier(mem_flags::mem_threadgroup)");
}

TEST_F(MslGeneratorImplTest, Degrees_Scalar) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

float tint_degrees(float param_0) {
  return param_0 * 57.295779513082322865;
}

kernel void test_function() {
  float val = 0.0f;
  float const tint_symbol = tint_degrees(val);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Degrees_Vector) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("degrees", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

float3 tint_degrees(float3 param_0) {
  return param_0 * 57.295779513082322865;
}

kernel void test_function() {
  float3 val = 0.0f;
  float3 const tint_symbol = tint_degrees(val);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Radians_Scalar) {
    auto* val = Var("val", ty.f32());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

float tint_radians(float param_0) {
  return param_0 * 0.017453292519943295474;
}

kernel void test_function() {
  float val = 0.0f;
  float const tint_symbol = tint_radians(val);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Radians_Vector) {
    auto* val = Var("val", ty.vec3<f32>());
    auto* call = Call("radians", val);
    WrapInFunction(val, call);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

float3 tint_radians(float3 param_0) {
  return param_0 * 0.017453292519943295474;
}

kernel void test_function() {
  float3 val = 0.0f;
  float3 const tint_symbol = tint_radians(val);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Pack2x16Float) {
    auto* call = Call("pack2x16float", "p1");
    Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "as_type<uint>(half2(p1))");
}

TEST_F(MslGeneratorImplTest, Unpack2x16Float) {
    auto* call = Call("unpack2x16float", "p1");
    Global("p1", ty.u32(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(call));

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
    EXPECT_EQ(out.str(), "float2(as_type<half2>(p1))");
}

TEST_F(MslGeneratorImplTest, DotI32) {
    Global("v", ty.vec3<i32>(), ast::StorageClass::kPrivate);
    WrapInFunction(CallStmt(Call("dot", "v", "v")));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

template<typename T>
T tint_dot3(vec<T,3> a, vec<T,3> b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
kernel void test_function() {
  thread int3 tint_symbol = 0;
  tint_dot3(tint_symbol, tint_symbol);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Ignore) {
    Func("f", {Param("a", ty.i32()), Param("b", ty.i32()), Param("c", ty.i32())}, ty.i32(),
         {Return(Mul(Add("a", "b"), "c"))});

    Func("func", {}, ty.void_(), {CallStmt(Call("f", 1_i, 2_i, 3_i))},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
int f(int a, int b, int c) {
  return as_type<int>((as_type<uint>(as_type<int>((as_type<uint>(a) + as_type<uint>(b)))) * as_type<uint>(c)));
}

kernel void func() {
  f(1, 2, 3);
  return;
}

)");
}

}  // namespace
}  // namespace tint::writer::msl
