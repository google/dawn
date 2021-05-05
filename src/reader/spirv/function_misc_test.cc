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

using ::testing::Eq;
using ::testing::HasSubstr;

std::string Preamble() {
  return R"(
   OpCapability Shader
   OpMemoryModel Logical Simple
   OpEntryPoint Fragment %100 "main"
   OpExecutionMode %100 OriginUpperLeft
)";
}

std::string CommonTypes() {
  return R"(
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %bool = OpTypeBool
  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %v2bool = OpTypeVector %bool 2
  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
)";
}

using SpvParserTestMiscInstruction = SpvParserTest;

TEST_F(SpvParserTestMiscInstruction, OpUndef_BeforeFunction_Scalar) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %1 = OpUndef %bool
     %2 = OpUndef %uint
     %3 = OpUndef %int
     %4 = OpUndef %float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %11 = OpCopyObject %bool %1
     %12 = OpCopyObject %uint %2
     %13 = OpCopyObject %int %3
     %14 = OpCopyObject %float %4
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  VariableConst{
    x_11
    none
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_12
    none
    __u32
    {
      ScalarConstructor[not set]{0u}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_13
    none
    __i32
    {
      ScalarConstructor[not set]{0}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_14
    none
    __f32
    {
      ScalarConstructor[not set]{0.000000}
    }
  }
})"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_BeforeFunction_Vector) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %4 = OpUndef %v2bool
     %1 = OpUndef %v2uint
     %2 = OpUndef %v2int
     %3 = OpUndef %v2float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel

     %14 = OpCopyObject %v2uint %4
     %11 = OpCopyObject %v2uint %1
     %12 = OpCopyObject %v2int %2
     %13 = OpCopyObject %v2float %3
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  VariableConst{
    x_14
    none
    __vec_2__bool
    {
      TypeConstructor[not set]{
        __vec_2__bool
        ScalarConstructor[not set]{false}
        ScalarConstructor[not set]{false}
      }
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_11
    none
    __vec_2__u32
    {
      TypeConstructor[not set]{
        __vec_2__u32
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_12
    none
    __vec_2__i32
    {
      TypeConstructor[not set]{
        __vec_2__i32
        ScalarConstructor[not set]{0}
        ScalarConstructor[not set]{0}
      }
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_13
    none
    __vec_2__f32
    {
      TypeConstructor[not set]{
        __vec_2__f32
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
      }
    }
  }
})"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Scalar) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %bool
     %2 = OpUndef %uint
     %3 = OpUndef %int
     %4 = OpUndef %float

     %11 = OpCopyObject %bool %1
     %12 = OpCopyObject %uint %2
     %13 = OpCopyObject %int %3
     %14 = OpCopyObject %float %4
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  VariableConst{
    x_11
    none
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_12
    none
    __u32
    {
      ScalarConstructor[not set]{0u}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_13
    none
    __i32
    {
      ScalarConstructor[not set]{0}
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_14
    none
    __f32
    {
      ScalarConstructor[not set]{0.000000}
    }
  }
})"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Vector) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %v2uint
     %2 = OpUndef %v2int
     %3 = OpUndef %v2float

     %11 = OpCopyObject %v2uint %1
     %12 = OpCopyObject %v2int %2
     %13 = OpCopyObject %v2float %3
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  VariableConst{
    x_11
    none
    __vec_2__u32
    {
      TypeConstructor[not set]{
        __vec_2__u32
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_12
    none
    __vec_2__i32
    {
      TypeConstructor[not set]{
        __vec_2__i32
        ScalarConstructor[not set]{0}
        ScalarConstructor[not set]{0}
      }
    }
  }
}
VariableDeclStatement{
  VariableConst{
    x_13
    none
    __vec_2__f32
    {
      TypeConstructor[not set]{
        __vec_2__f32
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
      }
    }
  }
})"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Matrix) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %mat = OpTypeMatrix %v2float 2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %mat

     %11 = OpCopyObject %mat %1
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  VariableConst{
    x_11
    none
    __mat_2_2__f32
    {
      TypeConstructor[not set]{
        __mat_2_2__f32
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{0.000000}
          ScalarConstructor[not set]{0.000000}
        }
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{0.000000}
          ScalarConstructor[not set]{0.000000}
        }
      }
    }
  }
})"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Array) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %uint_2 = OpConstant %uint 2
     %arr = OpTypeArray %uint %uint_2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %arr

     %11 = OpCopyObject %arr %1
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  VariableConst{
    x_11
    none
    __array__u32_2
    {
      TypeConstructor[not set]{
        __array__u32_2
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  }
})"));
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Struct) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %strct = OpTypeStruct %bool %uint %int %float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %strct

     %11 = OpCopyObject %strct %1
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(p->builder(), fe.ast_body()),
              HasSubstr(R"(VariableDeclStatement{
  VariableConst{
    x_11
    none
    __type_name_S
    {
      TypeConstructor[not set]{
        __type_name_S
        ScalarConstructor[not set]{false}
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0}
        ScalarConstructor[not set]{0.000000}
      }
    }
  }
})"));
}

TEST_F(SpvParserTestMiscInstruction, OpNop) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpNop
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << p->error() << assembly;
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(p->builder(), fe.ast_body()), Eq(R"(Return{}
)"));
}

// Test swizzle generation.

struct SwizzleCase {
  uint32_t index;
  std::string expected_expr;
  std::string expected_error;
};
using SpvParserSwizzleTest =
    SpvParserTestBase<::testing::TestWithParam<SwizzleCase>>;

TEST_P(SpvParserSwizzleTest, Sample) {
  // We need a function so we can get a FunctionEmitter.
  const auto assembly = Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  auto fe = p->function_emitter(100);

  auto* result = fe.Swizzle(GetParam().index);
  if (GetParam().expected_error.empty()) {
    Program program(p->program());
    EXPECT_TRUE(fe.success());
    ASSERT_NE(result, nullptr);
    auto got = program.str(result);
    EXPECT_EQ(got, GetParam().expected_expr);
  } else {
    EXPECT_EQ(result, nullptr);
    EXPECT_FALSE(fe.success());
    EXPECT_EQ(p->error(), GetParam().expected_error);
  }
}

INSTANTIATE_TEST_SUITE_P(
    ValidIndex,
    SpvParserSwizzleTest,
    ::testing::ValuesIn(std::vector<SwizzleCase>{
        {0, "Identifier[not set]{x}\n", ""},
        {1, "Identifier[not set]{y}\n", ""},
        {2, "Identifier[not set]{z}\n", ""},
        {3, "Identifier[not set]{w}\n", ""},
        {4, "", "vector component index is larger than 3: 4"},
        {99999, "", "vector component index is larger than 3: 99999"}}));

// TODO(dneto): OpSizeof : requires Kernel (OpenCL)

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
