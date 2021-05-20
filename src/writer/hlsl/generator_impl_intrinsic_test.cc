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
#include "src/ast/call_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/sem/call.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using IntrinsicType = sem::IntrinsicType;

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
    default:
      break;
  }
  return nullptr;
}
using HlslIntrinsicTest = TestParamHelper<IntrinsicData>;
TEST_P(HlslIntrinsicTest, Emit) {
  auto param = GetParam();

  Global("f2", ty.vec2<float>(), ast::StorageClass::kPrivate);
  Global("f3", ty.vec3<float>(), ast::StorageClass::kPrivate);
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
        IntrinsicData{IntrinsicType::kMix, ParamType::kF32, "lerp"},
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

  Global("param1", ty.vec3<f32>(), ast::StorageClass::kPrivate);
  Global("param2", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  WrapInFunction(call);

  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_EQ(result(), "dot(param1, param2)");
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack4x8Snorm) {
  auto* call = Call("pack4x8snorm", "p1");
  Global("p1", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("int4 tint_tmp = int4(round(clamp(p1, "
                                      "-1.0, 1.0) * 127.0)) & 0xff;"));
  EXPECT_THAT(result(), HasSubstr("asuint(tint_tmp.x | tint_tmp.y << 8 | "
                                  "tint_tmp.z << 16 | tint_tmp.w << 24)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack4x8Unorm) {
  auto* call = Call("pack4x8unorm", "p1");
  Global("p1", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint4 tint_tmp = uint4(round(clamp(p1, "
                                      "0.0, 1.0) * 255.0));"));
  EXPECT_THAT(result(), HasSubstr("(tint_tmp.x | tint_tmp.y << 8 | "
                                  "tint_tmp.z << 16 | tint_tmp.w << 24)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack2x16Snorm) {
  auto* call = Call("pack2x16snorm", "p1");
  Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("int2 tint_tmp = int2(round(clamp(p1, "
                                      "-1.0, 1.0) * 32767.0)) & 0xffff;"));
  EXPECT_THAT(result(), HasSubstr("asuint(tint_tmp.x | tint_tmp.y << 16)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack2x16Unorm) {
  auto* call = Call("pack2x16unorm", "p1");
  Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint2 tint_tmp = uint2(round(clamp(p1, "
                                      "0.0, 1.0) * 65535.0));"));
  EXPECT_THAT(result(), HasSubstr("(tint_tmp.x | tint_tmp.y << 16)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Pack2x16Float) {
  auto* call = Call("pack2x16float", "p1");
  Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint2 tint_tmp = f32tof16(p1);"));
  EXPECT_THAT(result(), HasSubstr("(tint_tmp.x | tint_tmp.y << 16)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Unpack4x8Snorm) {
  auto* call = Call("unpack4x8snorm", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("int tint_tmp_1 = int(p1);"));
  EXPECT_THAT(pre_result(),
              HasSubstr("int4 tint_tmp = int4(tint_tmp_1 << 24, tint_tmp_1 "
                        "<< 16, tint_tmp_1 << 8, tint_tmp_1) >> 24;"));
  EXPECT_THAT(result(),
              HasSubstr("clamp(float4(tint_tmp) / 127.0, -1.0, 1.0)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Unpack4x8Unorm) {
  auto* call = Call("unpack4x8unorm", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint tint_tmp_1 = p1;"));
  EXPECT_THAT(
      pre_result(),
      HasSubstr("uint4 tint_tmp = uint4(tint_tmp_1 & 0xff, (tint_tmp_1 >> "
                "8) & 0xff, (tint_tmp_1 >> 16) & 0xff, tint_tmp_1 >> 24);"));
  EXPECT_THAT(result(), HasSubstr("float4(tint_tmp) / 255.0"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Unpack2x16Snorm) {
  auto* call = Call("unpack2x16snorm", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("int tint_tmp_1 = int(p1);"));
  EXPECT_THAT(
      pre_result(),
      HasSubstr("int2 tint_tmp = int2(tint_tmp_1 << 16, tint_tmp_1) >> 16;"));
  EXPECT_THAT(result(),
              HasSubstr("clamp(float2(tint_tmp) / 32767.0, -1.0, 1.0)"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Unpack2x16Unorm) {
  auto* call = Call("unpack2x16unorm", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint tint_tmp_1 = p1;"));
  EXPECT_THAT(
      pre_result(),
      HasSubstr(
          "uint2 tint_tmp = uint2(tint_tmp_1 & 0xffff, tint_tmp_1 >> 16);"));
  EXPECT_THAT(result(), HasSubstr("float2(tint_tmp) / 65535.0"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, Unpack2x16Float) {
  auto* call = Call("unpack2x16float", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(call);
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  ASSERT_TRUE(gen.EmitExpression(pre, out, call)) << gen.error();
  EXPECT_THAT(pre_result(), HasSubstr("uint tint_tmp = p1;"));
  EXPECT_THAT(result(),
              HasSubstr("f16tof32(uint2(tint_tmp & 0xffff, tint_tmp >> 16))"));
}

TEST_F(HlslGeneratorImplTest_Intrinsic, StorageBarrier) {
  Func("main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("storageBarrier"))},
       {Stage(ast::PipelineStage::kCompute)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(1, 1, 1)]
void main() {
  DeviceMemoryBarrierWithGroupSync();
  return;
}

)");

  Validate();
}

TEST_F(HlslGeneratorImplTest_Intrinsic, WorkgroupBarrier) {
  Func("main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("workgroupBarrier"))},
       {Stage(ast::PipelineStage::kCompute)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(1, 1, 1)]
void main() {
  GroupMemoryBarrierWithGroupSync();
  return;
}

)");

  Validate();
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
