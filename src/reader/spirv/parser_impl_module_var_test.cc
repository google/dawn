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

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;

std::string CommonTypes() {
  return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %ptr_bool = OpTypePointer Private %bool
    %ptr_float = OpTypePointer Private %float
    %ptr_uint = OpTypePointer Private %uint
    %ptr_int = OpTypePointer Private %int

    %true = OpConstantTrue %bool
    %false = OpConstantFalse %bool
    %float_0 = OpConstant %float 0.0
    %float_1p5 = OpConstant %float 1.5
    %uint_1 = OpConstant %uint 1
    %int_m1 = OpConstant %int -1
    %uint_2 = OpConstant %uint 2

    %v2bool = OpTypeVector %bool 2
    %v2uint = OpTypeVector %uint 2
    %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
    %m3v2float = OpTypeMatrix %v2float 3

    %arr2uint = OpTypeArray %uint %uint_2
    %strct = OpTypeStruct %uint %float %arr2uint
  )";
}

TEST_F(SpvParserTest, ModuleScopeVar_NoVar) {
  auto* p = parser(test::Assemble(""));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->module().to_str();
  EXPECT_THAT(module_ast, Not(HasSubstr("Variable")));
}

TEST_F(SpvParserTest, ModuleScopeVar_BadStorageClass) {
  auto* p = parser(test::Assemble(R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer CrossWorkgroup %float
    %52 = OpVariable %ptr CrossWorkgroup
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  // Normally we should run ParserImpl::RegisterTypes before emitting
  // variables. But defensive coding in EmitModuleScopeVariables lets
  // us catch this error.
  EXPECT_FALSE(p->EmitModuleScopeVariables()) << p->error();
  EXPECT_THAT(p->error(), HasSubstr("unknown SPIR-V storage class: 5"));
}

TEST_F(SpvParserTest, ModuleScopeVar_BadPointerType) {
  auto* p = parser(test::Assemble(R"(
    %float = OpTypeFloat 32
    %fn_ty = OpTypeFunction %float
    %3 = OpTypePointer Private %fn_ty
    %52 = OpVariable %3 Private
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  // Normally we should run ParserImpl::RegisterTypes before emitting
  // variables. But defensive coding in EmitModuleScopeVariables lets
  // us catch this error.
  EXPECT_FALSE(p->EmitModuleScopeVariables());
  EXPECT_THAT(p->error(), HasSubstr("internal error: failed to register Tint "
                                    "AST type for SPIR-V type with ID: 3"));
}

TEST_F(SpvParserTest, ModuleScopeVar_AnonWorkgroupVar) {
  auto* p = parser(test::Assemble(R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Workgroup %float
    %52 = OpVariable %ptr Workgroup
  )"));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    x_52
    workgroup
    __f32
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_NamedWorkgroupVar) {
  auto* p = parser(test::Assemble(R"(
    OpName %52 "the_counter"
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Workgroup %float
    %52 = OpVariable %ptr Workgroup
  )"));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    the_counter
    workgroup
    __f32
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_PrivateVar) {
  auto* p = parser(test::Assemble(R"(
    OpName %52 "my_own_private_idaho"
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Private %float
    %52 = OpVariable %ptr Private
  )"));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    my_own_private_idaho
    private
    __f32
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_BuiltinVerteIndex) {
  auto* p = parser(test::Assemble(R"(
    OpDecorate %52 BuiltIn VertexIndex
    %uint = OpTypeInt 32 0
    %ptr = OpTypePointer Input %uint
    %52 = OpVariable %ptr Input
  )"));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{vertex_idx}
    }
    x_52
    in
    __u32
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_ScalarInitializers) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %1 = OpVariable %ptr_bool Private %true
     %2 = OpVariable %ptr_bool Private %false
     %3 = OpVariable %ptr_int Private %int_m1
     %4 = OpVariable %ptr_uint Private %uint_1
     %5 = OpVariable %ptr_float Private %float_1p5
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_1
    private
    __bool
    {
      ScalarConstructor{true}
    }
  }
  Variable{
    x_2
    private
    __bool
    {
      ScalarConstructor{false}
    }
  }
  Variable{
    x_3
    private
    __i32
    {
      ScalarConstructor{-1}
    }
  }
  Variable{
    x_4
    private
    __u32
    {
      ScalarConstructor{1}
    }
  }
  Variable{
    x_5
    private
    __f32
    {
      ScalarConstructor{1.500000}
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_ScalarNullInitializers) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %null_bool = OpConstantNull %bool
     %null_int = OpConstantNull %int
     %null_uint = OpConstantNull %uint
     %null_float = OpConstantNull %float

     %1 = OpVariable %ptr_bool Private %null_bool
     %2 = OpVariable %ptr_int Private %null_int
     %3 = OpVariable %ptr_uint Private %null_uint
     %4 = OpVariable %ptr_float Private %null_float
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_1
    private
    __bool
    {
      ScalarConstructor{false}
    }
  }
  Variable{
    x_2
    private
    __i32
    {
      ScalarConstructor{0}
    }
  }
  Variable{
    x_3
    private
    __u32
    {
      ScalarConstructor{0}
    }
  }
  Variable{
    x_4
    private
    __f32
    {
      ScalarConstructor{0.000000}
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_VectorInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %two = OpConstant %float 2.0
     %const = OpConstantComposite %v2float %float_1p5 %two
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__f32
    {
      TypeConstructor{
        __vec_2__f32
        ScalarConstructor{1.500000}
        ScalarConstructor{2.000000}
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_VectorBoolNullInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2bool
     %const = OpConstantNull %v2bool
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__bool
    {
      TypeConstructor{
        __vec_2__bool
        ScalarConstructor{false}
        ScalarConstructor{false}
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_VectorUintNullInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2uint
     %const = OpConstantNull %v2uint
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__u32
    {
      TypeConstructor{
        __vec_2__u32
        ScalarConstructor{0}
        ScalarConstructor{0}
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_VectorIntNullInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2int
     %const = OpConstantNull %v2int
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__i32
    {
      TypeConstructor{
        __vec_2__i32
        ScalarConstructor{0}
        ScalarConstructor{0}
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_VectorFloatNullInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %const = OpConstantNull %v2float
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__f32
    {
      TypeConstructor{
        __vec_2__f32
        ScalarConstructor{0.000000}
        ScalarConstructor{0.000000}
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_MatrixInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %two = OpConstant %float 2.0
     %three = OpConstant %float 3.0
     %four = OpConstant %float 4.0
     %v0 = OpConstantComposite %v2float %float_1p5 %two
     %v1 = OpConstantComposite %v2float %two %three
     %v2 = OpConstantComposite %v2float %three %four
     %const = OpConstantComposite %m3v2float %v0 %v1 %v2
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
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
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_MatrixNullInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %const = OpConstantNull %m3v2float
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __mat_2_3__f32
    {
      TypeConstructor{
        __mat_2_3__f32
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
        TypeConstructor{
          __vec_2__f32
          ScalarConstructor{0.000000}
          ScalarConstructor{0.000000}
        }
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_ArrayInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __array__u32_2
    {
      TypeConstructor{
        __array__u32_2
        ScalarConstructor{1}
        ScalarConstructor{2}
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_ArrayNullInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %const = OpConstantNull %arr2uint
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __array__u32_2
    {
      TypeConstructor{
        __array__u32_2
        ScalarConstructor{0}
        ScalarConstructor{0}
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_StructInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantComposite %strct %uint_1 %float_1p5 %arrconst
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __struct_S
    {
      TypeConstructor{
        __struct_S
        ScalarConstructor{1}
        ScalarConstructor{1.500000}
        TypeConstructor{
          __array__u32_2
          ScalarConstructor{1}
          ScalarConstructor{2}
        }
      }
    }
  })"));
}

TEST_F(SpvParserTest, ModuleScopeVar_StructNullInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %const = OpConstantNull %strct
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __struct_S
    {
      TypeConstructor{
        __struct_S
        ScalarConstructor{0}
        ScalarConstructor{0.000000}
        TypeConstructor{
          __array__u32_2
          ScalarConstructor{0}
          ScalarConstructor{0}
        }
      }
    }
  })"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
