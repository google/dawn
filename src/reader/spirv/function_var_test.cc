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

/// @returns a SPIR-V assembly segment which assigns debug names
/// to particular IDs.
std::string Names(std::vector<std::string> ids) {
  std::ostringstream outs;
  for (auto& id : ids) {
    outs << "    OpName %" << id << " \"" << id << "\"\n";
  }
  return outs.str();
}

std::string CommonTypes() {
  return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %ptr_bool = OpTypePointer Function %bool
    %ptr_float = OpTypePointer Function %float
    %ptr_uint = OpTypePointer Function %uint
    %ptr_int = OpTypePointer Function %int

    %true = OpConstantTrue %bool
    %false = OpConstantFalse %bool
    %float_0 = OpConstant %float 0.0
    %float_1p5 = OpConstant %float 1.5
    %uint_1 = OpConstant %uint 1
    %int_m1 = OpConstant %int -1
    %uint_2 = OpConstant %uint 2

    %v2float = OpTypeVector %float 2
    %m3v2float = OpTypeMatrix %v2float 3

    %arr2uint = OpTypeArray %uint %uint_2
    %strct = OpTypeStruct %uint %float %arr2uint
  )";
}

TEST_F(SpvParserTest, EmitFunctionVariables_AnonymousVars) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_uint Function
     %2 = OpVariable %ptr_uint Function
     %3 = OpVariable %ptr_uint Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_1
    function
    __u32
  }
}
VariableDeclStatement{
  Variable{
    x_2
    function
    __u32
  }
}
VariableDeclStatement{
  Variable{
    x_3
    function
    __u32
  }
}
)"));
}

TEST_F(SpvParserTest, EmitFunctionVariables_NamedVars) {
  auto* p = parser(test::Assemble(Names({"a", "b", "c"}) + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_uint Function
     %b = OpVariable %ptr_uint Function
     %c = OpVariable %ptr_uint Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    function
    __u32
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __u32
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __u32
  }
}
)"));
}

TEST_F(SpvParserTest, EmitFunctionVariables_MixedTypes) {
  auto* p = parser(test::Assemble(Names({"a", "b", "c"}) + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_uint Function
     %b = OpVariable %ptr_int Function
     %c = OpVariable %ptr_float Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    function
    __u32
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __i32
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __f32
  }
}
)"));
}

TEST_F(SpvParserTest, EmitFunctionVariables_ScalarInitializers) {
  auto* p = parser(
      test::Assemble(Names({"a", "b", "c", "d", "e"}) + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_bool Function %true
     %b = OpVariable %ptr_bool Function %false
     %c = OpVariable %ptr_int Function %int_m1
     %d = OpVariable %ptr_uint Function %uint_1
     %e = OpVariable %ptr_float Function %float_1p5
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    function
    __bool
    {
      ScalarConstructor{true}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __bool
    {
      ScalarConstructor{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __i32
    {
      ScalarConstructor{-1}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    function
    __u32
    {
      ScalarConstructor{1}
    }
  }
}
VariableDeclStatement{
  Variable{
    e
    function
    __f32
    {
      ScalarConstructor{1.500000}
    }
  }
}
)"));
}

TEST_F(SpvParserTest, EmitFunctionVariables_ScalarNullInitializers) {
  auto* p =
      parser(test::Assemble(Names({"a", "b", "c", "d"}) + CommonTypes() + R"(
     %null_bool = OpConstantNull %bool
     %null_int = OpConstantNull %int
     %null_uint = OpConstantNull %uint
     %null_float = OpConstantNull %float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_bool Function %null_bool
     %b = OpVariable %ptr_int Function %null_int
     %c = OpVariable %ptr_uint Function %null_uint
     %d = OpVariable %ptr_float Function %null_float
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    function
    __bool
    {
      ScalarConstructor{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __i32
    {
      ScalarConstructor{0}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __u32
    {
      ScalarConstructor{0}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    function
    __f32
    {
      ScalarConstructor{0.000000}
    }
  }
}
)"));
}

TEST_F(SpvParserTest, EmitFunctionVariables_VectorInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Function %v2float
     %two = OpConstant %float 2.0
     %const = OpConstantComposite %v2float %float_1p5 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    function
    __vec_2__f32
    {
      TypeConstructor{
        __vec_2__f32
        ScalarConstructor{1.500000}
        ScalarConstructor{2.000000}
      }
    }
  }
}
)")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTest, EmitFunctionVariables_MatrixInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Function %m3v2float
     %two = OpConstant %float 2.0
     %three = OpConstant %float 3.0
     %four = OpConstant %float 4.0
     %v0 = OpConstantComposite %v2float %float_1p5 %two
     %v1 = OpConstantComposite %v2float %two %three
     %v2 = OpConstantComposite %v2float %three %four
     %const = OpConstantComposite %m3v2float %v0 %v1 %v2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    function
    __mat_2_3__f32
    {
      TypeConstructor{
        __mat_2_3__f32
        TypeConstructor{
          __vec_2__f32
          ScalarConstructor{1.500000}
          ScalarConstructor{2.000000}
        }
        TypeConstructor{
          __vec_2__f32
          ScalarConstructor{2.000000}
          ScalarConstructor{3.000000}
        }
        TypeConstructor{
          __vec_2__f32
          ScalarConstructor{3.000000}
          ScalarConstructor{4.000000}
        }
      }
    }
  }
}
)"));
}

TEST_F(SpvParserTest, EmitFunctionVariables_ArrayInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    function
    __array__u32_2
    {
      TypeConstructor{
        __array__u32_2
        ScalarConstructor{1}
        ScalarConstructor{2}
      }
    }
  }
}
)")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTest, EmitFunctionVariables_ArrayInitializer_AliasType) {
  auto* p = parser(test::Assemble(
      std::string("OpDecorate %arr2uint ArrayStride 16\n") + CommonTypes() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    function
    __alias_Arr__array__u32_2_16
    {
      TypeConstructor{
        __alias_Arr__array__u32_2_16
        ScalarConstructor{1}
        ScalarConstructor{2}
      }
    }
  }
}
)")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTest, EmitFunctionVariables_ArrayInitializer_Null) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantNull %arr2uint

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    function
    __array__u32_2
    {
      TypeConstructor{
        __array__u32_2
      }
    }
  }
}
)")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTest, EmitFunctionVariables_ArrayInitializer_AliasType_Null) {
  auto* p = parser(test::Assemble(
      std::string("OpDecorate %arr2uint ArrayStride 16\n") + CommonTypes() + R"(
     %ptr = OpTypePointer Function %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantNull %arr2uint

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    function
    __alias_Arr__array__u32_2_16
    {
      TypeConstructor{
        __alias_Arr__array__u32_2_16
      }
    }
  }
}
)")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTest, EmitFunctionVariables_StructInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Function %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantComposite %strct %uint_1 %float_1p5 %arrconst

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    function
    __alias_S__struct_S
    {
      TypeConstructor{
        __alias_S__struct_S
        ScalarConstructor{1}
        ScalarConstructor{1.500000}
        TypeConstructor{
          __array__u32_2
          ScalarConstructor{1}
          ScalarConstructor{2}
        }
      }
    }
  }
}
)")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTest, EmitFunctionVariables_StructInitializer_Null) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Function %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantNull %strct

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %200 = OpVariable %ptr Function %const
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_200
    function
    __alias_S__struct_S
    {
      TypeConstructor{
        __alias_S__struct_S
      }
    }
  }
}
)")) << ToString(fe.ast_body());
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
