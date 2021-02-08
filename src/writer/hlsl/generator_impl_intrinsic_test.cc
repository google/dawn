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

using IntrinsicType = semantic::IntrinsicType;

using ::testing::HasSubstr;

using HlslGeneratorImplTest_Intrinsic = TestHelper;

enum class ParamType {
  kF32,
  kU32,
  kBool,
};

struct IntrinsicData {
  IntrinsicType intrinsic;
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
        IntrinsicData{IntrinsicType::kAbs, ParamType::kF32, "abs"},
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
                      "countbits"},
        IntrinsicData{IntrinsicType::kCross, ParamType::kF32, "cross"},
        IntrinsicData{IntrinsicType::kDeterminant, ParamType::kF32,
                      "determinant"},
        IntrinsicData{IntrinsicType::kDistance, ParamType::kF32, "distance"},
        IntrinsicData{IntrinsicType::kDot, ParamType::kF32, "dot"},
        IntrinsicData{IntrinsicType::kDpdx, ParamType::kF32, "ddx"},
        IntrinsicData{IntrinsicType::kDpdxCoarse, ParamType::kF32,
                      "ddx_coarse"},
        IntrinsicData{IntrinsicType::kDpdxFine, ParamType::kF32, "ddx_fine"},
        IntrinsicData{IntrinsicType::kDpdy, ParamType::kF32, "ddy"},
        IntrinsicData{IntrinsicType::kDpdyCoarse, ParamType::kF32,
                      "ddy_coarse"},
        IntrinsicData{IntrinsicType::kDpdyFine, ParamType::kF32, "ddy_fine"},
        IntrinsicData{IntrinsicType::kExp, ParamType::kF32, "exp"},
        IntrinsicData{IntrinsicType::kExp2, ParamType::kF32, "exp2"},
        IntrinsicData{IntrinsicType::kFaceForward, ParamType::kF32,
                      "faceforward"},
        IntrinsicData{IntrinsicType::kFloor, ParamType::kF32, "floor"},
        IntrinsicData{IntrinsicType::kFma, ParamType::kF32, "fma"},
        IntrinsicData{IntrinsicType::kFract, ParamType::kF32, "frac"},
        IntrinsicData{IntrinsicType::kFwidth, ParamType::kF32, "fwidth"},
        IntrinsicData{IntrinsicType::kFwidthCoarse, ParamType::kF32, "fwidth"},
        IntrinsicData{IntrinsicType::kFwidthFine, ParamType::kF32, "fwidth"},
        IntrinsicData{IntrinsicType::kInverseSqrt, ParamType::kF32, "rsqrt"},
        IntrinsicData{IntrinsicType::kIsFinite, ParamType::kF32, "isfinite"},
        IntrinsicData{IntrinsicType::kIsInf, ParamType::kF32, "isinf"},
        IntrinsicData{IntrinsicType::kIsNan, ParamType::kF32, "isnan"},
        IntrinsicData{IntrinsicType::kLdexp, ParamType::kF32, "ldexp"},
        IntrinsicData{IntrinsicType::kLength, ParamType::kF32, "length"},
        IntrinsicData{IntrinsicType::kLog, ParamType::kF32, "log"},
        IntrinsicData{IntrinsicType::kLog2, ParamType::kF32, "log2"},
        IntrinsicData{IntrinsicType::kMax, ParamType::kF32, "max"},
        IntrinsicData{IntrinsicType::kMax, ParamType::kU32, "max"},
        IntrinsicData{IntrinsicType::kMin, ParamType::kF32, "min"},
        IntrinsicData{IntrinsicType::kMin, ParamType::kU32, "min"},
        IntrinsicData{IntrinsicType::kNormalize, ParamType::kF32, "normalize"},
        IntrinsicData{IntrinsicType::kPow, ParamType::kF32, "pow"},
        IntrinsicData{IntrinsicType::kReflect, ParamType::kF32, "reflect"},
        IntrinsicData{IntrinsicType::kReverseBits, ParamType::kU32,
                      "reversebits"},
        IntrinsicData{IntrinsicType::kRound, ParamType::kU32, "round"},
        IntrinsicData{IntrinsicType::kSign, ParamType::kF32, "sign"},
        IntrinsicData{IntrinsicType::kSin, ParamType::kF32, "sin"},
        IntrinsicData{IntrinsicType::kSinh, ParamType::kF32, "sinh"},
        IntrinsicData{IntrinsicType::kSmoothStep, ParamType::kF32,
                      "smoothstep"},
        IntrinsicData{IntrinsicType::kSqrt, ParamType::kF32, "sqrt"},
        IntrinsicData{IntrinsicType::kStep, ParamType::kF32, "step"},
        IntrinsicData{IntrinsicType::kTan, ParamType::kF32, "tan"},
        IntrinsicData{IntrinsicType::kTanh, ParamType::kF32, "tanh"},
        IntrinsicData{IntrinsicType::kTrunc, ParamType::kF32, "trunc"}));

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
