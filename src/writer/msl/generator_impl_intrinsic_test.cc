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

#include "src/sem/call.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using IntrinsicType = sem::IntrinsicType;

using MslGeneratorImplTest = TestHelper;

enum class ParamType {
  kF32,
  kU32,
  kBool,
};

struct IntrinsicData {
  IntrinsicType intrinsic;
  ParamType type;
  const char* msl_name;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
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

ast::CallExpression* GenerateCall(IntrinsicType intrinsic,
                                  ParamType type,
                                  ProgramBuilder* builder) {
  std::string name;
  std::ostringstream str(name);
  str << intrinsic;
  switch (intrinsic) {
    case IntrinsicType::kAcos:
    case IntrinsicType::kAsin:
    case IntrinsicType::kAtan:
    case IntrinsicType::kCeil:
    case IntrinsicType::kCos:
    case IntrinsicType::kCosh:
    case IntrinsicType::kDpdx:
    case IntrinsicType::kDpdxCoarse:
    case IntrinsicType::kDpdxFine:
    case IntrinsicType::kDpdy:
    case IntrinsicType::kDpdyCoarse:
    case IntrinsicType::kDpdyFine:
    case IntrinsicType::kExp:
    case IntrinsicType::kExp2:
    case IntrinsicType::kFloor:
    case IntrinsicType::kFract:
    case IntrinsicType::kFwidth:
    case IntrinsicType::kFwidthCoarse:
    case IntrinsicType::kFwidthFine:
    case IntrinsicType::kInverseSqrt:
    case IntrinsicType::kIsFinite:
    case IntrinsicType::kIsInf:
    case IntrinsicType::kIsNan:
    case IntrinsicType::kIsNormal:
    case IntrinsicType::kLength:
    case IntrinsicType::kLog:
    case IntrinsicType::kLog2:
    case IntrinsicType::kNormalize:
    case IntrinsicType::kRound:
    case IntrinsicType::kSin:
    case IntrinsicType::kSinh:
    case IntrinsicType::kSqrt:
    case IntrinsicType::kTan:
    case IntrinsicType::kTanh:
    case IntrinsicType::kTrunc:
    case IntrinsicType::kSign:
      return builder->Call(str.str(), "f2");
    case IntrinsicType::kLdexp:
      return builder->Call(str.str(), "f2", "u2");
    case IntrinsicType::kAtan2:
    case IntrinsicType::kDot:
    case IntrinsicType::kDistance:
    case IntrinsicType::kPow:
    case IntrinsicType::kReflect:
    case IntrinsicType::kStep:
      return builder->Call(str.str(), "f2", "f2");
    case IntrinsicType::kStorageBarrier:
      return builder->Call(str.str());
    case IntrinsicType::kCross:
      return builder->Call(str.str(), "f3", "f3");
    case IntrinsicType::kFma:
    case IntrinsicType::kMix:
    case IntrinsicType::kFaceForward:
    case IntrinsicType::kSmoothStep:
      return builder->Call(str.str(), "f2", "f2", "f2");
    case IntrinsicType::kAll:
    case IntrinsicType::kAny:
      return builder->Call(str.str(), "b2");
    case IntrinsicType::kAbs:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f2");
      } else {
        return builder->Call(str.str(), "u2");
      }
    case IntrinsicType::kCountOneBits:
    case IntrinsicType::kReverseBits:
      return builder->Call(str.str(), "u2");
    case IntrinsicType::kMax:
    case IntrinsicType::kMin:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f2", "f2");
      } else {
        return builder->Call(str.str(), "u2", "u2");
      }
    case IntrinsicType::kClamp:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f2", "f2", "f2");
      } else {
        return builder->Call(str.str(), "u2", "u2", "u2");
      }
    case IntrinsicType::kSelect:
      return builder->Call(str.str(), "f2", "f2", "b2");
    case IntrinsicType::kDeterminant:
      return builder->Call(str.str(), "m2x2");
    case IntrinsicType::kPack2x16Snorm:
    case IntrinsicType::kPack2x16Unorm:
      return builder->Call(str.str(), "f2");
    case IntrinsicType::kPack4x8Snorm:
    case IntrinsicType::kPack4x8Unorm:
      return builder->Call(str.str(), "f4");
    case IntrinsicType::kUnpack4x8Snorm:
    case IntrinsicType::kUnpack4x8Unorm:
    case IntrinsicType::kUnpack2x16Snorm:
    case IntrinsicType::kUnpack2x16Unorm:
      return builder->Call(str.str(), "u1");
    case IntrinsicType::kWorkgroupBarrier:
      return builder->Call(str.str());
    default:
      break;
  }
  return nullptr;
}

using MslIntrinsicTest = TestParamHelper<IntrinsicData>;
TEST_P(MslIntrinsicTest, Emit) {
  auto param = GetParam();

  Global("f2", ty.vec2<float>(), ast::StorageClass::kPrivate);
  Global("f3", ty.vec3<float>(), ast::StorageClass::kPrivate);
  Global("f4", ty.vec4<float>(), ast::StorageClass::kPrivate);
  Global("u1", ty.u32(), ast::StorageClass::kPrivate);
  Global("u2", ty.vec2<unsigned int>(), ast::StorageClass::kPrivate);
  Global("b2", ty.vec2<bool>(), ast::StorageClass::kPrivate);
  Global("m2x2", ty.mat2x2<float>(), ast::StorageClass::kPrivate);

  auto* call = GenerateCall(param.intrinsic, param.type, this);
  ASSERT_NE(nullptr, call) << "Unhandled intrinsic";
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  auto* sem = program->Sem().Get(call);
  ASSERT_NE(sem, nullptr);
  auto* target = sem->Target();
  ASSERT_NE(target, nullptr);
  auto* intrinsic = target->As<sem::Intrinsic>();
  ASSERT_NE(intrinsic, nullptr);

  EXPECT_EQ(gen.generate_builtin_name(intrinsic), param.msl_name);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslIntrinsicTest,
    testing::Values(
        IntrinsicData{IntrinsicType::kAbs, ParamType::kF32, "fabs"},
        IntrinsicData{IntrinsicType::kAbs, ParamType::kU32, "abs"},
        IntrinsicData{IntrinsicType::kAcos, ParamType::kF32, "acos"},
        IntrinsicData{IntrinsicType::kAll, ParamType::kBool, "all"},
        IntrinsicData{IntrinsicType::kAny, ParamType::kBool, "any"},
        IntrinsicData{IntrinsicType::kAsin, ParamType::kF32, "asin"},
        IntrinsicData{IntrinsicType::kAtan, ParamType::kF32, "atan"},
        IntrinsicData{IntrinsicType::kAtan2, ParamType::kF32, "atan2"},
        IntrinsicData{IntrinsicType::kCeil, ParamType::kF32, "ceil"},
        IntrinsicData{IntrinsicType::kClamp, ParamType::kF32, "clamp"},
        IntrinsicData{IntrinsicType::kClamp, ParamType::kU32, "clamp"},
        IntrinsicData{IntrinsicType::kCos, ParamType::kF32, "cos"},
        IntrinsicData{IntrinsicType::kCosh, ParamType::kF32, "cosh"},
        IntrinsicData{IntrinsicType::kCountOneBits, ParamType::kU32,
                      "popcount"},
        IntrinsicData{IntrinsicType::kCross, ParamType::kF32, "cross"},
        IntrinsicData{IntrinsicType::kDeterminant, ParamType::kF32,
                      "determinant"},
        IntrinsicData{IntrinsicType::kDistance, ParamType::kF32, "distance"},
        IntrinsicData{IntrinsicType::kDot, ParamType::kF32, "dot"},
        IntrinsicData{IntrinsicType::kDpdx, ParamType::kF32, "dfdx"},
        IntrinsicData{IntrinsicType::kDpdxCoarse, ParamType::kF32, "dfdx"},
        IntrinsicData{IntrinsicType::kDpdxFine, ParamType::kF32, "dfdx"},
        IntrinsicData{IntrinsicType::kDpdy, ParamType::kF32, "dfdy"},
        IntrinsicData{IntrinsicType::kDpdyCoarse, ParamType::kF32, "dfdy"},
        IntrinsicData{IntrinsicType::kDpdyFine, ParamType::kF32, "dfdy"},
        IntrinsicData{IntrinsicType::kExp, ParamType::kF32, "exp"},
        IntrinsicData{IntrinsicType::kExp2, ParamType::kF32, "exp2"},
        IntrinsicData{IntrinsicType::kFaceForward, ParamType::kF32,
                      "faceforward"},
        IntrinsicData{IntrinsicType::kFloor, ParamType::kF32, "floor"},
        IntrinsicData{IntrinsicType::kFma, ParamType::kF32, "fma"},
        IntrinsicData{IntrinsicType::kFract, ParamType::kF32, "fract"},
        IntrinsicData{IntrinsicType::kFwidth, ParamType::kF32, "fwidth"},
        IntrinsicData{IntrinsicType::kFwidthCoarse, ParamType::kF32, "fwidth"},
        IntrinsicData{IntrinsicType::kFwidthFine, ParamType::kF32, "fwidth"},
        IntrinsicData{IntrinsicType::kInverseSqrt, ParamType::kF32, "rsqrt"},
        IntrinsicData{IntrinsicType::kIsFinite, ParamType::kF32, "isfinite"},
        IntrinsicData{IntrinsicType::kIsInf, ParamType::kF32, "isinf"},
        IntrinsicData{IntrinsicType::kIsNan, ParamType::kF32, "isnan"},
        IntrinsicData{IntrinsicType::kIsNormal, ParamType::kF32, "isnormal"},
        IntrinsicData{IntrinsicType::kLdexp, ParamType::kF32, "ldexp"},
        IntrinsicData{IntrinsicType::kLength, ParamType::kF32, "length"},
        IntrinsicData{IntrinsicType::kLog, ParamType::kF32, "log"},
        IntrinsicData{IntrinsicType::kLog2, ParamType::kF32, "log2"},
        IntrinsicData{IntrinsicType::kMax, ParamType::kF32, "fmax"},
        IntrinsicData{IntrinsicType::kMax, ParamType::kU32, "max"},
        IntrinsicData{IntrinsicType::kMin, ParamType::kF32, "fmin"},
        IntrinsicData{IntrinsicType::kMin, ParamType::kU32, "min"},
        IntrinsicData{IntrinsicType::kNormalize, ParamType::kF32, "normalize"},
        IntrinsicData{IntrinsicType::kPack4x8Snorm, ParamType::kF32,
                      "pack_float_to_snorm4x8"},
        IntrinsicData{IntrinsicType::kPack4x8Unorm, ParamType::kF32,
                      "pack_float_to_unorm4x8"},
        IntrinsicData{IntrinsicType::kPack2x16Snorm, ParamType::kF32,
                      "pack_float_to_snorm2x16"},
        IntrinsicData{IntrinsicType::kPack2x16Unorm, ParamType::kF32,
                      "pack_float_to_unorm2x16"},
        IntrinsicData{IntrinsicType::kPow, ParamType::kF32, "pow"},
        IntrinsicData{IntrinsicType::kReflect, ParamType::kF32, "reflect"},
        IntrinsicData{IntrinsicType::kReverseBits, ParamType::kU32,
                      "reverse_bits"},
        IntrinsicData{IntrinsicType::kRound, ParamType::kU32, "rint"},
        IntrinsicData{IntrinsicType::kSelect, ParamType::kF32, "select"},
        IntrinsicData{IntrinsicType::kSign, ParamType::kF32, "sign"},
        IntrinsicData{IntrinsicType::kSin, ParamType::kF32, "sin"},
        IntrinsicData{IntrinsicType::kSinh, ParamType::kF32, "sinh"},
        IntrinsicData{IntrinsicType::kSmoothStep, ParamType::kF32,
                      "smoothstep"},
        IntrinsicData{IntrinsicType::kSqrt, ParamType::kF32, "sqrt"},
        IntrinsicData{IntrinsicType::kStep, ParamType::kF32, "step"},
        IntrinsicData{IntrinsicType::kTan, ParamType::kF32, "tan"},
        IntrinsicData{IntrinsicType::kTanh, ParamType::kF32, "tanh"},
        IntrinsicData{IntrinsicType::kTrunc, ParamType::kF32, "trunc"},
        IntrinsicData{IntrinsicType::kUnpack4x8Snorm, ParamType::kU32,
                      "unpack_snorm4x8_to_float"},
        IntrinsicData{IntrinsicType::kUnpack4x8Unorm, ParamType::kU32,
                      "unpack_unorm4x8_to_float"},
        IntrinsicData{IntrinsicType::kUnpack2x16Snorm, ParamType::kU32,
                      "unpack_snorm2x16_to_float"},
        IntrinsicData{IntrinsicType::kUnpack2x16Unorm, ParamType::kU32,
                      "unpack_unorm2x16_to_float"}));

TEST_F(MslGeneratorImplTest, Intrinsic_Call) {
  Global("param1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  Global("param2", ty.vec2<f32>(), ast::StorageClass::kPrivate);

  auto* call = Call("dot", "param1", "param2");
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "  dot(param1, param2)");
}

TEST_F(MslGeneratorImplTest, StorageBarrier) {
  auto* call = Call("storageBarrier");
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "  threadgroup_barrier(mem_flags::mem_device)");
}

TEST_F(MslGeneratorImplTest, WorkgroupBarrier) {
  auto* call = Call("workgroupBarrier");
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "  threadgroup_barrier(mem_flags::mem_threadgroup)");
}

TEST_F(MslGeneratorImplTest, Pack2x16Float) {
  auto* call = Call("pack2x16float", "p1");
  Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "  as_type<uint>(half2(p1))");
}

TEST_F(MslGeneratorImplTest, Unpack2x16Float) {
  auto* call = Call("unpack2x16float", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "  float2(as_type<half2>(p1))");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
