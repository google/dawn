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
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

std::string Preamble() {
  return R"(
  OpCapability Shader
  %glsl = OpExtInstImport "GLSL.std.450"
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %100 "main"
  OpExecutionMode %100 LocalSize 1 1 1

  OpName %u1 "u1"
  OpName %u2 "u2"
  OpName %u3 "u3"
  OpName %i1 "i1"
  OpName %i2 "i2"
  OpName %i3 "i3"
  OpName %f1 "f1"
  OpName %f2 "f2"
  OpName %f3 "f3"
  OpName %v2u1 "v2u1"
  OpName %v2u2 "v2u2"
  OpName %v2u3 "v2u3"
  OpName %v2i1 "v2i1"
  OpName %v2i2 "v2i2"
  OpName %v2i3 "v2i3"
  OpName %v2f1 "v2f1"
  OpName %v2f2 "v2f2"
  OpName %v2f3 "v2f3"
  OpName %v3f1 "v3f1"
  OpName %v3f2 "v3f2"
  OpName %v4f1 "v4f1"

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %uint_10 = OpConstant %uint 10
  %uint_15 = OpConstant %uint 15
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_35 = OpConstant %int 35
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
  %v3float = OpTypeVector %float 3
  %v4float = OpTypeVector %float 4

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2uint_15_15 = OpConstantComposite %v2uint %uint_15 %uint_15
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2int_35_35 = OpConstantComposite %v2int %int_35 %int_35
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2float_70_70 = OpConstantComposite %v2float %float_70 %float_70

  %v3float_50_60_70 = OpConstantComposite %v3float %float_50 %float_60 %float_70
  %v3float_60_70_50 = OpConstantComposite %v3float %float_60 %float_70 %float_50

  %v4float_50_50_50_50 = OpConstantComposite %v4float %float_50 %float_50 %float_50 %float_50

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  %u1 = OpCopyObject %uint %uint_10
  %u2 = OpCopyObject %uint %uint_15
  %u3 = OpCopyObject %uint %uint_20

  %i1 = OpCopyObject %int %int_30
  %i2 = OpCopyObject %int %int_35
  %i3 = OpCopyObject %int %int_40

  %f1 = OpCopyObject %float %float_50
  %f2 = OpCopyObject %float %float_60
  %f3 = OpCopyObject %float %float_70

  %v2u1 = OpCopyObject %v2uint %v2uint_10_20
  %v2u2 = OpCopyObject %v2uint %v2uint_20_10
  %v2u3 = OpCopyObject %v2uint %v2uint_15_15

  %v2i1 = OpCopyObject %v2int %v2int_30_40
  %v2i2 = OpCopyObject %v2int %v2int_40_30
  %v2i3 = OpCopyObject %v2int %v2int_35_35

  %v2f1 = OpCopyObject %v2float %v2float_50_60
  %v2f2 = OpCopyObject %v2float %v2float_60_50
  %v2f3 = OpCopyObject %v2float %v2float_70_70

  %v3f1 = OpCopyObject %v3float %v3float_50_60_70
  %v3f2 = OpCopyObject %v3float %v3float_60_70_50

  %v4f1 = OpCopyObject %v4float %v4float_50_50_50_50
)";
}

struct GlslStd450Case {
  std::string opcode;
  std::string wgsl_func;
};
inline std::ostream& operator<<(std::ostream& out, GlslStd450Case c) {
  out << "GlslStd450Case(" << c.opcode << " " << c.wgsl_func << ")";
  return out;
}

// Nomenclature:
// Float = scalar float
// Floating = scalar float or vector-of-float
// Float3 = 3-element vector of float
// Int = scalar signed int
// Inting = scalar int or vector-of-int
// Uint = scalar unsigned int
// Uinting = scalar unsigned or vector-of-unsigned

using SpvParserTest_GlslStd450_Float_Floating =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Float_FloatingFloating =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Floating_Floating =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Floating_FloatingFloating =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Floating_FloatingFloatingFloating =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Floating_FloatingInting =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Floating_FloatingUinting =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Float3_Float3Float3 =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;

using SpvParserTest_GlslStd450_Inting_Inting =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Inting_IntingInting =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Inting_IntingIntingInting =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Uinting_UintingUinting =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpvParserTest_GlslStd450_Uinting_UintingUintingUinting =
    SpvParserTestBase<::testing::TestWithParam<GlslStd450Case>>;

TEST_P(SpvParserTest_GlslStd450_Float_Floating, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{f1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Float_Floating, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %v2f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2f1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Float_FloatingFloating, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %f1 %f2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{f1}
          Identifier[not set]{f2}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Float_FloatingFloating, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %v2f1 %v2f2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2f1}
          Identifier[not set]{v2f2}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_Floating, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{f1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_Floating, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                        GetParam().opcode + R"( %v2f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2f1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingFloating, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %f1 %f2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{f1}
          Identifier[not set]{f2}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingFloating, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                        GetParam().opcode + R"( %v2f1 %v2f2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2f1}
          Identifier[not set]{v2f2}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingFloatingFloating, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %f1 %f2 %f3
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{f1}
          Identifier[not set]{f2}
          Identifier[not set]{f3}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingFloatingFloating, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                        GetParam().opcode +
                        R"( %v2f1 %v2f2 %v2f3
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2f1}
          Identifier[not set]{v2f2}
          Identifier[not set]{v2f3}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingUinting, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %f1 %u1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{f1}
          Identifier[not set]{u1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingUinting, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                        GetParam().opcode +
                        R"( %v2f1 %v2u1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2f1}
          Identifier[not set]{v2u1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingInting, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %f1 %i1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{f1}
          Identifier[not set]{i1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingInting, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                        GetParam().opcode +
                        R"( %v2f1 %v2i1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2f1}
          Identifier[not set]{v2i1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Float3_Float3Float3, Samples) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v3float %glsl )" +
                        GetParam().opcode +
                        R"( %v3f1 %v3f2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_3__f32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v3f1}
          Identifier[not set]{v3f2}
        )
      }
    }
  })"))
      << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Float_Floating,
                         ::testing::Values(GlslStd450Case{"Length", "length"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Float_FloatingFloating,
                         ::testing::Values(GlslStd450Case{"Distance",
                                                          "distance"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Floating_Floating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"Acos", "acos"},
                             {"Asin", "asin"},
                             {"Atan", "atan"},
                             {"Ceil", "ceil"},
                             {"Cos", "cos"},
                             {"Cosh", "cosh"},
                             {"Exp", "exp"},
                             {"Exp2", "exp2"},
                             {"FAbs", "abs"},
                             {"FSign", "sign"},
                             {"Floor", "floor"},
                             {"Fract", "fract"},
                             {"InverseSqrt", "inverseSqrt"},
                             {"Log", "log"},
                             {"Log2", "log2"},
                             {"Round", "round"},
                             {"RoundEven", "round"},
                             {"Sin", "sin"},
                             {"Sinh", "sinh"},
                             {"Sqrt", "sqrt"},
                             {"Tan", "tan"},
                             {"Tanh", "tanh"},
                             {"Trunc", "trunc"},
                         }));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Floating_FloatingFloating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"Atan2", "atan2"},
                             {"NMax", "max"},
                             {"NMin", "min"},
                             {"FMax", "max"},  // WGSL max promises more for NaN
                             {"FMin", "min"},  // WGSL min promises more for NaN
                             {"Pow", "pow"},
                             {"Reflect", "reflect"},
                             {"Step", "step"},
                         }));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Floating_FloatingUinting,
                         ::testing::Values(GlslStd450Case{"Ldexp", "ldexp"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Floating_FloatingInting,
                         ::testing::Values(GlslStd450Case{"Ldexp", "ldexp"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Float3_Float3Float3,
                         ::testing::Values(GlslStd450Case{"Cross", "cross"}));

INSTANTIATE_TEST_SUITE_P(
    Samples,
    SpvParserTest_GlslStd450_Floating_FloatingFloatingFloating,
    ::testing::ValuesIn(std::vector<GlslStd450Case>{
        {"NClamp", "clamp"},
        {"FClamp", "clamp"},  // WGSL FClamp promises more for NaN
        {"FaceForward", "faceForward"},
        {"Fma", "fma"},
        {"FMix", "mix"},
        {"SmoothStep", "smoothStep"}}));

TEST_P(SpvParserTest_GlslStd450_Inting_Inting, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                        GetParam().opcode +
                        R"( %i1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __i32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{i1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Inting_Inting, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                        GetParam().opcode +
                        R"( %v2i1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__i32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2i1}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Inting_IntingInting, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                        GetParam().opcode +
                        R"( %i1 %i2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __i32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{i1}
          Identifier[not set]{i2}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Inting_IntingInting, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                        GetParam().opcode +
                        R"( %v2i1 %v2i2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__i32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2i1}
          Identifier[not set]{v2i2}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Inting_IntingIntingInting, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                        GetParam().opcode +
                        R"( %i1 %i2 %i3
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __i32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{i1}
          Identifier[not set]{i2}
          Identifier[not set]{i3}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Inting_IntingIntingInting, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                        GetParam().opcode +
                        R"( %v2i1 %v2i2 %v2i3
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__i32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2i1}
          Identifier[not set]{v2i2}
          Identifier[not set]{v2i3}
        )
      }
    }
  })"))
      << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Inting_Inting,
                         ::testing::Values(GlslStd450Case{"SAbs", "abs"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Inting_IntingInting,
                         ::testing::Values(GlslStd450Case{"SMax", "max"},
                                           GlslStd450Case{"SMin", "min"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Inting_IntingIntingInting,
                         ::testing::Values(GlslStd450Case{"SClamp", "clamp"}));

TEST_P(SpvParserTest_GlslStd450_Uinting_UintingUinting, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                        GetParam().opcode + R"( %u1 %u2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __u32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{u1}
          Identifier[not set]{u2}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Uinting_UintingUinting, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                        GetParam().opcode +
                        R"( %v2u1 %v2u2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__u32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2u1}
          Identifier[not set]{v2u2}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Uinting_UintingUintingUinting, Scalar) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                        GetParam().opcode + R"( %u1 %u2 %u3
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __u32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{u1}
          Identifier[not set]{u2}
          Identifier[not set]{u3}
        )
      }
    }
  })"))
      << body;
}

TEST_P(SpvParserTest_GlslStd450_Uinting_UintingUintingUinting, Vector) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                        GetParam().opcode +
                        R"( %v2u1 %v2u2 %v2u3
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__u32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              GetParam().wgsl_func +
                              R"(}
        (
          Identifier[not set]{v2u1}
          Identifier[not set]{v2u2}
          Identifier[not set]{v2u3}
        )
      }
    }
  })"))
      << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Uinting_UintingUinting,
                         ::testing::Values(GlslStd450Case{"UMax", "max"},
                                           GlslStd450Case{"UMin", "min"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Uinting_UintingUintingUinting,
                         ::testing::Values(GlslStd450Case{"UClamp", "clamp"}));

// Test Normalize.  WGSL does not have a scalar form of the normalize builtin.
// So we have to test it separately, as it does not fit the patterns tested
// above.

TEST_F(SpvParserTest, Normalize_Scalar) {
  // Scalar normalize always results in 1.0
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl Normalize %f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __f32
    {
      ScalarConstructor[not set]{1.000000}
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, Normalize_Vector2) {
  // Scalar normalize always results in 1.0
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Normalize %v2f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_2__f32
    {
      Call[not set]{
        Identifier[not set]{normalize}
        (
          Identifier[not set]{v2f1}
        )
      }
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, Normalize_Vector3) {
  // Scalar normalize always results in 1.0
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v3float %glsl Normalize %v3f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_3__f32
    {
      Call[not set]{
        Identifier[not set]{normalize}
        (
          Identifier[not set]{v3f1}
        )
      }
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, Normalize_Vector4) {
  // Scalar normalize always results in 1.0
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v4float %glsl Normalize %v4f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __vec_4__f32
    {
      Call[not set]{
        Identifier[not set]{normalize}
        (
          Identifier[not set]{v4f1}
        )
      }
    }
  })"))
      << body;
}

// Check that we convert signedness of operands and result type.
// This is needed for each of the integer-based extended instructions.

TEST_F(SpvParserTest, RectifyOperandsAndResult_SAbs) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl SAbs %u1
     %2 = OpExtInst %v2uint %glsl SAbs %v2u1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __u32
    {
      Bitcast[not set]<__u32>{
        Call[not set]{
          Identifier[not set]{abs}
          (
            Bitcast[not set]<__i32>{
              Identifier[not set]{u1}
            }
          )
        }
      }
    }
  })"))
      << body;
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_2
    none
    __vec_2__u32
    {
      Bitcast[not set]<__vec_2__u32>{
        Call[not set]{
          Identifier[not set]{abs}
          (
            Bitcast[not set]<__vec_2__i32>{
              Identifier[not set]{v2u1}
            }
          )
        }
      }
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, RectifyOperandsAndResult_SMax) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl SMax %u1 %u2
     %2 = OpExtInst %v2uint %glsl SMax %v2u1 %v2u2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __u32
    {
      Bitcast[not set]<__u32>{
        Call[not set]{
          Identifier[not set]{max}
          (
            Bitcast[not set]<__i32>{
              Identifier[not set]{u1}
            }
            Bitcast[not set]<__i32>{
              Identifier[not set]{u2}
            }
          )
        }
      }
    }
  })"))
      << body;
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_2
    none
    __vec_2__u32
    {
      Bitcast[not set]<__vec_2__u32>{
        Call[not set]{
          Identifier[not set]{max}
          (
            Bitcast[not set]<__vec_2__i32>{
              Identifier[not set]{v2u1}
            }
            Bitcast[not set]<__vec_2__i32>{
              Identifier[not set]{v2u2}
            }
          )
        }
      }
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, RectifyOperandsAndResult_SMin) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl SMin %u1 %u2
     %2 = OpExtInst %v2uint %glsl SMin %v2u1 %v2u2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __u32
    {
      Bitcast[not set]<__u32>{
        Call[not set]{
          Identifier[not set]{min}
          (
            Bitcast[not set]<__i32>{
              Identifier[not set]{u1}
            }
            Bitcast[not set]<__i32>{
              Identifier[not set]{u2}
            }
          )
        }
      }
    }
  })"))
      << body;
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_2
    none
    __vec_2__u32
    {
      Bitcast[not set]<__vec_2__u32>{
        Call[not set]{
          Identifier[not set]{min}
          (
            Bitcast[not set]<__vec_2__i32>{
              Identifier[not set]{v2u1}
            }
            Bitcast[not set]<__vec_2__i32>{
              Identifier[not set]{v2u2}
            }
          )
        }
      }
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, RectifyOperandsAndResult_SClamp) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl SClamp %u1 %i2 %u3
     %2 = OpExtInst %v2uint %glsl SClamp %v2u1 %v2i2 %v2u3
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __u32
    {
      Bitcast[not set]<__u32>{
        Call[not set]{
          Identifier[not set]{clamp}
          (
            Bitcast[not set]<__i32>{
              Identifier[not set]{u1}
            }
            Identifier[not set]{i2}
            Bitcast[not set]<__i32>{
              Identifier[not set]{u3}
            }
          )
        }
      }
    }
  })"))
      << body;
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_2
    none
    __vec_2__u32
    {
      Bitcast[not set]<__vec_2__u32>{
        Call[not set]{
          Identifier[not set]{clamp}
          (
            Bitcast[not set]<__vec_2__i32>{
              Identifier[not set]{v2u1}
            }
            Identifier[not set]{v2i2}
            Bitcast[not set]<__vec_2__i32>{
              Identifier[not set]{v2u3}
            }
          )
        }
      }
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, RectifyOperandsAndResult_UMax) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl UMax %i1 %i2
     %2 = OpExtInst %v2int %glsl UMax %v2i1 %v2i2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __i32
    {
      Bitcast[not set]<__i32>{
        Call[not set]{
          Identifier[not set]{max}
          (
            Bitcast[not set]<__u32>{
              Identifier[not set]{i1}
            }
            Bitcast[not set]<__u32>{
              Identifier[not set]{i2}
            }
          )
        }
      }
    }
  })"))
      << body;
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_2
    none
    __vec_2__i32
    {
      Bitcast[not set]<__vec_2__i32>{
        Call[not set]{
          Identifier[not set]{max}
          (
            Bitcast[not set]<__vec_2__u32>{
              Identifier[not set]{v2i1}
            }
            Bitcast[not set]<__vec_2__u32>{
              Identifier[not set]{v2i2}
            }
          )
        }
      }
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, RectifyOperandsAndResult_UMin) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl UMin %i1 %i2
     %2 = OpExtInst %v2int %glsl UMin %v2i1 %v2i2
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __i32
    {
      Bitcast[not set]<__i32>{
        Call[not set]{
          Identifier[not set]{min}
          (
            Bitcast[not set]<__u32>{
              Identifier[not set]{i1}
            }
            Bitcast[not set]<__u32>{
              Identifier[not set]{i2}
            }
          )
        }
      }
    }
  })"))
      << body;
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_2
    none
    __vec_2__i32
    {
      Bitcast[not set]<__vec_2__i32>{
        Call[not set]{
          Identifier[not set]{min}
          (
            Bitcast[not set]<__vec_2__u32>{
              Identifier[not set]{v2i1}
            }
            Bitcast[not set]<__vec_2__u32>{
              Identifier[not set]{v2i2}
            }
          )
        }
      }
    }
  })"))
      << body;
}

TEST_F(SpvParserTest, RectifyOperandsAndResult_UClamp) {
  const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl UClamp %i1 %u2 %i3
     %2 = OpExtInst %v2int %glsl UClamp %v2i1 %v2u2 %v2i3
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __i32
    {
      Bitcast[not set]<__i32>{
        Call[not set]{
          Identifier[not set]{clamp}
          (
            Bitcast[not set]<__u32>{
              Identifier[not set]{i1}
            }
            Identifier[not set]{u2}
            Bitcast[not set]<__u32>{
              Identifier[not set]{i3}
            }
          )
        }
      }
    }
  })"))
      << body;
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_2
    none
    __vec_2__i32
    {
      Bitcast[not set]<__vec_2__i32>{
        Call[not set]{
          Identifier[not set]{clamp}
          (
            Bitcast[not set]<__vec_2__u32>{
              Identifier[not set]{v2i1}
            }
            Identifier[not set]{v2u2}
            Bitcast[not set]<__vec_2__u32>{
              Identifier[not set]{v2i3}
            }
          )
        }
      }
    }
  })"))
      << body;
}

struct DataPackingCase {
  std::string opcode;
  std::string wgsl_func;
  uint32_t vec_size;
};

inline std::ostream& operator<<(std::ostream& out, DataPackingCase c) {
  out << "DataPacking(" << c.opcode << ")";
  return out;
}

using SpvParserTest_GlslStd450_DataPacking =
    SpvParserTestBase<::testing::TestWithParam<DataPackingCase>>;

TEST_P(SpvParserTest_GlslStd450_DataPacking, Valid) {
  auto param = GetParam();
  const auto assembly = Preamble() + R"(
  %1 = OpExtInst %uint %glsl )" +
                        param.opcode +
                        (param.vec_size == 2 ? " %v2f1" : " %v4f1") + R"(
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    __u32
    {
      Call[not set]{
        Identifier[not set]{)" +
                              param.wgsl_func + R"(}
        (
          Identifier[not set]{v)" +
                              std::to_string(param.vec_size) + R"(f1}
        )
      }
    }
  })"))
      << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_DataPacking,
                         ::testing::ValuesIn(std::vector<DataPackingCase>{
                             {"PackSnorm4x8", "pack4x8snorm", 4},
                             {"PackUnorm4x8", "pack4x8unorm", 4},
                             {"PackSnorm2x16", "pack2x16snorm", 2},
                             {"PackUnorm2x16", "pack2x16unorm", 2},
                             {"PackHalf2x16", "pack2x16float", 2}}));

using SpvParserTest_GlslStd450_DataUnpacking =
    SpvParserTestBase<::testing::TestWithParam<DataPackingCase>>;

TEST_P(SpvParserTest_GlslStd450_DataUnpacking, Valid) {
  auto param = GetParam();
  const auto assembly = Preamble() + R"(
  %1 = OpExtInst )" + (param.vec_size == 2 ? "%v2float" : "%v4float") +
                        std::string(" %glsl ") + param.opcode + R"( %u1
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  const auto body = ToString(p->builder(), fe.ast_body());
  EXPECT_THAT(body, HasSubstr(R"(
  VariableConst{
    x_1
    none
    )" + std::string(param.vec_size == 2 ? "__vec_2__f32" : "__vec_4__f32") +
                              R"(
    {
      Call[not set]{
        Identifier[not set]{)" +
                              param.wgsl_func + R"(}
        (
          Identifier[not set]{u1}
        )
      }
    }
  })"))
      << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_DataUnpacking,
                         ::testing::ValuesIn(std::vector<DataPackingCase>{
                             {"UnpackSnorm4x8", "unpack4x8snorm", 4},
                             {"UnpackUnorm4x8", "unpack4x8unorm", 4},
                             {"UnpackSnorm2x16", "unpack2x16snorm", 2},
                             {"UnpackUnorm2x16", "unpack2x16unorm", 2},
                             {"UnpackHalf2x16", "unpack2x16float", 2}}));

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
