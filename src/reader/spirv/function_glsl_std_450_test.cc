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

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl.h"
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

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2float_70_70 = OpConstantComposite %v2float %float_70 %float_70
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

TEST_P(SpvParserTest_GlslStd450_Float_Floating, Scalar) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %float_50
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          ScalarConstructor{50.000000}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Float_Floating, Vector) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %v2float_50_60
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{50.000000}
            ScalarConstructor{60.000000}
          }
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Float_FloatingFloating, Scalar) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %float_50 %float_60
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          ScalarConstructor{50.000000}
          ScalarConstructor{60.000000}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Float_FloatingFloating, Vector) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %v2float_50_60 %v2float_60_50
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{50.000000}
            ScalarConstructor{60.000000}
          }
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{60.000000}
            ScalarConstructor{50.000000}
          }
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Floating_Floating, Scalar) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %float_50
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          ScalarConstructor{50.000000}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Floating_Floating, Vector) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %v2float %glsl )" +
                        GetParam().opcode + R"( %v2float_50_60
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __vec_2__f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{50.000000}
            ScalarConstructor{60.000000}
          }
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingFloating, Scalar) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %float_50 %float_60
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          ScalarConstructor{50.000000}
          ScalarConstructor{60.000000}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingFloating, Vector) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %v2float %glsl )" +
                        GetParam().opcode + R"( %v2float_50_60 %v2float_60_50
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __vec_2__f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{50.000000}
            ScalarConstructor{60.000000}
          }
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{60.000000}
            ScalarConstructor{50.000000}
          }
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingFloatingFloating, Scalar) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %float %glsl )" +
                        GetParam().opcode + R"( %float_50 %float_60 %float_70
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          ScalarConstructor{50.000000}
          ScalarConstructor{60.000000}
          ScalarConstructor{70.000000}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_P(SpvParserTest_GlslStd450_Floating_FloatingFloatingFloating, Vector) {
  const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpExtInst %v2float %glsl )" +
                        GetParam().opcode +
                        R"( %v2float_50_60 %v2float_60_50 %v2float_70_70
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __vec_2__f32
    {
      Call{
        Identifier{)" + GetParam().wgsl_func + R"(}
        (
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{50.000000}
            ScalarConstructor{60.000000}
          }
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{60.000000}
            ScalarConstructor{50.000000}
          }
          TypeConstructor{
            __vec_2__f32
            ScalarConstructor{70.000000}
            ScalarConstructor{70.000000}
          }
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Float_Floating,
                         ::testing::Values(GlslStd450Case{
                             "Length", "std::glsl::length"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Float_FloatingFloating,
                         ::testing::Values(GlslStd450Case{
                             "Distance", "std::glsl::distance"}));

INSTANTIATE_TEST_SUITE_P(
    Samples,
    SpvParserTest_GlslStd450_Floating_Floating,
    ::testing::Values(GlslStd450Case{"Sin", "std::glsl::sin"},
                      GlslStd450Case{"Cos", "std::glsl::cos"},
                      GlslStd450Case{"Normalize", "std::glsl::normalize"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpvParserTest_GlslStd450_Floating_FloatingFloating,
                         ::testing::Values(GlslStd450Case{"Atan2",
                                                          "std::glsl::atan2"}));

INSTANTIATE_TEST_SUITE_P(
    Samples,
    SpvParserTest_GlslStd450_Floating_FloatingFloatingFloating,
    ::testing::Values(GlslStd450Case{"FClamp", "std::glsl::fclamp"}));

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
