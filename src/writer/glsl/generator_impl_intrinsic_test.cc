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
#include "src/ast/call_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/sem/call.h"
#include "src/writer/glsl/test_helper.h"

namespace tint {
namespace writer {
namespace glsl {
namespace {

using IntrinsicType = sem::IntrinsicType;

using ::testing::HasSubstr;

using GlslGeneratorImplTest_Intrinsic = TestHelper;

enum class ParamType {
  kF32,
  kU32,
  kBool,
};

struct IntrinsicData {
  IntrinsicType intrinsic;
  ParamType type;
  const char* glsl_name;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
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

const ast::CallExpression* GenerateCall(IntrinsicType intrinsic,
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
      return builder->Call(str.str(), "f2", "i2");
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
    case IntrinsicType::kTranspose:
      return builder->Call(str.str(), "m3x2");
    default:
      break;
  }
  return nullptr;
}
using GlslIntrinsicTest = TestParamHelper<IntrinsicData>;
TEST_P(GlslIntrinsicTest, Emit) {
  auto param = GetParam();

  Global("f2", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  Global("f3", ty.vec3<f32>(), ast::StorageClass::kPrivate);
  Global("u2", ty.vec2<u32>(), ast::StorageClass::kPrivate);
  Global("i2", ty.vec2<i32>(), ast::StorageClass::kPrivate);
  Global("b2", ty.vec2<bool>(), ast::StorageClass::kPrivate);
  Global("m2x2", ty.mat2x2<f32>(), ast::StorageClass::kPrivate);
  Global("m3x2", ty.mat3x2<f32>(), ast::StorageClass::kPrivate);

  auto* call = GenerateCall(param.intrinsic, param.type, this);
  ASSERT_NE(nullptr, call) << "Unhandled intrinsic";
  Func("func", {}, ty.void_(), {CallStmt(call)},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  auto* sem = program->Sem().Get(call);
  ASSERT_NE(sem, nullptr);
  auto* target = sem->Target();
  ASSERT_NE(target, nullptr);
  auto* intrinsic = target->As<sem::Intrinsic>();
  ASSERT_NE(intrinsic, nullptr);

  EXPECT_EQ(gen.generate_builtin_name(intrinsic), param.glsl_name);
}
INSTANTIATE_TEST_SUITE_P(
    GlslGeneratorImplTest_Intrinsic,
    GlslIntrinsicTest,
    testing::Values(
        IntrinsicData{IntrinsicType::kAbs, ParamType::kF32, "abs"},
        IntrinsicData{IntrinsicType::kAbs, ParamType::kU32, "abs"},
        IntrinsicData{IntrinsicType::kAcos, ParamType::kF32, "acos"},
        IntrinsicData{IntrinsicType::kAll, ParamType::kBool, "all"},
        IntrinsicData{IntrinsicType::kAny, ParamType::kBool, "any"},
        IntrinsicData{IntrinsicType::kAsin, ParamType::kF32, "asin"},
        IntrinsicData{IntrinsicType::kAtan, ParamType::kF32, "atan"},
        IntrinsicData{IntrinsicType::kAtan2, ParamType::kF32, "atan"},
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
        IntrinsicData{IntrinsicType::kFma, ParamType::kF32, "mad"},
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
        IntrinsicData{IntrinsicType::kMix, ParamType::kF32, "mix"},
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
        IntrinsicData{IntrinsicType::kTranspose, ParamType::kF32, "transpose"},
        IntrinsicData{IntrinsicType::kTrunc, ParamType::kF32, "trunc"}));

TEST_F(GlslGeneratorImplTest_Intrinsic, DISABLED_Intrinsic_IsNormal) {
  FAIL();
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Intrinsic_Call) {
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

TEST_F(GlslGeneratorImplTest_Intrinsic, Select_Scalar) {
  auto* call = Call("select", 1.0f, 2.0f, true);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_EQ(out.str(), "(true ? 2.0f : 1.0f)");
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Select_Vector) {
  auto* call =
      Call("select", vec2<i32>(1, 2), vec2<i32>(3, 4), vec2<bool>(true, false));
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_EQ(out.str(), "(bvec2(true, false) ? ivec2(3, 4) : ivec2(1, 2))");
}

#if 0
TEST_F(GlslGeneratorImplTest_Intrinsic, Modf_Scalar) {
  auto* res = Var("res", ty.f32());
  auto* call = Call("modf", 1.0f, AddressOf(res));
  WrapInFunction(res, call);

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("modf(1.0f, res)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Modf_Vector) {
  auto* res = Var("res", ty.vec3<f32>());
  auto* call = Call("modf", vec3<f32>(), AddressOf(res));
  WrapInFunction(res, call);

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("modf(vec3(0.0f, 0.0f, 0.0f), res)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Frexp_Scalar_i32) {
  auto* exp = Var("exp", ty.i32());
  auto* call = Call("frexp", 1.0f, AddressOf(exp));
  WrapInFunction(exp, call);

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr(R"(
  float tint_tmp;
  float tint_tmp_1 = frexp(1.0f, tint_tmp);
  exp = int(tint_tmp);
  tint_tmp_1;
)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Frexp_Vector_i32) {
  auto* res = Var("res", ty.vec3<i32>());
  auto* call = Call("frexp", vec3<f32>(), AddressOf(res));
  WrapInFunction(res, call);

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr(R"(
  vec3 tint_tmp;
  vec3 tint_tmp_1 = frexp(vec3(0.0f, 0.0f, 0.0f), tint_tmp);
  res = ivec3(tint_tmp);
  tint_tmp_1;
)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, IsNormal_Scalar) {
  auto* val = Var("val", ty.f32());
  auto* call = Call("isNormal", val);
  WrapInFunction(val, call);

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr(R"(
  uint tint_isnormal_exponent = asuint(val) & 0x7f80000;
  uint tint_isnormal_clamped = clamp(tint_isnormal_exponent, 0x0080000, 0x7f00000);
  (tint_isnormal_clamped == tint_isnormal_exponent);
)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, IsNormal_Vector) {
  auto* val = Var("val", ty.vec3<f32>());
  auto* call = Call("isNormal", val);
  WrapInFunction(val, call);

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr(R"(
  uvec3 tint_isnormal_exponent = asuint(val) & 0x7f80000;
  uvec3 tint_isnormal_clamped = clamp(tint_isnormal_exponent, 0x0080000, 0x7f00000);
  (tint_isnormal_clamped == tint_isnormal_exponent);
)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Pack4x8Snorm) {
  auto* call = Call("pack4x8snorm", "p1");
  Global("p1", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("ivec4 tint_tmp = ivec4(round(clamp(p1, "
                                      "-1.0, 1.0) * 127.0)) & 0xff;"));
  EXPECT_THAT(out.str(), HasSubstr("asuint(tint_tmp.x | tint_tmp.y << 8 | "
                                   "tint_tmp.z << 16 | tint_tmp.w << 24)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Pack4x8Unorm) {
  auto* call = Call("pack4x8unorm", "p1");
  Global("p1", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("uvec4 tint_tmp = uvec4(round(clamp(p1, "
                                      "0.0, 1.0) * 255.0));"));
  EXPECT_THAT(out.str(), HasSubstr("(tint_tmp.x | tint_tmp.y << 8 | "
                                   "tint_tmp.z << 16 | tint_tmp.w << 24)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Pack2x16Snorm) {
  auto* call = Call("pack2x16snorm", "p1");
  Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("int2 tint_tmp = int2(round(clamp(p1, "
                                      "-1.0, 1.0) * 32767.0)) & 0xffff;"));
  EXPECT_THAT(out.str(), HasSubstr("asuint(tint_tmp.x | tint_tmp.y << 16)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Pack2x16Unorm) {
  auto* call = Call("pack2x16unorm", "p1");
  Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("uint2 tint_tmp = uint2(round(clamp(p1, "
                                      "0.0, 1.0) * 65535.0));"));
  EXPECT_THAT(out.str(), HasSubstr("(tint_tmp.x | tint_tmp.y << 16)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Pack2x16Float) {
  auto* call = Call("pack2x16float", "p1");
  Global("p1", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("uint2 tint_tmp = f32tof16(p1);"));
  EXPECT_THAT(out.str(), HasSubstr("(tint_tmp.x | tint_tmp.y << 16)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Unpack4x8Snorm) {
  auto* call = Call("unpack4x8snorm", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("int tint_tmp_1 = int(p1);"));
  EXPECT_THAT(gen.result(),
              HasSubstr("ivec4 tint_tmp = ivec4(tint_tmp_1 << 24, tint_tmp_1 "
                        "<< 16, tint_tmp_1 << 8, tint_tmp_1) >> 24;"));
  EXPECT_THAT(out.str(),
              HasSubstr("clamp(float4(tint_tmp) / 127.0, -1.0, 1.0)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Unpack4x8Unorm) {
  auto* call = Call("unpack4x8unorm", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("uint tint_tmp_1 = p1;"));
  EXPECT_THAT(
      gen.result(),
      HasSubstr("uvec4 tint_tmp = uvec4(tint_tmp_1 & 0xff, (tint_tmp_1 >> "
                "8) & 0xff, (tint_tmp_1 >> 16) & 0xff, tint_tmp_1 >> 24);"));
  EXPECT_THAT(out.str(), HasSubstr("float4(tint_tmp) / 255.0"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Unpack2x16Snorm) {
  auto* call = Call("unpack2x16snorm", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("int tint_tmp_1 = int(p1);"));
  EXPECT_THAT(
      gen.result(),
      HasSubstr("int2 tint_tmp = int2(tint_tmp_1 << 16, tint_tmp_1) >> 16;"));
  EXPECT_THAT(out.str(),
              HasSubstr("clamp(float2(tint_tmp) / 32767.0, -1.0, 1.0)"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Unpack2x16Unorm) {
  auto* call = Call("unpack2x16unorm", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("uint tint_tmp_1 = p1;"));
  EXPECT_THAT(gen.result(),
              HasSubstr("uint2 tint_tmp = uint2(tint_tmp_1 & 0xffff, "
                        "tint_tmp_1 >> 16);"));
  EXPECT_THAT(out.str(), HasSubstr("float2(tint_tmp) / 65535.0"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Unpack2x16Float) {
  auto* call = Call("unpack2x16float", "p1");
  Global("p1", ty.u32(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(call));
  GeneratorImpl& gen = Build();

  gen.increment_indent();
  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.error();
  EXPECT_THAT(gen.result(), HasSubstr("uint tint_tmp = p1;"));
  EXPECT_THAT(out.str(),
              HasSubstr("f16tof32(uint2(tint_tmp & 0xffff, tint_tmp >> 16))"));
}

TEST_F(GlslGeneratorImplTest_Intrinsic, StorageBarrier) {
  Func("main", {}, ty.void_(),
       {CallStmt(Call("storageBarrier"))},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(1),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  DeviceMemoryBarrierWithGroupSync();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Intrinsic, WorkgroupBarrier) {
  Func("main", {}, ty.void_(),
       {CallStmt(Call("workgroupBarrier"))},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(1),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  GroupMemoryBarrierWithGroupSync();
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Intrinsic, Ignore) {
  Func("f", {Param("a", ty.i32()), Param("b", ty.i32()), Param("c", ty.i32())},
       ty.i32(), {Return(Mul(Add("a", "b"), "c"))});

  Func("main", {}, ty.void_(),
       {CallStmt(Call("ignore", Call("f", 1, 2, 3)))},
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(1),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(int f(int a, int b, int c) {
  return ((a + b) * c);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(1, 2, 3);
  return;
}
)");
}
#endif

TEST_F(GlslGeneratorImplTest_Intrinsic, DotI32) {
  Global("v", ty.vec3<i32>(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(Call("dot", "v", "v")));

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

int tint_int_dot(ivec3 a, ivec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

ivec3 v = ivec3(0, 0, 0);

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  tint_int_dot(v, v);
  return;
}
void main() {
  test_function();
}


)");
}

TEST_F(GlslGeneratorImplTest_Intrinsic, DotU32) {
  Global("v", ty.vec3<u32>(), ast::StorageClass::kPrivate);
  WrapInFunction(CallStmt(Call("dot", "v", "v")));

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

uint tint_int_dot(uvec3 a, uvec3 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

uvec3 v = uvec3(0u, 0u, 0u);

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  tint_int_dot(v, v);
  return;
}
void main() {
  test_function();
}


)");
}

}  // namespace
}  // namespace glsl
}  // namespace writer
}  // namespace tint
