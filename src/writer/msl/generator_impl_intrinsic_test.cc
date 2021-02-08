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

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/call_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/program.h"
#include "src/semantic/call.h"
#include "src/type/f32_type.h"
#include "src/type/vector_type.h"
#include "src/type_determiner.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using IntrinsicType = semantic::IntrinsicType;

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
    case IntrinsicType::kLdexp:
    case IntrinsicType::kLength:
    case IntrinsicType::kLog:
    case IntrinsicType::kLog2:
    case IntrinsicType::kNormalize:
    case IntrinsicType::kPack4x8Snorm:
    case IntrinsicType::kPack4x8Unorm:
    case IntrinsicType::kReflect:
    case IntrinsicType::kRound:
    case IntrinsicType::kSin:
    case IntrinsicType::kSinh:
    case IntrinsicType::kSqrt:
    case IntrinsicType::kTan:
    case IntrinsicType::kTanh:
    case IntrinsicType::kTrunc:
    case IntrinsicType::kSign:
      return builder->Call(str.str(), "f1");
    case IntrinsicType::kAtan2:
    case IntrinsicType::kCross:
    case IntrinsicType::kDot:
    case IntrinsicType::kDistance:
    case IntrinsicType::kPow:
    case IntrinsicType::kStep:
      return builder->Call(str.str(), "f1", "f2");
    case IntrinsicType::kFma:
    case IntrinsicType::kMix:
    case IntrinsicType::kFaceForward:
    case IntrinsicType::kSmoothStep:
      return builder->Call(str.str(), "f1", "f2", "f3");
    case IntrinsicType::kAll:
    case IntrinsicType::kAny:
      return builder->Call(str.str(), "b1");
    case IntrinsicType::kAbs:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f1");
      } else {
        return builder->Call(str.str(), "u1");
      }
    case IntrinsicType::kCountOneBits:
    case IntrinsicType::kReverseBits:
      return builder->Call(str.str(), "u1");
    case IntrinsicType::kMax:
    case IntrinsicType::kMin:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f1", "f2");
      } else {
        return builder->Call(str.str(), "u1", "u2");
      }
    case IntrinsicType::kClamp:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f1", "f2", "f3");
      } else {
        return builder->Call(str.str(), "u1", "u2", "u3");
      }
    case IntrinsicType::kSelect:
      return builder->Call(str.str(), "f1", "f2", "b1");
    case IntrinsicType::kDeterminant:
      return builder->Call(str.str(), "m1");
    case IntrinsicType::kPack2x16Snorm:
    case IntrinsicType::kPack2x16Unorm:
      return builder->Call(str.str(), "f4");
    default:
      break;
  }
  return nullptr;
}

using MslIntrinsicTest = TestParamHelper<IntrinsicData>;
TEST_P(MslIntrinsicTest, Emit) {
  auto param = GetParam();

  auto* call = GenerateCall(param.intrinsic, param.type, this);
  ASSERT_NE(nullptr, call) << "Unhandled intrinsic";
  WrapInFunction(call);

  Global("f1", ast::StorageClass::kFunction, ty.vec2<float>());
  Global("f2", ast::StorageClass::kFunction, ty.vec2<float>());
  Global("f3", ast::StorageClass::kFunction, ty.vec2<float>());
  Global("f4", ast::StorageClass::kFunction, ty.vec2<float>());
  Global("u1", ast::StorageClass::kFunction, ty.vec2<unsigned int>());
  Global("u2", ast::StorageClass::kFunction, ty.vec2<unsigned int>());
  Global("u3", ast::StorageClass::kFunction, ty.vec2<unsigned int>());
  Global("b1", ast::StorageClass::kFunction, ty.vec2<bool>());
  Global("m1", ast::StorageClass::kFunction, ty.mat2x2<float>());

  GeneratorImpl& gen = Build();

  auto* sem = program->Sem().Get(call);
  ASSERT_NE(sem, nullptr);
  auto* intrinsic = sem->As<semantic::IntrinsicCall>();
  ASSERT_NE(intrinsic, nullptr);

  EXPECT_EQ(gen.generate_builtin_name(intrinsic), param.msl_name);
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslIntrinsicTest,
    testing::Values(
        IntrinsicData{IntrinsicType::kAbs, ParamType::kF32, "metal::fabs"},
        IntrinsicData{IntrinsicType::kAbs, ParamType::kU32, "metal::abs"},
        IntrinsicData{IntrinsicType::kAcos, ParamType::kF32, "metal::acos"},
        IntrinsicData{IntrinsicType::kAll, ParamType::kBool, "metal::all"},
        IntrinsicData{IntrinsicType::kAny, ParamType::kBool, "metal::any"},
        IntrinsicData{IntrinsicType::kAsin, ParamType::kF32, "metal::asin"},
        IntrinsicData{IntrinsicType::kAtan, ParamType::kF32, "metal::atan"},
        IntrinsicData{IntrinsicType::kAtan2, ParamType::kF32, "metal::atan2"},
        IntrinsicData{IntrinsicType::kCeil, ParamType::kF32, "metal::ceil"},
        IntrinsicData{IntrinsicType::kClamp, ParamType::kF32, "metal::clamp"},
        IntrinsicData{IntrinsicType::kClamp, ParamType::kU32, "metal::clamp"},
        IntrinsicData{IntrinsicType::kCos, ParamType::kF32, "metal::cos"},
        IntrinsicData{IntrinsicType::kCosh, ParamType::kF32, "metal::cosh"},
        IntrinsicData{IntrinsicType::kCountOneBits, ParamType::kU32,
                      "metal::popcount"},
        IntrinsicData{IntrinsicType::kCross, ParamType::kF32, "metal::cross"},
        IntrinsicData{IntrinsicType::kDeterminant, ParamType::kF32,
                      "metal::determinant"},
        IntrinsicData{IntrinsicType::kDistance, ParamType::kF32,
                      "metal::distance"},
        IntrinsicData{IntrinsicType::kDot, ParamType::kF32, "metal::dot"},
        IntrinsicData{IntrinsicType::kDpdx, ParamType::kF32, "metal::dfdx"},
        IntrinsicData{IntrinsicType::kDpdxCoarse, ParamType::kF32,
                      "metal::dfdx"},
        IntrinsicData{IntrinsicType::kDpdxFine, ParamType::kF32, "metal::dfdx"},
        IntrinsicData{IntrinsicType::kDpdy, ParamType::kF32, "metal::dfdy"},
        IntrinsicData{IntrinsicType::kDpdyCoarse, ParamType::kF32,
                      "metal::dfdy"},
        IntrinsicData{IntrinsicType::kDpdyFine, ParamType::kF32, "metal::dfdy"},
        IntrinsicData{IntrinsicType::kExp, ParamType::kF32, "metal::exp"},
        IntrinsicData{IntrinsicType::kExp2, ParamType::kF32, "metal::exp2"},
        IntrinsicData{IntrinsicType::kFaceForward, ParamType::kF32,
                      "metal::faceforward"},
        IntrinsicData{IntrinsicType::kFloor, ParamType::kF32, "metal::floor"},
        IntrinsicData{IntrinsicType::kFma, ParamType::kF32, "metal::fma"},
        IntrinsicData{IntrinsicType::kFract, ParamType::kF32, "metal::fract"},
        IntrinsicData{IntrinsicType::kFwidth, ParamType::kF32, "metal::fwidth"},
        IntrinsicData{IntrinsicType::kFwidthCoarse, ParamType::kF32,
                      "metal::fwidth"},
        IntrinsicData{IntrinsicType::kFwidthFine, ParamType::kF32,
                      "metal::fwidth"},
        IntrinsicData{IntrinsicType::kInverseSqrt, ParamType::kF32,
                      "metal::rsqrt"},
        IntrinsicData{IntrinsicType::kIsFinite, ParamType::kF32,
                      "metal::isfinite"},
        IntrinsicData{IntrinsicType::kIsInf, ParamType::kF32, "metal::isinf"},
        IntrinsicData{IntrinsicType::kIsNan, ParamType::kF32, "metal::isnan"},
        IntrinsicData{IntrinsicType::kIsNormal, ParamType::kF32,
                      "metal::isnormal"},
        IntrinsicData{IntrinsicType::kLdexp, ParamType::kF32, "metal::ldexp"},
        IntrinsicData{IntrinsicType::kLength, ParamType::kF32, "metal::length"},
        IntrinsicData{IntrinsicType::kLog, ParamType::kF32, "metal::log"},
        IntrinsicData{IntrinsicType::kLog2, ParamType::kF32, "metal::log2"},
        IntrinsicData{IntrinsicType::kMax, ParamType::kF32, "metal::fmax"},
        IntrinsicData{IntrinsicType::kMax, ParamType::kU32, "metal::max"},
        IntrinsicData{IntrinsicType::kMin, ParamType::kF32, "metal::fmin"},
        IntrinsicData{IntrinsicType::kMin, ParamType::kU32, "metal::min"},
        IntrinsicData{IntrinsicType::kNormalize, ParamType::kF32,
                      "metal::normalize"},
        IntrinsicData{IntrinsicType::kPack4x8Snorm, ParamType::kF32,
                      "metal::pack_float_to_snorm4x8"},
        IntrinsicData{IntrinsicType::kPack4x8Unorm, ParamType::kF32,
                      "metal::pack_float_to_unorm4x8"},
        IntrinsicData{IntrinsicType::kPack2x16Snorm, ParamType::kF32,
                      "metal::pack_float_to_snorm2x16"},
        IntrinsicData{IntrinsicType::kPack2x16Unorm, ParamType::kF32,
                      "metal::pack_float_to_unorm2x16"},
        IntrinsicData{IntrinsicType::kPow, ParamType::kF32, "metal::pow"},
        IntrinsicData{IntrinsicType::kReflect, ParamType::kF32,
                      "metal::reflect"},
        IntrinsicData{IntrinsicType::kReverseBits, ParamType::kU32,
                      "metal::reverse_bits"},
        IntrinsicData{IntrinsicType::kRound, ParamType::kU32, "metal::round"},
        IntrinsicData{IntrinsicType::kSelect, ParamType::kF32, "metal::select"},
        IntrinsicData{IntrinsicType::kSign, ParamType::kF32, "metal::sign"},
        IntrinsicData{IntrinsicType::kSin, ParamType::kF32, "metal::sin"},
        IntrinsicData{IntrinsicType::kSinh, ParamType::kF32, "metal::sinh"},
        IntrinsicData{IntrinsicType::kSmoothStep, ParamType::kF32,
                      "metal::smoothstep"},
        IntrinsicData{IntrinsicType::kSqrt, ParamType::kF32, "metal::sqrt"},
        IntrinsicData{IntrinsicType::kStep, ParamType::kF32, "metal::step"},
        IntrinsicData{IntrinsicType::kTan, ParamType::kF32, "metal::tan"},
        IntrinsicData{IntrinsicType::kTanh, ParamType::kF32, "metal::tanh"},
        IntrinsicData{IntrinsicType::kTrunc, ParamType::kF32, "metal::trunc"}));

TEST_F(MslGeneratorImplTest, Intrinsic_Call) {
  Global("param1", ast::StorageClass::kFunction, ty.vec2<f32>());
  Global("param2", ast::StorageClass::kFunction, ty.vec2<f32>());

  auto* call = Call("dot", "param1", "param2");
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "  metal::dot(param1, param2)");
}

TEST_F(MslGeneratorImplTest, Pack2x16Float) {
  auto* call = Call("pack2x16float", "p1");
  Global("p1", ast::StorageClass::kFunction, ty.vec2<f32>());
  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(call)) << gen.error();
  EXPECT_EQ(gen.result(), "  as_type<uint>(half2(p1))");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
