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

using MslGeneratorImplTest = TestHelper;

enum class ParamType {
  kF32,
  kU32,
  kBool,
};

struct IntrinsicData {
  semantic::Intrinsic intrinsic;
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

ast::CallExpression* GenerateCall(semantic::Intrinsic intrinsic,
                                  ParamType type,
                                  ProgramBuilder* builder) {
  std::string name;
  std::ostringstream str(name);
  str << intrinsic;
  switch (intrinsic) {
    case semantic::Intrinsic::kAcos:
    case semantic::Intrinsic::kAsin:
    case semantic::Intrinsic::kAtan:
    case semantic::Intrinsic::kCeil:
    case semantic::Intrinsic::kCos:
    case semantic::Intrinsic::kCosh:
    case semantic::Intrinsic::kDpdx:
    case semantic::Intrinsic::kDpdxCoarse:
    case semantic::Intrinsic::kDpdxFine:
    case semantic::Intrinsic::kDpdy:
    case semantic::Intrinsic::kDpdyCoarse:
    case semantic::Intrinsic::kDpdyFine:
    case semantic::Intrinsic::kExp:
    case semantic::Intrinsic::kExp2:
    case semantic::Intrinsic::kFloor:
    case semantic::Intrinsic::kFract:
    case semantic::Intrinsic::kFwidth:
    case semantic::Intrinsic::kFwidthCoarse:
    case semantic::Intrinsic::kFwidthFine:
    case semantic::Intrinsic::kInverseSqrt:
    case semantic::Intrinsic::kIsFinite:
    case semantic::Intrinsic::kIsInf:
    case semantic::Intrinsic::kIsNan:
    case semantic::Intrinsic::kIsNormal:
    case semantic::Intrinsic::kLdexp:
    case semantic::Intrinsic::kLength:
    case semantic::Intrinsic::kLog:
    case semantic::Intrinsic::kLog2:
    case semantic::Intrinsic::kNormalize:
    case semantic::Intrinsic::kReflect:
    case semantic::Intrinsic::kRound:
    case semantic::Intrinsic::kSin:
    case semantic::Intrinsic::kSinh:
    case semantic::Intrinsic::kSqrt:
    case semantic::Intrinsic::kTan:
    case semantic::Intrinsic::kTanh:
    case semantic::Intrinsic::kTrunc:
    case semantic::Intrinsic::kSign:
      return builder->Call(str.str(), "f1");
    case semantic::Intrinsic::kAtan2:
    case semantic::Intrinsic::kCross:
    case semantic::Intrinsic::kDot:
    case semantic::Intrinsic::kDistance:
    case semantic::Intrinsic::kPow:
    case semantic::Intrinsic::kStep:
      return builder->Call(str.str(), "f1", "f2");
    case semantic::Intrinsic::kFma:
    case semantic::Intrinsic::kMix:
    case semantic::Intrinsic::kFaceForward:
    case semantic::Intrinsic::kSmoothStep:
      return builder->Call(str.str(), "f1", "f2", "f3");
    case semantic::Intrinsic::kAll:
    case semantic::Intrinsic::kAny:
      return builder->Call(str.str(), "b1");
    case semantic::Intrinsic::kAbs:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f1");
      } else {
        return builder->Call(str.str(), "u1");
      }
    case semantic::Intrinsic::kCountOneBits:
    case semantic::Intrinsic::kReverseBits:
      return builder->Call(str.str(), "u1");
    case semantic::Intrinsic::kMax:
    case semantic::Intrinsic::kMin:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f1", "f2");
      } else {
        return builder->Call(str.str(), "u1", "u2");
      }
    case semantic::Intrinsic::kClamp:
      if (type == ParamType::kF32) {
        return builder->Call(str.str(), "f1", "f2", "f3");
      } else {
        return builder->Call(str.str(), "u1", "u2", "u3");
      }
    case semantic::Intrinsic::kSelect:
      return builder->Call(str.str(), "f1", "f2", "b1");
    case semantic::Intrinsic::kDeterminant:
      return builder->Call(str.str(), "m1");
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
        IntrinsicData{semantic::Intrinsic::kAbs, ParamType::kF32,
                      "metal::fabs"},
        IntrinsicData{semantic::Intrinsic::kAbs, ParamType::kU32, "metal::abs"},
        IntrinsicData{semantic::Intrinsic::kAcos, ParamType::kF32,
                      "metal::acos"},
        IntrinsicData{semantic::Intrinsic::kAll, ParamType::kBool,
                      "metal::all"},
        IntrinsicData{semantic::Intrinsic::kAny, ParamType::kBool,
                      "metal::any"},
        IntrinsicData{semantic::Intrinsic::kAsin, ParamType::kF32,
                      "metal::asin"},
        IntrinsicData{semantic::Intrinsic::kAtan, ParamType::kF32,
                      "metal::atan"},
        IntrinsicData{semantic::Intrinsic::kAtan2, ParamType::kF32,
                      "metal::atan2"},
        IntrinsicData{semantic::Intrinsic::kCeil, ParamType::kF32,
                      "metal::ceil"},
        IntrinsicData{semantic::Intrinsic::kClamp, ParamType::kF32,
                      "metal::clamp"},
        IntrinsicData{semantic::Intrinsic::kClamp, ParamType::kU32,
                      "metal::clamp"},
        IntrinsicData{semantic::Intrinsic::kCos, ParamType::kF32, "metal::cos"},
        IntrinsicData{semantic::Intrinsic::kCosh, ParamType::kF32,
                      "metal::cosh"},
        IntrinsicData{semantic::Intrinsic::kCountOneBits, ParamType::kU32,
                      "metal::popcount"},
        IntrinsicData{semantic::Intrinsic::kCross, ParamType::kF32,
                      "metal::cross"},
        IntrinsicData{semantic::Intrinsic::kDeterminant, ParamType::kF32,
                      "metal::determinant"},
        IntrinsicData{semantic::Intrinsic::kDistance, ParamType::kF32,
                      "metal::distance"},
        IntrinsicData{semantic::Intrinsic::kDot, ParamType::kF32, "metal::dot"},
        IntrinsicData{semantic::Intrinsic::kDpdx, ParamType::kF32,
                      "metal::dfdx"},
        IntrinsicData{semantic::Intrinsic::kDpdxCoarse, ParamType::kF32,
                      "metal::dfdx"},
        IntrinsicData{semantic::Intrinsic::kDpdxFine, ParamType::kF32,
                      "metal::dfdx"},
        IntrinsicData{semantic::Intrinsic::kDpdy, ParamType::kF32,
                      "metal::dfdy"},
        IntrinsicData{semantic::Intrinsic::kDpdyCoarse, ParamType::kF32,
                      "metal::dfdy"},
        IntrinsicData{semantic::Intrinsic::kDpdyFine, ParamType::kF32,
                      "metal::dfdy"},
        IntrinsicData{semantic::Intrinsic::kExp, ParamType::kF32, "metal::exp"},
        IntrinsicData{semantic::Intrinsic::kExp2, ParamType::kF32,
                      "metal::exp2"},
        IntrinsicData{semantic::Intrinsic::kFaceForward, ParamType::kF32,
                      "metal::faceforward"},
        IntrinsicData{semantic::Intrinsic::kFloor, ParamType::kF32,
                      "metal::floor"},
        IntrinsicData{semantic::Intrinsic::kFma, ParamType::kF32, "metal::fma"},
        IntrinsicData{semantic::Intrinsic::kFract, ParamType::kF32,
                      "metal::fract"},
        IntrinsicData{semantic::Intrinsic::kFwidth, ParamType::kF32,
                      "metal::fwidth"},
        IntrinsicData{semantic::Intrinsic::kFwidthCoarse, ParamType::kF32,
                      "metal::fwidth"},
        IntrinsicData{semantic::Intrinsic::kFwidthFine, ParamType::kF32,
                      "metal::fwidth"},
        IntrinsicData{semantic::Intrinsic::kInverseSqrt, ParamType::kF32,
                      "metal::rsqrt"},
        IntrinsicData{semantic::Intrinsic::kIsFinite, ParamType::kF32,
                      "metal::isfinite"},
        IntrinsicData{semantic::Intrinsic::kIsInf, ParamType::kF32,
                      "metal::isinf"},
        IntrinsicData{semantic::Intrinsic::kIsNan, ParamType::kF32,
                      "metal::isnan"},
        IntrinsicData{semantic::Intrinsic::kIsNormal, ParamType::kF32,
                      "metal::isnormal"},
        IntrinsicData{semantic::Intrinsic::kLdexp, ParamType::kF32,
                      "metal::ldexp"},
        IntrinsicData{semantic::Intrinsic::kLength, ParamType::kF32,
                      "metal::length"},
        IntrinsicData{semantic::Intrinsic::kLog, ParamType::kF32, "metal::log"},
        IntrinsicData{semantic::Intrinsic::kLog2, ParamType::kF32,
                      "metal::log2"},
        IntrinsicData{semantic::Intrinsic::kMax, ParamType::kF32,
                      "metal::fmax"},
        IntrinsicData{semantic::Intrinsic::kMax, ParamType::kU32, "metal::max"},
        IntrinsicData{semantic::Intrinsic::kMin, ParamType::kF32,
                      "metal::fmin"},
        IntrinsicData{semantic::Intrinsic::kMin, ParamType::kU32, "metal::min"},
        IntrinsicData{semantic::Intrinsic::kNormalize, ParamType::kF32,
                      "metal::normalize"},
        IntrinsicData{semantic::Intrinsic::kPow, ParamType::kF32, "metal::pow"},
        IntrinsicData{semantic::Intrinsic::kReflect, ParamType::kF32,
                      "metal::reflect"},
        IntrinsicData{semantic::Intrinsic::kReverseBits, ParamType::kU32,
                      "metal::reverse_bits"},
        IntrinsicData{semantic::Intrinsic::kRound, ParamType::kU32,
                      "metal::round"},
        IntrinsicData{semantic::Intrinsic::kSelect, ParamType::kF32,
                      "metal::select"},
        IntrinsicData{semantic::Intrinsic::kSign, ParamType::kF32,
                      "metal::sign"},
        IntrinsicData{semantic::Intrinsic::kSin, ParamType::kF32, "metal::sin"},
        IntrinsicData{semantic::Intrinsic::kSinh, ParamType::kF32,
                      "metal::sinh"},
        IntrinsicData{semantic::Intrinsic::kSmoothStep, ParamType::kF32,
                      "metal::smoothstep"},
        IntrinsicData{semantic::Intrinsic::kSqrt, ParamType::kF32,
                      "metal::sqrt"},
        IntrinsicData{semantic::Intrinsic::kStep, ParamType::kF32,
                      "metal::step"},
        IntrinsicData{semantic::Intrinsic::kTan, ParamType::kF32, "metal::tan"},
        IntrinsicData{semantic::Intrinsic::kTanh, ParamType::kF32,
                      "metal::tanh"},
        IntrinsicData{semantic::Intrinsic::kTrunc, ParamType::kF32,
                      "metal::trunc"}));

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

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
