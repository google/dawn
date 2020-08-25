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

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
)";
}

using SpvParserTestMiscInstruction = SpvParserTest;

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Scalar) {
  const auto assembly = CommonTypes() + R"(
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
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_11
    none
    __bool
    {
      ScalarConstructor{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    x_12
    none
    __u32
    {
      ScalarConstructor{0}
    }
  }
}
VariableDeclStatement{
  Variable{
    x_13
    none
    __i32
    {
      ScalarConstructor{0}
    }
  }
}
VariableDeclStatement{
  Variable{
    x_14
    none
    __f32
    {
      ScalarConstructor{0.000000}
    }
  }
})")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Vector) {
  const auto assembly = CommonTypes() + R"(
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
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_11
    none
    __vec_2__u32
    {
      TypeConstructor{
        __vec_2__u32
        ScalarConstructor{0}
        ScalarConstructor{0}
      }
    }
  }
}
VariableDeclStatement{
  Variable{
    x_12
    none
    __vec_2__i32
    {
      TypeConstructor{
        __vec_2__i32
        ScalarConstructor{0}
        ScalarConstructor{0}
      }
    }
  }
}
VariableDeclStatement{
  Variable{
    x_13
    none
    __vec_2__f32
    {
      TypeConstructor{
        __vec_2__f32
        ScalarConstructor{0.000000}
        ScalarConstructor{0.000000}
      }
    }
  }
})")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Matrix) {
  const auto assembly = CommonTypes() + R"(
     %mat = OpTypeMatrix %v2float 2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %mat

     %11 = OpCopyObject %mat %1
     OpReturn
     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_11
    none
    __mat_2_2__f32
    {
      TypeConstructor{
        __mat_2_2__f32
        TypeConstructor{
          __vec_2__f32
          ScalarConstructor{0.000000}
          ScalarConstructor{0.000000}
        }
        TypeConstructor{
          __vec_2__f32
          ScalarConstructor{0.000000}
          ScalarConstructor{0.000000}
        }
      }
    }
  }
})")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Array) {
  const auto assembly = CommonTypes() + R"(
     %uint_2 = OpConstant %uint 2
     %arr = OpTypeArray %uint %uint_2

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %arr

     %11 = OpCopyObject %arr %1
     OpReturn
     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_11
    none
    __array__u32_2
    {
      TypeConstructor{
        __array__u32_2
        ScalarConstructor{0}
        ScalarConstructor{0}
      }
    }
  }
})")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTestMiscInstruction, OpUndef_InFunction_Struct) {
  const auto assembly = CommonTypes() + R"(
     %strct = OpTypeStruct %bool %uint %int %float

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpUndef %strct

     %11 = OpCopyObject %strct %1
     OpReturn
     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_11
    none
    __alias_S__struct_S
    {
      TypeConstructor{
        __alias_S__struct_S
        ScalarConstructor{false}
        ScalarConstructor{0}
        ScalarConstructor{0}
        ScalarConstructor{0.000000}
      }
    }
  }
})")) << ToString(fe.ast_body());
}

TEST_F(SpvParserTestMiscInstruction, OpNop) {
  const auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpNop
     OpReturn
     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions())
      << p->error() << assembly;
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_THAT(ToString(fe.ast_body()), Eq(R"(Return{}
)")) << ToString(fe.ast_body());
}

// TODO(dneto): OpSizeof : requires Kernel (OpenCL)

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
