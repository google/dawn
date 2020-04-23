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
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

std::string CommonTypes() {
  return R"(
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %bool = OpTypeBool
  %true = OpConstantTrue %bool
  %false = OpConstantFalse %bool

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60

  %ptr_uint = OpTypePointer Function %uint
  %ptr_int = OpTypePointer Function %int
  %ptr_float = OpTypePointer Function %float

  %v2bool = OpTypeVector %bool 2
  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2

  %v2bool_t_f = OpConstantComposite %v2bool %true %false
  %v2bool_f_t = OpConstantComposite %v2bool %false %true
  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
)";
}

// Returns the AST dump for a given SPIR-V assembly constant.
std::string AstFor(std::string assembly) {
  if (assembly == "v2bool_t_f") {
    return R"(TypeConstructor{
          __vec_2__bool
          ScalarConstructor{true}
          ScalarConstructor{false}
        })";
  }
  if (assembly == "v2bool_f_t") {
    return R"(TypeConstructor{
          __vec_2__bool
          ScalarConstructor{false}
          ScalarConstructor{true}
        })";
  }
  if (assembly == "v2uint_10_20") {
    return R"(TypeConstructor{
          __vec_2__u32
          ScalarConstructor{10}
          ScalarConstructor{20}
        })";
  }
  if (assembly == "v2uint_20_10") {
    return R"(TypeConstructor{
          __vec_2__u32
          ScalarConstructor{20}
          ScalarConstructor{10}
        })";
  }
  if (assembly == "cast_int_30") {
    return R"(As<__u32>{
          ScalarConstructor{30}
        })";
  }
  if (assembly == "cast_int_40") {
    return R"(As<__u32>{
          ScalarConstructor{40}
        })";
  }
  if (assembly == "v2int_30_40") {
    return R"(TypeConstructor{
          __vec_2__i32
          ScalarConstructor{30}
          ScalarConstructor{40}
        })";
  }
  if (assembly == "cast_v2int_30_40") {
    return R"(As<__vec_2__u32>{
          TypeConstructor{
            __vec_2__i32
            ScalarConstructor{30}
            ScalarConstructor{40}
          }
        })";
  }
  if (assembly == "v2int_40_30") {
    return R"(TypeConstructor{
          __vec_2__i32
          ScalarConstructor{40}
          ScalarConstructor{30}
        })";
  }
  if (assembly == "cast_v2int_40_30") {
    return R"(As<__vec_2__u32>{
          TypeConstructor{
            __vec_2__i32
            ScalarConstructor{40}
            ScalarConstructor{30}
          }
        })";
  }
  if (assembly == "v2int_40_30") {
    return R"(TypeConstructor{
          __vec_2__i32
          ScalarConstructor{40}
          ScalarConstructor{30}
        })";
  }
  if (assembly == "v2float_50_60") {
    return R"(TypeConstructor{
          __vec_2__f32
          ScalarConstructor{50.000000}
          ScalarConstructor{60.000000}
        })";
  }
  if (assembly == "v2float_60_50") {
    return R"(TypeConstructor{
          __vec_2__f32
          ScalarConstructor{60.000000}
          ScalarConstructor{50.000000}
        })";
  }
  return "bad case";
}

using SpvUnaryLogicalTest = SpvParserTestBase<::testing::Test>;

TEST_F(SpvUnaryLogicalTest, LogicalNot_Scalar) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpLogicalNot %bool %true
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __bool
    {
      UnaryOp{
        not
        ScalarConstructor{true}
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryLogicalTest, LogicalNot_Vector) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpLogicalNot %v2bool %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(
  Variable{
    x_1
    none
    __vec_2__bool
    {
      UnaryOp{
        not
        TypeConstructor{
          __vec_2__bool
          ScalarConstructor{true}
          ScalarConstructor{false}
        }
      }
    }
  })"))
      << ToString(fe.ast_body());
}

struct BinaryData {
  const std::string res_type;
  const std::string lhs;
  const std::string op;
  const std::string rhs;
  const std::string ast_type;
  const std::string ast_lhs;
  const std::string ast_op;
  const std::string ast_rhs;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
  out << "BinaryData{" << data.res_type << "," << data.lhs << "," << data.op
      << "," << data.rhs << "," << data.ast_type << "," << data.ast_lhs << ","
      << data.ast_op << "," << data.ast_rhs << "}";
  return out;
}

using SpvBinaryLogicalTest =
    SpvParserTestBase<::testing::TestWithParam<BinaryData>>;

TEST_P(SpvBinaryLogicalTest, EmitExpression) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = )" + GetParam().op +
                        " %" + GetParam().res_type + " %" + GetParam().lhs +
                        " %" + GetParam().rhs + R"(
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << p->error() << "\n"
      << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  std::ostringstream ss;
  ss << R"(Variable{
    x_1
    none
    )"
     << GetParam().ast_type << "\n    {\n      Binary{"
     << "\n        " << GetParam().ast_lhs << "\n        " << GetParam().ast_op
     << "\n        " << GetParam().ast_rhs;
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(ss.str())) << assembly;
}

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_IEqual,
    SpvBinaryLogicalTest,
    ::testing::Values(
        // Both uint
        BinaryData{"bool", "uint_10", "OpIEqual", "uint_20", "__bool",
                   "ScalarConstructor{10}", "equal", "ScalarConstructor{20}"},
        // Both int
        BinaryData{"bool", "int_30", "OpIEqual", "int_40", "__bool",
                   "ScalarConstructor{30}", "equal", "ScalarConstructor{40}"},
        // Both v2uint
        BinaryData{"v2bool", "v2uint_10_20", "OpIEqual", "v2uint_20_10",
                   "__vec_2__bool", AstFor("v2uint_10_20"), "equal",
                   AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2bool", "v2int_30_40", "OpIEqual", "v2int_40_30",
                   "__vec_2__bool", AstFor("v2int_30_40"), "equal",
                   AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_FOrdEqual,
    SpvBinaryLogicalTest,
    ::testing::Values(BinaryData{"bool", "float_50", "OpFOrdEqual", "float_60",
                                 "__bool", "ScalarConstructor{50.000000}",
                                 "equal", "ScalarConstructor{60.000000}"},
                      BinaryData{"v2bool", "v2float_50_60", "OpFOrdEqual",
                                 "v2float_60_50", "__vec_2__bool",
                                 AstFor("v2float_50_60"), "equal",
                                 AstFor("v2float_60_50")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_INotEqual,
    SpvBinaryLogicalTest,
    ::testing::Values(
        // Both uint
        BinaryData{"bool", "uint_10", "OpINotEqual", "uint_20", "__bool",
                   "ScalarConstructor{10}", "not_equal",
                   "ScalarConstructor{20}"},
        // Both int
        BinaryData{"bool", "int_30", "OpINotEqual", "int_40", "__bool",
                   "ScalarConstructor{30}", "not_equal",
                   "ScalarConstructor{40}"},
        // Both v2uint
        BinaryData{"v2bool", "v2uint_10_20", "OpINotEqual", "v2uint_20_10",
                   "__vec_2__bool", AstFor("v2uint_10_20"), "not_equal",
                   AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2bool", "v2int_30_40", "OpINotEqual", "v2int_40_30",
                   "__vec_2__bool", AstFor("v2int_30_40"), "not_equal",
                   AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_FOrdNotEqual,
    SpvBinaryLogicalTest,
    ::testing::Values(BinaryData{"bool", "float_50", "OpFOrdNotEqual",
                                 "float_60", "__bool",
                                 "ScalarConstructor{50.000000}", "not_equal",
                                 "ScalarConstructor{60.000000}"},
                      BinaryData{"v2bool", "v2float_50_60", "OpFOrdNotEqual",
                                 "v2float_60_50", "__vec_2__bool",
                                 AstFor("v2float_50_60"), "not_equal",
                                 AstFor("v2float_60_50")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_LogicalAnd,
    SpvBinaryLogicalTest,
    ::testing::Values(BinaryData{"bool", "true", "OpLogicalAnd", "false",
                                 "__bool", "ScalarConstructor{true}",
                                 "logical_and", "ScalarConstructor{false}"},
                      BinaryData{"v2bool", "v2bool_t_f", "OpLogicalAnd",
                                 "v2bool_f_t", "__vec_2__bool",
                                 AstFor("v2bool_t_f"), "logical_and",
                                 AstFor("v2bool_f_t")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_LogicalOr,
    SpvBinaryLogicalTest,
    ::testing::Values(BinaryData{"bool", "true", "OpLogicalOr", "false",
                                 "__bool", "ScalarConstructor{true}",
                                 "logical_or", "ScalarConstructor{false}"},
                      BinaryData{"v2bool", "v2bool_t_f", "OpLogicalOr",
                                 "v2bool_f_t", "__vec_2__bool",
                                 AstFor("v2bool_t_f"), "logical_or",
                                 AstFor("v2bool_f_t")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_LogicalEqual,
    SpvBinaryLogicalTest,
    ::testing::Values(BinaryData{"bool", "true", "OpLogicalEqual", "false",
                                 "__bool", "ScalarConstructor{true}", "equal",
                                 "ScalarConstructor{false}"},
                      BinaryData{"v2bool", "v2bool_t_f", "OpLogicalEqual",
                                 "v2bool_f_t", "__vec_2__bool",
                                 AstFor("v2bool_t_f"), "equal",
                                 AstFor("v2bool_f_t")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_LogicalNotEqual,
    SpvBinaryLogicalTest,
    ::testing::Values(BinaryData{"bool", "true", "OpLogicalNotEqual", "false",
                                 "__bool", "ScalarConstructor{true}",
                                 "not_equal", "ScalarConstructor{false}"},
                      BinaryData{"v2bool", "v2bool_t_f", "OpLogicalNotEqual",
                                 "v2bool_f_t", "__vec_2__bool",
                                 AstFor("v2bool_t_f"), "not_equal",
                                 AstFor("v2bool_f_t")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_UGreaterThan,
    SpvBinaryLogicalTest,
    ::testing::Values(
        // Both unsigned
        BinaryData{"bool", "uint_10", "OpUGreaterThan", "uint_20", "__bool",
                   "ScalarConstructor{10}", "greater_than",
                   "ScalarConstructor{20}"},
        // First arg signed
        BinaryData{"bool", "int_30", "OpUGreaterThan", "uint_20", "__bool",
                   AstFor("cast_int_30"), "greater_than",
                   "ScalarConstructor{20}"},
        // Second arg signed
        BinaryData{"bool", "uint_10", "OpUGreaterThan", "int_40", "__bool",
                   "ScalarConstructor{10}", "greater_than",
                   AstFor("cast_int_40")},
        // Vector, both unsigned
        BinaryData{"v2bool", "v2uint_10_20", "OpUGreaterThan", "v2uint_20_10",
                   "__vec_2__bool", AstFor("v2uint_10_20"), "greater_than",
                   AstFor("v2uint_20_10")},
        // First arg signed
        BinaryData{"v2bool", "v2int_30_40", "OpUGreaterThan", "v2uint_20_10",
                   "__vec_2__bool", AstFor("cast_v2int_30_40"), "greater_than",
                   AstFor("v2uint_20_10")},
        // Second arg signed
        BinaryData{"v2bool", "v2uint_10_20", "OpUGreaterThan", "v2int_40_30",
                   "__vec_2__bool", AstFor("v2uint_10_20"), "greater_than",
                   AstFor("cast_v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_UGreaterThanEqual,
    SpvBinaryLogicalTest,
    ::testing::Values(
        // Both unsigned
        BinaryData{"bool", "uint_10", "OpUGreaterThanEqual", "uint_20",
                   "__bool", "ScalarConstructor{10}", "greater_than_equal",
                   "ScalarConstructor{20}"},
        // First arg signed
        BinaryData{"bool", "int_30", "OpUGreaterThanEqual", "uint_20", "__bool",
                   AstFor("cast_int_30"), "greater_than_equal",
                   "ScalarConstructor{20}"},
        // Second arg signed
        BinaryData{"bool", "uint_10", "OpUGreaterThanEqual", "int_40", "__bool",
                   "ScalarConstructor{10}", "greater_than_equal",
                   AstFor("cast_int_40")},
        // Vector, both unsigned
        BinaryData{"v2bool", "v2uint_10_20", "OpUGreaterThanEqual",
                   "v2uint_20_10", "__vec_2__bool", AstFor("v2uint_10_20"),
                   "greater_than_equal", AstFor("v2uint_20_10")},
        // First arg signed
        BinaryData{"v2bool", "v2int_30_40", "OpUGreaterThanEqual",
                   "v2uint_20_10", "__vec_2__bool", AstFor("cast_v2int_30_40"),
                   "greater_than_equal", AstFor("v2uint_20_10")},
        // Second arg signed
        BinaryData{"v2bool", "v2uint_10_20", "OpUGreaterThanEqual",
                   "v2int_40_30", "__vec_2__bool", AstFor("v2uint_10_20"),
                   "greater_than_equal", AstFor("cast_v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_ULessThan,
    SpvBinaryLogicalTest,
    ::testing::Values(
        // Both unsigned
        BinaryData{"bool", "uint_10", "OpULessThan", "uint_20", "__bool",
                   "ScalarConstructor{10}", "less_than",
                   "ScalarConstructor{20}"},
        // First arg signed
        BinaryData{"bool", "int_30", "OpULessThan", "uint_20", "__bool",
                   AstFor("cast_int_30"), "less_than", "ScalarConstructor{20}"},
        // Second arg signed
        BinaryData{"bool", "uint_10", "OpULessThan", "int_40", "__bool",
                   "ScalarConstructor{10}", "less_than", AstFor("cast_int_40")},
        // Vector, both unsigned
        BinaryData{"v2bool", "v2uint_10_20", "OpULessThan", "v2uint_20_10",
                   "__vec_2__bool", AstFor("v2uint_10_20"), "less_than",
                   AstFor("v2uint_20_10")},
        // First arg signed
        BinaryData{"v2bool", "v2int_30_40", "OpULessThan", "v2uint_20_10",
                   "__vec_2__bool", AstFor("cast_v2int_30_40"), "less_than",
                   AstFor("v2uint_20_10")},
        // Second arg signed
        BinaryData{"v2bool", "v2uint_10_20", "OpULessThan", "v2int_40_30",
                   "__vec_2__bool", AstFor("v2uint_10_20"), "less_than",
                   AstFor("cast_v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_ULessThanEqual,
    SpvBinaryLogicalTest,
    ::testing::Values(
        // Both unsigned
        BinaryData{"bool", "uint_10", "OpULessThanEqual", "uint_20", "__bool",
                   "ScalarConstructor{10}", "less_than_equal",
                   "ScalarConstructor{20}"},
        // First arg signed
        BinaryData{"bool", "int_30", "OpULessThanEqual", "uint_20", "__bool",
                   AstFor("cast_int_30"), "less_than_equal",
                   "ScalarConstructor{20}"},
        // Second arg signed
        BinaryData{"bool", "uint_10", "OpULessThanEqual", "int_40", "__bool",
                   "ScalarConstructor{10}", "less_than_equal",
                   AstFor("cast_int_40")},
        // Vector, both unsigned
        BinaryData{"v2bool", "v2uint_10_20", "OpULessThanEqual", "v2uint_20_10",
                   "__vec_2__bool", AstFor("v2uint_10_20"), "less_than_equal",
                   AstFor("v2uint_20_10")},
        // First arg signed
        BinaryData{"v2bool", "v2int_30_40", "OpULessThanEqual", "v2uint_20_10",
                   "__vec_2__bool", AstFor("cast_v2int_30_40"),
                   "less_than_equal", AstFor("v2uint_20_10")},
        // Second arg signed
        BinaryData{"v2bool", "v2uint_10_20", "OpULessThanEqual", "v2int_40_30",
                   "__vec_2__bool", AstFor("v2uint_10_20"), "less_than_equal",
                   AstFor("cast_v2int_40_30")}));

// TODO(dneto): OpAny  - likely builtin function TBD
// TODO(dneto): OpAll  - likely builtin function TBD
// TODO(dneto): OpIsNan - likely builtin function TBD
// TODO(dneto): OpIsInf - likely builtin function TBD
// TODO(dneto): Kernel-guarded instructions.
// TODO(dneto): OpSelect - likely builtin function TBD
// TODO(dneto): OpSGreaterThan
// TODO(dneto): OpSGreaterThanEqual
// TODO(dneto): OpSLessThan
// TODO(dneto): OpSLessThanEqual
// TODO(dneto): OpFUnordEqual
// TODO(dneto): OpFOrdNotEqual
// TODO(dneto): OpFUnordNotEqual
// TODO(dneto): OpFOrdLessThan
// TODO(dneto): OpFUnordLessThan
// TODO(dneto): OpFOrdGreaterThan
// TODO(dneto): OpFUnordGreaterThan
// TODO(dneto): OpFOrdLessThanEqual
// TODO(dneto): OpFUnordLessThanEqual
// TODO(dneto): OpFOrdGreaterThanEqual
// TODO(dneto): OpFUnordGreaterThanEqual

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
