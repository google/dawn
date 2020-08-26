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

using ::testing::Eq;
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

TEST_F(SpvParserTest, ModuleScopeVar_BadStorageClass_NotAWebGPUStorageClass) {
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

TEST_F(SpvParserTest, ModuleScopeVar_BadStorageClass_Function) {
  auto* p = parser(test::Assemble(R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Function %float
    %52 = OpVariable %ptr Function
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  // Normally we should run ParserImpl::RegisterTypes before emitting
  // variables. But defensive coding in EmitModuleScopeVariables lets
  // us catch this error.
  EXPECT_FALSE(p->EmitModuleScopeVariables()) << p->error();
  EXPECT_THAT(p->error(),
              HasSubstr("invalid SPIR-V storage class 7 for module scope "
                        "variable: %52 = OpVariable %2 Function"));
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

TEST_F(SpvParserTest, ModuleScopeVar_NonPointerType) {
  auto* p = parser(test::Assemble(R"(
    %float = OpTypeFloat 32
    %5 = OpTypeFunction %float
    %3 = OpTypePointer Private %5
    %52 = OpVariable %float Private
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  EXPECT_FALSE(p->RegisterTypes());
  EXPECT_THAT(
      p->error(),
      HasSubstr("SPIR-V pointer type with ID 3 has invalid pointee type 5"));
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

TEST_F(SpvParserTest, ModuleScopeVar_BuiltinVertexIndex) {
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

std::string PerVertexPreamble() {
  return R"(
    OpCapability Shader
    OpCapability Linkage ; so we don't have to declare an entry point
    OpMemoryModel Logical Simple

    OpMemberDecorate %10 0 BuiltIn Position
    OpMemberDecorate %10 1 BuiltIn PointSize
    OpMemberDecorate %10 2 BuiltIn ClipDistance
    OpMemberDecorate %10 3 BuiltIn CullDistance
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %12 = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %arr = OpTypeArray %float %uint_1
    %10 = OpTypeStruct %12 %float %arr %arr
    %11 = OpTypePointer Output %10
    %1 = OpVariable %11 Output
)";
}

TEST_F(SpvParserTest, ModuleScopeVar_BuiltinPosition_MapsToModuleScopeVec4Var) {
  // In Vulkan SPIR-V, Position is the first member of gl_PerVertex
  const std::string assembly = PerVertexPreamble();
  auto* p = parser(test::Assemble(assembly));

  EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
  EXPECT_TRUE(p->error().empty()) << p->error();
  const auto& position_info = p->GetBuiltInPositionInfo();
  EXPECT_EQ(position_info.struct_type_id, 10u);
  EXPECT_EQ(position_info.member_index, 0u);
  EXPECT_EQ(position_info.member_type_id, 12u);
  EXPECT_EQ(position_info.pointer_type_id, 11u);
  EXPECT_EQ(position_info.storage_class, SpvStorageClassOutput);
  EXPECT_EQ(position_info.per_vertex_var_id, 1u);
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{position}
    }
    gl_Position
    out
    __vec_4__f32
  })"))
      << module_str;
}

TEST_F(SpvParserTest,
       ModuleScopeVar_BuiltinPosition_StoreWholeStruct_NotSupported) {
  // Glslang does not generate this code pattern.
  const std::string assembly = PerVertexPreamble() + R"(
  %nil = OpConstantNull %10 ; the whole struct

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpStore %1 %nil  ; store the whole struct
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
  EXPECT_THAT(p->error(), Eq("storing to the whole per-vertex structure is not "
                             "supported: OpStore %1 %9"))
      << p->error();
}

TEST_F(SpvParserTest,
       ModuleScopeVar_BuiltinPosition_IntermediateWholeStruct_NotSupported) {
  const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %1000 = OpUndef %10
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
  EXPECT_THAT(p->error(), Eq("operations producing a per-vertex structure are "
                             "not supported: %1000 = OpUndef %10"))
      << p->error();
}

TEST_F(SpvParserTest,
       ModuleScopeVar_BuiltinPosition_IntermediatePtrWholeStruct_NotSupported) {
  const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %1000 = OpUndef %11
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(), Eq("operations producing a per-vertex structure are "
                             "not supported: %1000 = OpUndef %11"))
      << p->error();
}

TEST_F(SpvParserTest, ModuleScopeVar_BuiltinPosition_StorePosition) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr_v4float = OpTypePointer Output %12
  %nil = OpConstantNull %12

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr_v4float %1 %uint_0 ; address of the Position member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
    Assignment{
      Identifier{gl_Position}
      TypeConstructor{
        __vec_4__f32
        ScalarConstructor{0.000000}
        ScalarConstructor{0.000000}
        ScalarConstructor{0.000000}
        ScalarConstructor{0.000000}
      }
    })"))
      << module_str;
}

TEST_F(SpvParserTest,
       ModuleScopeVar_BuiltinPosition_StorePositionMember_OneAccessChain) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr_float %1 %uint_0 %uint_1 ; address of the Position.y member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
    Assignment{
      MemberAccessor{
        Identifier{gl_Position}
        Identifier{y}
      }
      ScalarConstructor{0.000000}
    })"))
      << module_str;
}

TEST_F(SpvParserTest,
       ModuleScopeVar_BuiltinPosition_StorePositionMember_TwoAccessChain) {
  // The algorithm is smart enough to collapse it down.
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr_v4float = OpTypePointer Output %12
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr_v4float %1 %uint_0 ; address of the Position member
  %101 = OpAccessChain %ptr_float %100 %uint_1 ; address of the Position.y member
  OpStore %101 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  {
    Assignment{
      MemberAccessor{
        Identifier{gl_Position}
        Identifier{y}
      }
      ScalarConstructor{0.000000}
    }
    Return{}
  })"))
      << module_str;
}

TEST_F(SpvParserTest, ModuleScopeVar_BuiltinPointSize_NotSupported) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr_v4float = OpTypePointer Output %12
  %nil = OpConstantNull %12

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr_v4float %1 %uint_1 ; address of the PointSize member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(), Eq("accessing per-vertex member 1 is not supported. "
                             "Only Position is supported"));
}

TEST_F(SpvParserTest, ModuleScopeVar_BuiltinClipDistance_NotSupported) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float
  %uint_2 = OpConstant %uint 2

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
; address of the first entry in ClipDistance
  %100 = OpAccessChain %ptr_float %1 %uint_2 %uint_0
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(), Eq("accessing per-vertex member 2 is not supported. "
                             "Only Position is supported"));
}

TEST_F(SpvParserTest, ModuleScopeVar_BuiltinCullDistance_NotSupported) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float
  %uint_3 = OpConstant %uint 3

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
; address of the first entry in CullDistance
  %100 = OpAccessChain %ptr_float %1 %uint_3 %uint_0
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(), Eq("accessing per-vertex member 3 is not supported. "
                             "Only Position is supported"));
}

TEST_F(SpvParserTest, ModuleScopeVar_BuiltinPerVertex_MemberIndex_NotConstant) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %sum = OpIAdd %uint %uint_0 %uint_0
  %100 = OpAccessChain %ptr_float %1 %sum
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("first index of access chain into per-vertex structure is not "
                 "a constant: %100 = OpAccessChain %9 %1 %16"));
}

TEST_F(SpvParserTest,
       ModuleScopeVar_BuiltinPerVertex_MemberIndex_NotConstantInteger) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
; nil is bad here!
  %100 = OpAccessChain %ptr_float %1 %nil
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("first index of access chain into per-vertex structure is not "
                 "a constant integer: %100 = OpAccessChain %9 %1 %13"));
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

TEST_F(SpvParserTest, ModuleScopeVar_ScalarUndefInitializers) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %undef_bool = OpUndef %bool
     %undef_int = OpUndef %int
     %undef_uint = OpUndef %uint
     %undef_float = OpUndef %float

     %1 = OpVariable %ptr_bool Private %undef_bool
     %2 = OpVariable %ptr_int Private %undef_int
     %3 = OpVariable %ptr_uint Private %undef_uint
     %4 = OpVariable %ptr_float Private %undef_float
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
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

TEST_F(SpvParserTest, ModuleScopeVar_VectorBoolUndefInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2bool
     %const = OpUndef %v2bool
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

TEST_F(SpvParserTest, ModuleScopeVar_VectorUintUndefInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2uint
     %const = OpUndef %v2uint
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

TEST_F(SpvParserTest, ModuleScopeVar_VectorIntUndefInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2int
     %const = OpUndef %v2int
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

TEST_F(SpvParserTest, ModuleScopeVar_VectorFloatUndefInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %const = OpUndef %v2float
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

TEST_F(SpvParserTest, ModuleScopeVar_MatrixUndefInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %const = OpUndef %m3v2float
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

TEST_F(SpvParserTest, ModuleScopeVar_ArrayUndefInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %const = OpUndef %arr2uint
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
  })"))
      << module_str;
}

TEST_F(SpvParserTest, ModuleScopeVar_StructNullInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %const = OpConstantNull %strct
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __alias_S__struct_S
    {
      TypeConstructor{
        __alias_S__struct_S
        ScalarConstructor{0}
        ScalarConstructor{0.000000}
        TypeConstructor{
          __array__u32_2
          ScalarConstructor{0}
          ScalarConstructor{0}
        }
      }
    }
  })"))
      << module_str;
}

TEST_F(SpvParserTest, ModuleScopeVar_StructUndefInitializer) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %const = OpUndef %strct
     %200 = OpVariable %ptr Private %const
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __alias_S__struct_S
    {
      TypeConstructor{
        __alias_S__struct_S
        ScalarConstructor{0}
        ScalarConstructor{0.000000}
        TypeConstructor{
          __array__u32_2
          ScalarConstructor{0}
          ScalarConstructor{0}
        }
      }
    }
  })"))
      << module_str;
}

TEST_F(SpvParserTest, ModuleScopeVar_LocationDecoration_Valid) {
  auto* p = parser(test::Assemble(R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Location 3
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Input %uint
     %myvar = OpVariable %ptr Input
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  DecoratedVariable{
    Decorations{
      LocationDecoration{3}
    }
    myvar
    in
    __u32
  })"))
      << module_str;
}

TEST_F(SpvParserTest,
       ModuleScopeVar_LocationDecoration_MissingOperandWontAssemble) {
  const auto assembly = R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Location
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Input %uint
     %myvar = OpVariable %ptr Input
  )";
  EXPECT_THAT(test::AssembleFailure(assembly),
              Eq("4:4: Expected operand, found next instruction instead."));
}

TEST_F(SpvParserTest,
       ModuleScopeVar_LocationDecoration_TwoOperandsWontAssemble) {
  const auto assembly = R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Location 3 4
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Input %uint
     %myvar = OpVariable %ptr Input
  )";
  EXPECT_THAT(
      test::AssembleFailure(assembly),
      Eq("2:34: Expected <opcode> or <result-id> at the beginning of an "
         "instruction, found '4'."));
}

TEST_F(SpvParserTest, ModuleScopeVar_DescriptorSetDecoration_Valid) {
  auto* p = parser(test::Assemble(R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 3
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  DecoratedVariable{
    Decorations{
      SetDecoration{3}
    }
    myvar
    storage_buffer
    __alias_S__struct_S
  })"))
      << module_str;
}

TEST_F(SpvParserTest,
       ModuleScopeVar_DescriptorSetDecoration_MissingOperandWontAssemble) {
  const auto assembly = R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )";
  EXPECT_THAT(test::AssembleFailure(assembly),
              Eq("3:5: Expected operand, found next instruction instead."));
}

TEST_F(SpvParserTest,
       ModuleScopeVar_DescriptorSetDecoration_TwoOperandsWontAssemble) {
  const auto assembly = R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 3 4
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )";
  EXPECT_THAT(
      test::AssembleFailure(assembly),
      Eq("2:39: Expected <opcode> or <result-id> at the beginning of an "
         "instruction, found '4'."));
}

TEST_F(SpvParserTest, ModuleScopeVar_BindingDecoration_Valid) {
  auto* p = parser(test::Assemble(R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Binding 3
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  DecoratedVariable{
    Decorations{
      BindingDecoration{3}
    }
    myvar
    storage_buffer
    __alias_S__struct_S
  })"))
      << module_str;
}

TEST_F(SpvParserTest,
       ModuleScopeVar_BindingDecoration_MissingOperandWontAssemble) {
  const auto assembly = R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Binding
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )";
  EXPECT_THAT(test::AssembleFailure(assembly),
              Eq("3:5: Expected operand, found next instruction instead."));
}

TEST_F(SpvParserTest,
       ModuleScopeVar_BindingDecoration_TwoOperandsWontAssemble) {
  const auto assembly = R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Binding 3 4
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )";
  EXPECT_THAT(
      test::AssembleFailure(assembly),
      Eq("2:33: Expected <opcode> or <result-id> at the beginning of an "
         "instruction, found '4'."));
}

TEST_F(SpvParserTest, ModuleScopeVar_NonReadableDecoration_DroppedForNow) {
  auto* p = parser(test::Assemble(R"(
     OpName %myvar "myvar"
     OpDecorate %strct Block
     OpMemberDecorate %strct 0 NonReadable
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    myvar
    storage_buffer
    __alias_S__struct_S
  }
  S -> __struct_S
  [[block]] Struct{
    StructMember{field0: __u32}
    StructMember{field1: __f32}
    StructMember{field2: __array__u32_2}
  }
})")) << module_str;
}

TEST_F(SpvParserTest, ModuleScopeVar_NonWritableDecoration_DroppedForNow) {
  auto* p = parser(test::Assemble(R"(
     OpName %myvar "myvar"
     OpDecorate %strct Block
     OpMemberDecorate %strct 0 NonWritable
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    myvar
    storage_buffer
    __alias_S__struct_S
  }
  S -> __struct_S
  [[block]] Struct{
    StructMember{field0: __u32}
    StructMember{field1: __f32}
    StructMember{field2: __array__u32_2}
  }
})")) << module_str;
}

TEST_F(SpvParserTest, ModuleScopeVar_ColMajorDecoration_Dropped) {
  auto* p = parser(test::Assemble(R"(
     OpName %myvar "myvar"
     OpDecorate %s Block
     OpMemberDecorate %s 0 ColMajor
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    myvar
    storage_buffer
    __alias_S__struct_S
  }
  S -> __struct_S
  [[block]] Struct{
    StructMember{field0: __mat_2_3__f32}
  }
})")) << module_str;
}

TEST_F(SpvParserTest, ModuleScopeVar_MatrixStrideDecoration_Dropped) {
  auto* p = parser(test::Assemble(R"(
     OpName %myvar "myvar"
     OpDecorate %s Block
     OpMemberDecorate %s 0 MatrixStride 8
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->module().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    myvar
    storage_buffer
    __alias_S__struct_S
  }
  S -> __struct_S
  [[block]] Struct{
    StructMember{field0: __mat_2_3__f32}
  }
})")) << module_str;
}

TEST_F(SpvParserTest, ModuleScopeVar_RowMajorDecoration_IsError) {
  auto* p = parser(test::Assemble(R"(
     OpName %myvar "myvar"
     OpDecorate %s Block
     OpMemberDecorate %s 0 RowMajor
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )"));
  EXPECT_FALSE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_THAT(
      p->error(),
      Eq(R"(WGSL does not support row-major matrices: can't translate member 0 of %2 = OpTypeStruct %5)"))
      << p->error();
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
