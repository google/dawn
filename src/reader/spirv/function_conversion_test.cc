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

using ::testing::Eq;
using ::testing::HasSubstr;

std::string CommonTypes() {
  return R"(
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %bool = OpTypeBool
  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %true = OpConstantTrue %bool
  %false = OpConstantFalse %bool
  %v2bool = OpTypeVector %bool 2
  %v2bool_t_f = OpConstantComposite %v2bool %true %false

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

using SpvUnaryConversionTest = SpvParserTestBase<::testing::Test>;

TEST_F(SpvUnaryConversionTest, Bitcast_Scalar) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpBitcast %uint %float_50
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
        ScalarConstructor{50.000000}
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, Bitcast_Vector) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpBitcast %v2float %v2uint_10_20
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
      As<__vec_2__f32>{
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

TEST_F(SpvUnaryConversionTest, ConvertSToF_BadArg) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %float %void
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_BadArg) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %float %void
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_BadArg) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %float %void
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_BadArg) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %float %void
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              HasSubstr("unhandled expression for ID 2\n%2 = OpTypeVoid"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_BadArgType) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %float %false
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              HasSubstr("operand for conversion to floating point must be "
                        "integral scalar or vector, but got: __bool"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_BadArgType) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertSToF %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(
      p->error(),
      HasSubstr("operand for conversion to floating point must be integral "
                "scalar or vector, but got: __vec_2__bool"));
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_FromSigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %int %int_30
     %1 = OpConvertSToF %float %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __f32
    {
      Cast<__f32>(
        Identifier{x_30}
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Scalar_FromUnsigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %uint %uint_10
     %1 = OpConvertSToF %float %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __f32
    {
      Cast<__f32>(
        As<__i32>{
          Identifier{x_30}
        }
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_FromSigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2int %v2int_30_40
     %1 = OpConvertSToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __vec_2__f32
    {
      Cast<__vec_2__f32>(
        Identifier{x_30}
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertSToF_Vector_FromUnsigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2uint %v2uint_10_20
     %1 = OpConvertSToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __vec_2__f32
    {
      Cast<__vec_2__f32>(
        As<__vec_2__i32>{
          Identifier{x_30}
        }
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_BadArgType) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %float %false
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(), Eq("operand for conversion to floating point must be "
                             "integral scalar or vector, but got: __bool"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_BadArgType) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertUToF %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              Eq("operand for conversion to floating point must be integral "
                 "scalar or vector, but got: __vec_2__bool"));
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_FromSigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %int %int_30
     %1 = OpConvertUToF %float %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __f32
    {
      Cast<__f32>(
        As<__u32>{
          Identifier{x_30}
        }
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Scalar_FromUnsigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %uint %uint_10
     %1 = OpConvertUToF %float %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __f32
    {
      Cast<__f32>(
        Identifier{x_30}
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_FromSigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2int %v2int_30_40
     %1 = OpConvertUToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __vec_2__f32
    {
      Cast<__vec_2__f32>(
        As<__vec_2__u32>{
          Identifier{x_30}
        }
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertUToF_Vector_FromUnsigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2uint %v2uint_10_20
     %1 = OpConvertUToF %v2float %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __vec_2__f32
    {
      Cast<__vec_2__f32>(
        Identifier{x_30}
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_BadArgType) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %int %uint_10
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              Eq("operand for conversion to signed integer must be floating "
                 "point scalar or vector, but got: __u32"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_BadArgType) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToS %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              Eq("operand for conversion to signed integer must be floating "
                 "point scalar or vector, but got: __vec_2__bool"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_ToSigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToS %int %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __i32
    {
      Cast<__i32>(
        Identifier{x_30}
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Scalar_ToUnsigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToS %uint %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __u32
    {
      As<__u32>{
        Cast<__i32>(
          Identifier{x_30}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_ToSigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToS %v2int %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __vec_2__i32
    {
      Cast<__vec_2__i32>(
        Identifier{x_30}
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertFToS_Vector_ToUnsigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToS %v2uint %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __vec_2__u32
    {
      As<__vec_2__u32>{
        Cast<__vec_2__i32>(
          Identifier{x_30}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_BadArgType) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %int %uint_10
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              Eq("operand for conversion to unsigned integer must be floating "
                 "point scalar or vector, but got: __u32"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_BadArgType) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpConvertFToU %v2float %v2bool_t_f
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(fe.EmitBody());
  EXPECT_THAT(p->error(),
              Eq("operand for conversion to unsigned integer must be floating "
                 "point scalar or vector, but got: __vec_2__bool"));
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_ToSigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToU %int %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __i32
    {
      As<__i32>{
        Cast<__u32>(
          Identifier{x_30}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Scalar_ToUnsigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %float %float_50
     %1 = OpConvertFToU %uint %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __u32
    {
      Cast<__u32>(
        Identifier{x_30}
      )
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_ToSigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToU %v2int %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __vec_2__i32
    {
      As<__vec_2__i32>{
        Cast<__vec_2__u32>(
          Identifier{x_30}
        )
      }
    }
  })"))
      << ToString(fe.ast_body());
}

TEST_F(SpvUnaryConversionTest, ConvertFToU_Vector_ToUnsigned) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %30 = OpCopyObject %v2float %v2float_50_60
     %1 = OpConvertFToU %v2uint %30
     OpReturn
     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Variable{
    x_1
    none
    __vec_2__u32
    {
      Cast<__vec_2__u32>(
        Identifier{x_30}
      )
    }
  })"))
      << ToString(fe.ast_body());
}

// TODO(dneto): OpSConvert // only if multiple widths
// TODO(dneto): OpUConvert // only if multiple widths
// TODO(dneto): OpFConvert // only if multiple widths
// TODO(dneto): OpQuantizeToF16 // only if f16 supported
// TODO(dneto): OpSatConvertSToU // Kernel (OpenCL), not in WebGPU
// TODO(dneto): OpSatConvertUToS // Kernel (OpenCL), not in WebGPU

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
