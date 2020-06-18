// Copyright 2020 The Tint Authors.  //
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

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2

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
  if (assembly == "v2int_30_40") {
    return R"(TypeConstructor{
          __vec_2__i32
          ScalarConstructor{30}
          ScalarConstructor{40}
        })";
  }
  if (assembly == "v2int_40_30") {
    return R"(TypeConstructor{
          __vec_2__i32
          ScalarConstructor{40}
          ScalarConstructor{30}
        })";
  }
  if (assembly == "cast_int_v2uint_10_20") {
    return R"(As<__vec_2__i32>{
          TypeConstructor{
            __vec_2__u32
            ScalarConstructor{10}
            ScalarConstructor{20}
          }
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

using SpvUnaryBitTest = SpvParserTestBase<::testing::Test>;

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

using SpvBinaryBitTest =
    SpvParserTestBase<::testing::TestWithParam<BinaryData>>;
using SpvBinaryBitTestBasic = SpvParserTestBase<::testing::Test>;

TEST_P(SpvBinaryBitTest, EmitExpression) {
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
    SpvParserTest_ShiftLeftLogical,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpShiftLeftLogical", "uint_20", "__u32",
                   "ScalarConstructor{10}", "shift_left",
                   "ScalarConstructor{20}"},
        // Both int
        BinaryData{"int", "int_30", "OpShiftLeftLogical", "int_40", "__i32",
                   "ScalarConstructor{30}", "shift_left",
                   "ScalarConstructor{40}"},
        // Mixed, returning uint
        BinaryData{"uint", "int_30", "OpShiftLeftLogical", "uint_10", "__u32",
                   "ScalarConstructor{30}", "shift_left",
                   "ScalarConstructor{10}"},
        // Mixed, returning int
        BinaryData{"int", "int_30", "OpShiftLeftLogical", "uint_10", "__i32",
                   "ScalarConstructor{30}", "shift_left",
                   "ScalarConstructor{10}"},
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpShiftLeftLogical",
                   "v2uint_20_10", "__vec_2__u32", AstFor("v2uint_10_20"),
                   "shift_left", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpShiftLeftLogical", "v2int_40_30",
                   "__vec_2__i32", AstFor("v2int_30_40"), "shift_left",
                   AstFor("v2int_40_30")},
        // Mixed, returning v2uint
        BinaryData{"v2uint", "v2int_30_40", "OpShiftLeftLogical",
                   "v2uint_10_20", "__vec_2__u32", AstFor("v2int_30_40"),
                   "shift_left", AstFor("v2uint_10_20")},
        // Mixed, returning v2int
        BinaryData{"v2int", "v2int_40_30", "OpShiftLeftLogical", "v2uint_20_10",
                   "__vec_2__i32", AstFor("v2int_40_30"), "shift_left",
                   AstFor("v2uint_20_10")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_ShiftRightLogical,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpShiftRightLogical", "uint_20", "__u32",
                   "ScalarConstructor{10}", "shift_right",
                   "ScalarConstructor{20}"},
        // Both int
        BinaryData{"int", "int_30", "OpShiftRightLogical", "int_40", "__i32",
                   "ScalarConstructor{30}", "shift_right",
                   "ScalarConstructor{40}"},
        // Mixed, returning uint
        BinaryData{"uint", "int_30", "OpShiftRightLogical", "uint_10", "__u32",
                   "ScalarConstructor{30}", "shift_right",
                   "ScalarConstructor{10}"},
        // Mixed, returning int
        BinaryData{"int", "int_30", "OpShiftRightLogical", "uint_10", "__i32",
                   "ScalarConstructor{30}", "shift_right",
                   "ScalarConstructor{10}"},
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpShiftRightLogical",
                   "v2uint_20_10", "__vec_2__u32", AstFor("v2uint_10_20"),
                   "shift_right", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpShiftRightLogical", "v2int_40_30",
                   "__vec_2__i32", AstFor("v2int_30_40"), "shift_right",
                   AstFor("v2int_40_30")},
        // Mixed, returning v2uint
        BinaryData{"v2uint", "v2int_30_40", "OpShiftRightLogical",
                   "v2uint_10_20", "__vec_2__u32", AstFor("v2int_30_40"),
                   "shift_right", AstFor("v2uint_10_20")},
        // Mixed, returning v2int
        BinaryData{"v2int", "v2int_40_30", "OpShiftRightLogical",
                   "v2uint_20_10", "__vec_2__i32", AstFor("v2int_40_30"),
                   "shift_right", AstFor("v2uint_20_10")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_ShiftRightArithmetic,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpShiftRightArithmetic", "uint_20",
                   "__u32", "ScalarConstructor{10}", "shift_right",
                   "ScalarConstructor{20}"},
        // Both int
        BinaryData{"int", "int_30", "OpShiftRightArithmetic", "int_40", "__i32",
                   "ScalarConstructor{30}", "shift_right",
                   "ScalarConstructor{40}"},
        // Mixed, returning uint
        BinaryData{"uint", "int_30", "OpShiftRightArithmetic", "uint_10",
                   "__u32", "ScalarConstructor{30}", "shift_right",
                   "ScalarConstructor{10}"},
        // Mixed, returning int
        BinaryData{"int", "int_30", "OpShiftRightArithmetic", "uint_10",
                   "__i32", "ScalarConstructor{30}", "shift_right",
                   "ScalarConstructor{10}"},
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpShiftRightArithmetic",
                   "v2uint_20_10", "__vec_2__u32", AstFor("v2uint_10_20"),
                   "shift_right", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpShiftRightArithmetic",
                   "v2int_40_30", "__vec_2__i32", AstFor("v2int_30_40"),
                   "shift_right", AstFor("v2int_40_30")},
        // Mixed, returning v2uint
        BinaryData{"v2uint", "v2int_30_40", "OpShiftRightArithmetic",
                   "v2uint_10_20", "__vec_2__u32", AstFor("v2int_30_40"),
                   "shift_right", AstFor("v2uint_10_20")},
        // Mixed, returning v2int
        BinaryData{"v2int", "v2int_40_30", "OpShiftRightArithmetic",
                   "v2uint_20_10", "__vec_2__i32", AstFor("v2int_40_30"),
                   "shift_right", AstFor("v2uint_20_10")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_BitwiseAnd,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpBitwiseAnd", "uint_20", "__u32",
                   "ScalarConstructor{10}", "and", "ScalarConstructor{20}"},
        // Both int
        BinaryData{"int", "int_30", "OpBitwiseAnd", "int_40", "__i32",
                   "ScalarConstructor{30}", "and", "ScalarConstructor{40}"},
        // Mixed, returning uint
        BinaryData{"uint", "int_30", "OpBitwiseAnd", "uint_10", "__u32",
                   "ScalarConstructor{30}", "and", "ScalarConstructor{10}"},
        // Mixed, returning int
        BinaryData{"int", "int_30", "OpBitwiseAnd", "uint_10", "__i32",
                   "ScalarConstructor{30}", "and", "ScalarConstructor{10}"},
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpBitwiseAnd", "v2uint_20_10",
                   "__vec_2__u32", AstFor("v2uint_10_20"), "and",
                   AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpBitwiseAnd", "v2int_40_30",
                   "__vec_2__i32", AstFor("v2int_30_40"), "and",
                   AstFor("v2int_40_30")},
        // Mixed, returning v2uint
        BinaryData{"v2uint", "v2int_30_40", "OpBitwiseAnd", "v2uint_10_20",
                   "__vec_2__u32", AstFor("v2int_30_40"), "and",
                   AstFor("v2uint_10_20")},
        // Mixed, returning v2int
        BinaryData{"v2int", "v2int_40_30", "OpBitwiseAnd", "v2uint_20_10",
                   "__vec_2__i32", AstFor("v2int_40_30"), "and",
                   AstFor("v2uint_20_10")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_BitwiseOr,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpBitwiseOr", "uint_20", "__u32",
                   "ScalarConstructor{10}", "or", "ScalarConstructor{20}"},
        // Both int
        BinaryData{"int", "int_30", "OpBitwiseOr", "int_40", "__i32",
                   "ScalarConstructor{30}", "or", "ScalarConstructor{40}"},
        // Mixed, returning uint
        BinaryData{"uint", "int_30", "OpBitwiseOr", "uint_10", "__u32",
                   "ScalarConstructor{30}", "or", "ScalarConstructor{10}"},
        // Mixed, returning int
        BinaryData{"int", "int_30", "OpBitwiseOr", "uint_10", "__i32",
                   "ScalarConstructor{30}", "or", "ScalarConstructor{10}"},
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpBitwiseOr", "v2uint_20_10",
                   "__vec_2__u32", AstFor("v2uint_10_20"), "or",
                   AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpBitwiseOr", "v2int_40_30",
                   "__vec_2__i32", AstFor("v2int_30_40"), "or",
                   AstFor("v2int_40_30")},
        // Mixed, returning v2uint
        BinaryData{"v2uint", "v2int_30_40", "OpBitwiseOr", "v2uint_10_20",
                   "__vec_2__u32", AstFor("v2int_30_40"), "or",
                   AstFor("v2uint_10_20")},
        // Mixed, returning v2int
        BinaryData{"v2int", "v2int_40_30", "OpBitwiseOr", "v2uint_20_10",
                   "__vec_2__i32", AstFor("v2int_40_30"), "or",
                   AstFor("v2uint_20_10")}));

INSTANTIATE_TEST_SUITE_P(
    SpvParserTest_BitwiseXor,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpBitwiseXor", "uint_20", "__u32",
                   "ScalarConstructor{10}", "xor", "ScalarConstructor{20}"},
        // Both int
        BinaryData{"int", "int_30", "OpBitwiseXor", "int_40", "__i32",
                   "ScalarConstructor{30}", "xor", "ScalarConstructor{40}"},
        // Mixed, returning uint
        BinaryData{"uint", "int_30", "OpBitwiseXor", "uint_10", "__u32",
                   "ScalarConstructor{30}", "xor", "ScalarConstructor{10}"},
        // Mixed, returning int
        BinaryData{"int", "int_30", "OpBitwiseXor", "uint_10", "__i32",
                   "ScalarConstructor{30}", "xor", "ScalarConstructor{10}"},
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpBitwiseXor", "v2uint_20_10",
                   "__vec_2__u32", AstFor("v2uint_10_20"), "xor",
                   AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpBitwiseXor", "v2int_40_30",
                   "__vec_2__i32", AstFor("v2int_30_40"), "xor",
                   AstFor("v2int_40_30")},
        // Mixed, returning v2uint
        BinaryData{"v2uint", "v2int_30_40", "OpBitwiseXor", "v2uint_10_20",
                   "__vec_2__u32", AstFor("v2int_30_40"), "xor",
                   AstFor("v2uint_10_20")},
        // Mixed, returning v2int
        BinaryData{"v2int", "v2int_40_30", "OpBitwiseXor", "v2uint_20_10",
                   "__vec_2__i32", AstFor("v2int_40_30"), "xor",
                   AstFor("v2uint_20_10")}));

TEST_F(SpvUnaryBitTest, Not_Int_Int) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %int %int_30
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
    __i32
    {
      UnaryOp{
        not
        ScalarConstructor{30}
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryBitTest, Not_Int_Uint) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %int %uint_10
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
    __i32
    {
      As<__i32>{
        UnaryOp{
          not
          ScalarConstructor{10}
        }
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryBitTest, Not_Uint_Int) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %uint %int_30
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
    __u32
    {
      As<__u32>{
        UnaryOp{
          not
          ScalarConstructor{30}
        }
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryBitTest, Not_Uint_Uint) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %uint %uint_10
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
    __u32
    {
      UnaryOp{
        not
        ScalarConstructor{10}
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryBitTest, Not_SignedVec_SignedVec) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %v2int %v2int_30_40
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
    __vec_2__i32
    {
      UnaryOp{
        not
        TypeConstructor{
          __vec_2__i32
          ScalarConstructor{30}
          ScalarConstructor{40}
        }
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryBitTest, Not_SignedVec_UnsignedVec) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %v2int %v2uint_10_20
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
    __vec_2__i32
    {
      As<__vec_2__i32>{
        UnaryOp{
          not
          TypeConstructor{
            __vec_2__u32
            ScalarConstructor{10}
            ScalarConstructor{20}
          }
        }
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryBitTest, Not_UnsignedVec_SignedVec) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %v2uint %v2int_30_40
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
    __vec_2__u32
    {
      As<__vec_2__u32>{
        UnaryOp{
          not
          TypeConstructor{
            __vec_2__i32
            ScalarConstructor{30}
            ScalarConstructor{40}
          }
        }
      }
    }
  })"))
      << ToString(fe.ast_body());
}
TEST_F(SpvUnaryBitTest, Not_UnsignedVec_UnsignedVec) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %v2uint %v2uint_10_20
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
    __vec_2__u32
    {
      UnaryOp{
        not
        TypeConstructor{
          __vec_2__u32
          ScalarConstructor{10}
          ScalarConstructor{20}
        }
      }
    }
  })"))
      << ToString(fe.ast_body());
}

// TODO(dneto): OpBitFieldInsert
// TODO(dneto): OpBitFieldSExtract
// TODO(dneto): OpBitFieldUExtract
// TODO(dneto): OpBitReverse
// TODO(dneto): OpBitCount

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
