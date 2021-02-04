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

#include "gmock/gmock.h"
#include "src/ast/call_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/program.h"
#include "src/semantic/call.h"
#include "src/type/f32_type.h"
#include "src/type/vector_type.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using ::testing::HasSubstr;

using HlslGeneratorImplTest_Intrinsic = TestHelper;

enum class ParamType {
  kF32,
  kU32,
  kBool,
};

struct IntrinsicData {
  semantic::Intrinsic intrinsic;
  ParamType type;
  const char* hlsl_name;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
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
using HlslIntrinsicTest = TestParamHelper<IntrinsicData>;
TEST_P(HlslIntrinsicTest, Emit) {
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

  EXPECT_EQ(gen.generate_builtin_name(intrinsic), param.hlsl_name);
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_Intrinsic,
    HlslIntrinsicTest,
    testing::Values(
        IntrinsicData{semantic::Intrinsic::kAbs, ParamType::kF32, "abs"},
        IntrinsicData{semantic::Intrinsic::kAbs, ParamType::kU32, "abs"},
        IntrinsicData{semantic::Intrinsic::kAcos, ParamType::kF32, "acos"},
        IntrinsicData{semantic::Intrinsic::kAll, ParamType::kBool, "all"},
        IntrinsicData{semantic::Intrinsic::kAny, ParamType::kBool, "any"},
        IntrinsicData{semantic::Intrinsic::kAsin, ParamType::kF32, "asin"},
        IntrinsicData{semantic::Intrinsic::kAtan, ParamType::kF32, "atan"},
        IntrinsicData{semantic::Intrinsic::kAtan2, ParamType::kF32, "atan2"},
        IntrinsicData{semantic::Intrinsic::kCeil, ParamType::kF32, "ceil"},
        IntrinsicData{semantic::Intrinsic::kClamp, ParamType::kF32, "clamp"},
        IntrinsicData{semantic::Intrinsic::kClamp, ParamType::kU32, "clamp"},
        IntrinsicData{semantic::Intrinsic::kCos, ParamType::kF32, "cos"},
        IntrinsicData{semantic::Intrinsic::kCosh, ParamType::kF32, "cosh"},
        IntrinsicData{semantic::Intrinsic::kCountOneBits, ParamType::kU32,
                      "countbits"},
        IntrinsicData{semantic::Intrinsic::kCross, ParamType::kF32, "cross"},
        IntrinsicData{semantic::Intrinsic::kDeterminant, ParamType::kF32,
                      "determinant"},
        IntrinsicData{semantic::Intrinsic::kDistance, ParamType::kF32,
                      "distance"},
        IntrinsicData{semantic::Intrinsic::kDot, ParamType::kF32, "dot"},
        IntrinsicData{semantic::Intrinsic::kDpdx, ParamType::kF32, "ddx"},
        IntrinsicData{semantic::Intrinsic::kDpdxCoarse, ParamType::kF32,
                      "ddx_coarse"},
        IntrinsicData{semantic::Intrinsic::kDpdxFine, ParamType::kF32,
                      "ddx_fine"},
        IntrinsicData{semantic::Intrinsic::kDpdy, ParamType::kF32, "ddy"},
        IntrinsicData{semantic::Intrinsic::kDpdyCoarse, ParamType::kF32,
                      "ddy_coarse"},
        IntrinsicData{semantic::Intrinsic::kDpdyFine, ParamType::kF32,
                      "ddy_fine"},
        IntrinsicData{semantic::Intrinsic::kExp, ParamType::kF32, "exp"},
        IntrinsicData{semantic::Intrinsic::kExp2, ParamType::kF32, "exp2"},
        IntrinsicData{semantic::Intrinsic::kFaceForward, ParamType::kF32,
                      "faceforward"},
        IntrinsicData{semantic::Intrinsic::kFloor, ParamType::kF32, "floor"},
        IntrinsicData{semantic::Intrinsic::kFma, ParamType::kF32, "fma"},
        IntrinsicData{semantic::Intrinsic::kFract, ParamType::kF32, "frac"},
        IntrinsicData{semantic::Intrinsic::kFwidth, ParamType::kF32, "fwidth"},
        IntrinsicData{semantic::Intrinsic::kFwidthCoarse, ParamType::kF32,
                      "fwidth"},
        IntrinsicData{semantic::Intrinsic::kFwidthFine, ParamType::kF32,
                      "fwidth"},
        IntrinsicData{semantic::Intrinsic::kInverseSqrt, ParamType::kF32,
                      "rsqrt"},
        IntrinsicData{semantic::Intrinsic::kIsFinite, ParamType::kF32,
                      "isfinite"},
        IntrinsicData{semantic::Intrinsic::kIsInf, ParamType::kF32, "isinf"},
        IntrinsicData{semantic::Intrinsic::kIsNan, ParamType::kF32, "isnan"},
        IntrinsicData{semantic::Intrinsic::kLdexp, ParamType::kF32, "ldexp"},
        IntrinsicData{semantic::Intrinsic::kLength, ParamType::kF32, "length"},
        IntrinsicData{semantic::Intrinsic::kLog, ParamType::kF32, "log"},
        IntrinsicData{semantic::Intrinsic::kLog2, ParamType::kF32, "log2"},
        IntrinsicData{semantic::Intrinsic::kMax, ParamType::kF32, "max"},
        IntrinsicData{semantic::Intrinsic::kMax, ParamType::kU32, "max"},
        IntrinsicData{semantic::Intrinsic::kMin, ParamType::kF32, "min"},
        IntrinsicData{semantic::Intrinsic::kMin, ParamType::kU32, "min"},
        IntrinsicData{semantic::Intrinsic::kNormalize, ParamType::kF32,
                      "normalize"},
        IntrinsicData{semantic::Intrinsic::kPow, ParamType::kF32, "pow"},
        IntrinsicData{semantic::Intrinsic::kReflect, ParamType::kF32,
                      "reflect"},
        IntrinsicData{semantic::Intrinsic::kReverseBits, ParamType::kU32,
                      "reversebits"},
        IntrinsicData{semantic::Intrinsic::kRound, ParamType::kU32, "round"},
        IntrinsicData{semantic::Intrinsic::kSign, ParamType::kF32, "sign"},
        IntrinsicData{semantic::Intrinsic::kSin, ParamType::kF32, "sin"},
        IntrinsicData{semantic::Intrinsic::kSinh, ParamType::kF32, "sinh"},
        IntrinsicData{semantic::Intrinsic::kSmoothStep, ParamType::kF32,
                      "smoothstep"},
        IntrinsicData{semantic::Intrinsic::kSqrt, ParamType::kF32, "sqrt"},
        IntrinsicData{semantic::Intrinsic::kStep, ParamType::kF32, "step"},
        IntrinsicData{semantic::Intrinsic::kTan, ParamType::kF32, "tan"},
        IntrinsicData{semantic::Intrinsic::kTanh, ParamType::kF32, "tanh"},
        IntrinsicData{semantic::Intrinsic::kTrunc, ParamType::kF32, "trunc"}));

TEST_F(HlslGeneratorImplTest_Intrinsic, DISABLED_Intrinsic_IsNormal) {
  FAIL();
}

TEST_F(HlslGeneratorImplTest_Intrinsic, DISABLED_Intrinsic_Select) {
  FAIL();
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Intrinsic_Call) {
  auto* call = Call("dot", "param1", "param2");

  Global("param1", ast::StorageClass::kFunction, ty.vec3<f32>());
  Global("param2", ast::StorageClass::kFunction, ty.vec3<f32>());

  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_EQ(result(), "  dot(param1, param2)");
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack4x8Snorm) {
  auto* call = Call("pack4x8snorm", "p1");
  Global("p1", ast::StorageClass::kPrivate, ty.vec4<f32>());
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("int4 _tint_tmp = int4(round(clamp(p1, "
                                      "-1.0, 1.0) * 127.0)) & 0xff;"));
  EXPECT_THAT(result(), HasSubstr("asuint(_tint_tmp.x | _tint_tmp.y << 8 | "
                                  "_tint_tmp.z << 16 | _tint_tmp.w << 24)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack4x8Unorm) {
  auto* call = Call("pack4x8unorm", "p1");
  Global("p1", ast::StorageClass::kPrivate, ty.vec4<f32>());
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint4 _tint_tmp = uint4(round(clamp(p1, "
                                      "0.0, 1.0) * 255.0));"));
  EXPECT_THAT(result(), HasSubstr("(_tint_tmp.x | _tint_tmp.y << 8 | "
                                  "_tint_tmp.z << 16 | _tint_tmp.w << 24)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack2x16Snorm) {
  auto* call = Call("pack2x16snorm", "p1");
  Global("p1", ast::StorageClass::kPrivate, ty.vec4<f32>());
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("int2 _tint_tmp = int2(round(clamp(p1, "
                                      "-1.0, 1.0) * 32767.0)) & 0xffff;"));
  EXPECT_THAT(result(), HasSubstr("asuint(_tint_tmp.x | _tint_tmp.y << 16)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack2x16Unorm) {
  auto* call = Call("pack2x16unorm", "p1");
  Global("p1", ast::StorageClass::kPrivate, ty.vec4<f32>());
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint2 _tint_tmp = uint2(round(clamp(p1, "
                                      "0.0, 1.0) * 65535.0));"));
  EXPECT_THAT(result(), HasSubstr("(_tint_tmp.x | _tint_tmp.y << 16)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack2x16float) {
  auto* call = Call("pack2x16float", "p1");
  Global("p1", ast::StorageClass::kPrivate, ty.vec4<f32>());
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint2 _tint_tmp = f32tof16(p1);"));
  EXPECT_THAT(result(), HasSubstr("(_tint_tmp.x | _tint_tmp.y << 16)"));
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
