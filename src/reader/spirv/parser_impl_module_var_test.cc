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

using SpvModuleScopeVarParserTest = SpvParserTest;

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;

std::string Preamble() {
  return R"(
   OpCapability Shader
   OpMemoryModel Logical Simple
)";
}

std::string FragMain() {
  return R"(
   OpEntryPoint Fragment %main "main"
   OpExecutionMode %main OriginUpperLeft
)";
}

std::string MainBody() {
  return R"(
   %main = OpFunction %void None %voidfn
   %main_entry = OpLabel
   OpReturn
   OpFunctionEnd
)";
}

std::string CommonCapabilities() {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
)";
}

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

// Returns layout annotations for types in CommonTypes()
std::string CommonLayout() {
  return R"(
    OpMemberDecorate %strct 0 Offset 0
    OpMemberDecorate %strct 1 Offset 4
    OpMemberDecorate %strct 2 Offset 8
    OpDecorate %arr2uint ArrayStride 4
)";
}

TEST_F(SpvModuleScopeVarParserTest, NoVar) {
  auto assembly = Preamble() + FragMain() + CommonTypes() + MainBody();
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_ast = p->program().to_str();
  EXPECT_THAT(module_ast, Not(HasSubstr("Variable"))) << module_ast;
}

TEST_F(SpvModuleScopeVarParserTest, BadStorageClass_NotAWebGPUStorageClass) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer CrossWorkgroup %float
    %52 = OpVariable %ptr CrossWorkgroup
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));
  EXPECT_TRUE(p->BuildInternalModule());
  // Normally we should run ParserImpl::RegisterTypes before emitting
  // variables. But defensive coding in EmitModuleScopeVariables lets
  // us catch this error.
  EXPECT_FALSE(p->EmitModuleScopeVariables()) << p->error();
  EXPECT_THAT(p->error(), HasSubstr("unknown SPIR-V storage class: 5"));
}

TEST_F(SpvModuleScopeVarParserTest, BadStorageClass_Function) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Function %float
    %52 = OpVariable %ptr Function
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));
  EXPECT_TRUE(p->BuildInternalModule());
  // Normally we should run ParserImpl::RegisterTypes before emitting
  // variables. But defensive coding in EmitModuleScopeVariables lets
  // us catch this error.
  EXPECT_FALSE(p->EmitModuleScopeVariables()) << p->error();
  EXPECT_THAT(p->error(),
              HasSubstr("invalid SPIR-V storage class 7 for module scope "
                        "variable: %52 = OpVariable %3 Function"));
}

TEST_F(SpvModuleScopeVarParserTest, BadPointerType) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %fn_ty = OpTypeFunction %float
    %3 = OpTypePointer Private %fn_ty
    %52 = OpVariable %3 Private
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));
  EXPECT_TRUE(p->BuildInternalModule());
  // Normally we should run ParserImpl::RegisterTypes before emitting
  // variables. But defensive coding in EmitModuleScopeVariables lets
  // us catch this error.
  EXPECT_FALSE(p->EmitModuleScopeVariables());
  EXPECT_THAT(p->error(), HasSubstr("internal error: failed to register Tint "
                                    "AST type for SPIR-V type with ID: 3"));
}

TEST_F(SpvModuleScopeVarParserTest, NonPointerType) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %5 = OpTypeFunction %float
    %3 = OpTypePointer Private %5
    %52 = OpVariable %float Private
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));
  EXPECT_TRUE(p->BuildInternalModule());
  EXPECT_FALSE(p->RegisterTypes());
  EXPECT_THAT(
      p->error(),
      HasSubstr("SPIR-V pointer type with ID 3 has invalid pointee type 5"));
}

TEST_F(SpvModuleScopeVarParserTest, AnonWorkgroupVar) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Workgroup %float
    %52 = OpVariable %ptr Workgroup
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    x_52
    workgroup
    __f32
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, NamedWorkgroupVar) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    OpName %52 "the_counter"
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Workgroup %float
    %52 = OpVariable %ptr Workgroup
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    the_counter
    workgroup
    __f32
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, PrivateVar) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
    OpName %52 "my_own_private_idaho"
    %float = OpTypeFloat 32
    %ptr = OpTypePointer Private %float
    %52 = OpVariable %ptr Private
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    my_own_private_idaho
    private
    __f32
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinVertexIndex) {
  // This is the simple case for the vertex_index builtin,
  // where the SPIR-V uses the same store type as in WGSL.
  // See later for tests where the SPIR-V store type is signed
  // integer, as in GLSL.
  auto p = parser(test::Assemble(Preamble() + R"(
    OpEntryPoint Vertex %main "main" %52
    OpDecorate %52 BuiltIn VertexIndex
    %uint = OpTypeInt 32 0
    %ptr = OpTypePointer Input %uint
    %52 = OpVariable %ptr Input
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
  )" + MainBody()));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{vertex_index}
    }
    x_52
    in
    __u32
  })"));
}

std::string PerVertexPreamble() {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1

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

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_MapsToModuleScopeVec4Var) {
  // In Vulkan SPIR-V, Position is the first member of gl_PerVertex
  const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
  EXPECT_TRUE(p->error().empty()) << p->error();
  const auto& position_info = p->GetBuiltInPositionInfo();
  EXPECT_EQ(position_info.struct_type_id, 10u);
  EXPECT_EQ(position_info.position_member_index, 0u);
  EXPECT_EQ(position_info.position_member_type_id, 12u);
  EXPECT_EQ(position_info.pointer_type_id, 11u);
  EXPECT_EQ(position_info.storage_class, SpvStorageClassOutput);
  EXPECT_EQ(position_info.per_vertex_var_id, 1u);
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{position}
    }
    gl_Position
    out
    __vec_4__f32
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPosition_StoreWholeStruct_NotSupported) {
  // Glslang does not generate this code pattern.
  const std::string assembly = PerVertexPreamble() + R"(
  %nil = OpConstantNull %10 ; the whole struct

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpStore %1 %nil  ; store the whole struct
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
  EXPECT_THAT(p->error(), Eq("storing to the whole per-vertex structure is not "
                             "supported: OpStore %1 %13"))
      << p->error();
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPosition_IntermediateWholeStruct_NotSupported) {
  const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %1000 = OpUndef %10
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
  EXPECT_THAT(p->error(), Eq("operations producing a per-vertex structure are "
                             "not supported: %1000 = OpUndef %10"))
      << p->error();
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPosition_IntermediatePtrWholeStruct_NotSupported) {
  const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %1000 = OpUndef %11
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("operations producing a pointer to a per-vertex structure are "
                 "not supported: %1000 = OpUndef %11"))
      << p->error();
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_StorePosition) {
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
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
    Assignment{
      Identifier[not set]{gl_Position}
      TypeConstructor[not set]{
        __vec_4__f32
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPosition_StorePosition_PerVertexStructOutOfOrderDecl) {
  const std::string assembly = R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint Vertex %main "main" %1

 ;  scramble the member indices
  OpMemberDecorate %10 0 BuiltIn ClipDistance
  OpMemberDecorate %10 1 BuiltIn CullDistance
  OpMemberDecorate %10 2 BuiltIn Position
  OpMemberDecorate %10 3 BuiltIn PointSize
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %float = OpTypeFloat 32
  %12 = OpTypeVector %float 4
  %uint = OpTypeInt 32 0
  %uint_0 = OpConstant %uint 0
  %uint_1 = OpConstant %uint 1
  %uint_2 = OpConstant %uint 2
  %arr = OpTypeArray %float %uint_1
  %10 = OpTypeStruct %arr %arr %12 %float
  %11 = OpTypePointer Output %10
  %1 = OpVariable %11 Output

  %ptr_v4float = OpTypePointer Output %12
  %nil = OpConstantNull %12

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr_v4float %1 %uint_2 ; address of the Position member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
    Assignment{
      Identifier[not set]{gl_Position}
      TypeConstructor[not set]{
        __vec_4__f32
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPosition_StorePositionMember_OneAccessChain) {
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
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{gl_Position}
        Identifier[not set]{y}
      }
      ScalarConstructor[not set]{0.000000}
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPosition_StorePositionMember_TwoAccessChain) {
  // The algorithm is smart enough to collapse it down.
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %12
  %ptr_float = OpTypePointer Output %float
  %nil = OpConstantNull %float

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_0 ; address of the Position member
  %101 = OpAccessChain %ptr_float %100 %uint_1 ; address of the Position.y member
  OpStore %101 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  {
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{gl_Position}
        Identifier[not set]{y}
      }
      ScalarConstructor[not set]{0.000000}
    }
    Return{}
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Write1_IsErased) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %one = OpConstant %float 1.0

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_1 ; address of the PointSize member
  OpStore %100 %one
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_EQ(module_str, R"(Module{
  Variable{
    Decorations{
      BuiltinDecoration{position}
    }
    gl_Position
    out
    __vec_4__f32
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Return{}
  }
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_WriteNon1_IsError) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %999 = OpConstant %float 2.0

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_1 ; address of the PointSize member
  OpStore %100 %999
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              HasSubstr("cannot store a value other than constant 1.0 to "
                        "PointSize builtin: OpStore %100 %999"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_ReadReplaced) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %nil = OpConstantNull %12
  %private_ptr = OpTypePointer Private %float
  %900 = OpVariable %private_ptr Private

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_1 ; address of the PointSize member
  %99 = OpLoad %float %100
  OpStore %900 %99
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_EQ(module_str, R"(Module{
  Variable{
    x_900
    private
    __f32
  }
  Variable{
    Decorations{
      BuiltinDecoration{position}
    }
    gl_Position
    out
    __vec_4__f32
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Assignment{
      Identifier[not set]{x_900}
      ScalarConstructor[not set]{1.000000}
    }
    Return{}
  }
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPointSize_WriteViaCopyObjectPriorAccess_Unsupported) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %nil = OpConstantNull %12

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %20 = OpCopyObject %11 %1
  %100 = OpAccessChain %20 %1 %uint_1 ; address of the PointSize member
  OpStore %100 %nil
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule()) << p->error();
  EXPECT_THAT(
      p->error(),
      HasSubstr("operations producing a pointer to a per-vertex structure are "
                "not supported: %20 = OpCopyObject %11 %1"));
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPointSize_WriteViaCopyObjectPostAccessChainErased) {
  const std::string assembly = PerVertexPreamble() + R"(
  %ptr = OpTypePointer Output %float
  %one = OpConstant %float 1.0

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %ptr %1 %uint_1 ; address of the PointSize member
  %101 = OpCopyObject %ptr %100
  OpStore %101 %one
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_EQ(module_str, R"(Module{
  Variable{
    Decorations{
      BuiltinDecoration{position}
    }
    gl_Position
    out
    __vec_4__f32
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Return{}
  }
}
)") << module_str;
}

std::string LoosePointSizePreamble(std::string stage = "Vertex") {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint )" +
         stage + R"( %500 "main" %1
)" + (stage == "Fragment" ? "OpExecutionMode %500 OriginUpperLeft" : "") +
         R"(
    OpDecorate %1 BuiltIn PointSize
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %11 = OpTypePointer Output %float
    %1 = OpVariable %11 Output
)";
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Loose_Write1_IsErased) {
  const std::string assembly = LoosePointSizePreamble() + R"(
  %ptr = OpTypePointer Output %float
  %one = OpConstant %float 1.0

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  OpStore %1 %one
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_EQ(module_str, R"(Module{
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Return{}
  }
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPointSize_Loose_WriteNon1_IsError) {
  const std::string assembly = LoosePointSizePreamble() + R"(
  %ptr = OpTypePointer Output %float
  %999 = OpConstant %float 2.0

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  OpStore %1 %999
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              HasSubstr("cannot store a value other than constant 1.0 to "
                        "PointSize builtin: OpStore %1 %999"));
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPointSize_Loose_ReadReplaced_Vertex) {
  const std::string assembly = LoosePointSizePreamble() + R"(
  %ptr = OpTypePointer Private %float
  %900 = OpVariable %ptr Private

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  %99 = OpLoad %float %1
  OpStore %900 %99
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_EQ(module_str, R"(Module{
  Variable{
    x_900
    private
    __f32
  }
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Assignment{
      Identifier[not set]{x_900}
      ScalarConstructor[not set]{1.000000}
    }
    Return{}
  }
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPointSize_Loose_ReadReplaced_Fragment) {
  const std::string assembly = LoosePointSizePreamble("Fragment") + R"(
  %ptr = OpTypePointer Private %float
  %900 = OpVariable %ptr Private

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  %99 = OpLoad %float %1
  OpStore %900 %99
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  // This example is invalid because you PointSize is not valid in Vulkan
  // Fragment shaders.
  EXPECT_FALSE(p->Parse());
  EXPECT_FALSE(p->success());
  EXPECT_THAT(p->error(), HasSubstr("VUID-PointSize-PointSize-04314"));
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPointSize_Loose_WriteViaCopyObjectPriorAccess_Erased) {
  const std::string assembly = LoosePointSizePreamble() + R"(
  %one = OpConstant %float 1.0

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  %20 = OpCopyObject %11 %1
  %100 = OpAccessChain %11 %20
  OpStore %100 %one
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_EQ(module_str, R"(Module{
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Return{}
  }
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPointSize_Loose_WriteViaCopyObjectPostAccessChainErased) {
  const std::string assembly = LoosePointSizePreamble() + R"(
  %one = OpConstant %float 1.0

  %500 = OpFunction %void None %voidfn
  %entry = OpLabel
  %100 = OpAccessChain %11 %1
  %101 = OpCopyObject %11 %100
  OpStore %101 %one
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
  EXPECT_TRUE(p->error().empty()) << p->error();
  const auto module_str = p->program().to_str();
  EXPECT_EQ(module_str, R"(Module{
  Function main -> __void
  StageDecoration{vertex}
  ()
  {
    Return{}
  }
}
)") << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinClipDistance_NotSupported) {
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
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_EQ(p->error(),
            "accessing per-vertex member 2 is not supported. Only Position is "
            "supported, and PointSize is ignored");
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinCullDistance_NotSupported) {
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
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_EQ(p->error(),
            "accessing per-vertex member 3 is not supported. Only Position is "
            "supported, and PointSize is ignored");
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPerVertex_MemberIndex_NotConstant) {
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
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("first index of access chain into per-vertex structure is not "
                 "a constant: %100 = OpAccessChain %13 %1 %16"));
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPerVertex_MemberIndex_NotConstantInteger) {
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
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("first index of access chain into per-vertex structure is not "
                 "a constant integer: %100 = OpAccessChain %13 %1 %14"));
}

TEST_F(SpvModuleScopeVarParserTest, ScalarInitializers) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %1 = OpVariable %ptr_bool Private %true
     %2 = OpVariable %ptr_bool Private %false
     %3 = OpVariable %ptr_int Private %int_m1
     %4 = OpVariable %ptr_uint Private %uint_1
     %5 = OpVariable %ptr_float Private %float_1p5
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_1
    private
    __bool
    {
      ScalarConstructor[not set]{true}
    }
  }
  Variable{
    x_2
    private
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
  Variable{
    x_3
    private
    __i32
    {
      ScalarConstructor[not set]{-1}
    }
  }
  Variable{
    x_4
    private
    __u32
    {
      ScalarConstructor[not set]{1u}
    }
  }
  Variable{
    x_5
    private
    __f32
    {
      ScalarConstructor[not set]{1.500000}
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, ScalarNullInitializers) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %null_bool = OpConstantNull %bool
     %null_int = OpConstantNull %int
     %null_uint = OpConstantNull %uint
     %null_float = OpConstantNull %float

     %1 = OpVariable %ptr_bool Private %null_bool
     %2 = OpVariable %ptr_int Private %null_int
     %3 = OpVariable %ptr_uint Private %null_uint
     %4 = OpVariable %ptr_float Private %null_float
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_1
    private
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
  Variable{
    x_2
    private
    __i32
    {
      ScalarConstructor[not set]{0}
    }
  }
  Variable{
    x_3
    private
    __u32
    {
      ScalarConstructor[not set]{0u}
    }
  }
  Variable{
    x_4
    private
    __f32
    {
      ScalarConstructor[not set]{0.000000}
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, ScalarUndefInitializers) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %undef_bool = OpUndef %bool
     %undef_int = OpUndef %int
     %undef_uint = OpUndef %uint
     %undef_float = OpUndef %float

     %1 = OpVariable %ptr_bool Private %undef_bool
     %2 = OpVariable %ptr_int Private %undef_int
     %3 = OpVariable %ptr_uint Private %undef_uint
     %4 = OpVariable %ptr_float Private %undef_float
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_1
    private
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
  Variable{
    x_2
    private
    __i32
    {
      ScalarConstructor[not set]{0}
    }
  }
  Variable{
    x_3
    private
    __u32
    {
      ScalarConstructor[not set]{0u}
    }
  }
  Variable{
    x_4
    private
    __f32
    {
      ScalarConstructor[not set]{0.000000}
    }
  })"));

  // This example module emits ok, but is not valid SPIR-V in the first place.
  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, VectorInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %two = OpConstant %float 2.0
     %const = OpConstantComposite %v2float %float_1p5 %two
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__f32
    {
      TypeConstructor[not set]{
        __vec_2__f32
        ScalarConstructor[not set]{1.500000}
        ScalarConstructor[not set]{2.000000}
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorBoolNullInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2bool
     %const = OpConstantNull %v2bool
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__bool
    {
      TypeConstructor[not set]{
        __vec_2__bool
        ScalarConstructor[not set]{false}
        ScalarConstructor[not set]{false}
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorBoolUndefInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2bool
     %const = OpUndef %v2bool
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__bool
    {
      TypeConstructor[not set]{
        __vec_2__bool
        ScalarConstructor[not set]{false}
        ScalarConstructor[not set]{false}
      }
    }
  })"));

  // This example module emits ok, but is not valid SPIR-V in the first place.
  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, VectorUintNullInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2uint
     %const = OpConstantNull %v2uint
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__u32
    {
      TypeConstructor[not set]{
        __vec_2__u32
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorUintUndefInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2uint
     %const = OpUndef %v2uint
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__u32
    {
      TypeConstructor[not set]{
        __vec_2__u32
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  })"));

  // This example module emits ok, but is not valid SPIR-V in the first place.
  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, VectorIntNullInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2int
     %const = OpConstantNull %v2int
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__i32
    {
      TypeConstructor[not set]{
        __vec_2__i32
        ScalarConstructor[not set]{0}
        ScalarConstructor[not set]{0}
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorIntUndefInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2int
     %const = OpUndef %v2int
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__i32
    {
      TypeConstructor[not set]{
        __vec_2__i32
        ScalarConstructor[not set]{0}
        ScalarConstructor[not set]{0}
      }
    }
  })"));

  // This example module emits ok, but is not valid SPIR-V in the first place.
  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, VectorFloatNullInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %const = OpConstantNull %v2float
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__f32
    {
      TypeConstructor[not set]{
        __vec_2__f32
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, VectorFloatUndefInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %v2float
     %const = OpUndef %v2float
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __vec_2__f32
    {
      TypeConstructor[not set]{
        __vec_2__f32
        ScalarConstructor[not set]{0.000000}
        ScalarConstructor[not set]{0.000000}
      }
    }
  })"));

  // This example module emits ok, but is not valid SPIR-V in the first place.
  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, MatrixInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %two = OpConstant %float 2.0
     %three = OpConstant %float 3.0
     %four = OpConstant %float 4.0
     %v0 = OpConstantComposite %v2float %float_1p5 %two
     %v1 = OpConstantComposite %v2float %two %three
     %v2 = OpConstantComposite %v2float %three %four
     %const = OpConstantComposite %m3v2float %v0 %v1 %v2
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __mat_2_3__f32
    {
      TypeConstructor[not set]{
        __mat_2_3__f32
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{1.500000}
          ScalarConstructor[not set]{2.000000}
        }
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{2.000000}
          ScalarConstructor[not set]{3.000000}
        }
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{3.000000}
          ScalarConstructor[not set]{4.000000}
        }
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, MatrixNullInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %const = OpConstantNull %m3v2float
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __mat_2_3__f32
    {
      TypeConstructor[not set]{
        __mat_2_3__f32
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
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{0.000000}
          ScalarConstructor[not set]{0.000000}
        }
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, MatrixUndefInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %m3v2float
     %const = OpUndef %m3v2float
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __mat_2_3__f32
    {
      TypeConstructor[not set]{
        __mat_2_3__f32
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
        TypeConstructor[not set]{
          __vec_2__f32
          ScalarConstructor[not set]{0.000000}
          ScalarConstructor[not set]{0.000000}
        }
      }
    }
  })"));

  // This example module emits ok, but is not valid SPIR-V in the first place.
  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, ArrayInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %two = OpConstant %uint 2
     %const = OpConstantComposite %arr2uint %uint_1 %two
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __array__u32_2
    {
      TypeConstructor[not set]{
        __array__u32_2
        ScalarConstructor[not set]{1u}
        ScalarConstructor[not set]{2u}
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, ArrayNullInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %const = OpConstantNull %arr2uint
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __array__u32_2
    {
      TypeConstructor[not set]{
        __array__u32_2
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, ArrayUndefInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %arr2uint
     %const = OpUndef %arr2uint
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __array__u32_2
    {
      TypeConstructor[not set]{
        __array__u32_2
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0u}
      }
    }
  })"));

  // This example module emits ok, but is not valid SPIR-V in the first place.
  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, StructInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %two = OpConstant %uint 2
     %arrconst = OpConstantComposite %arr2uint %uint_1 %two
     %const = OpConstantComposite %strct %uint_1 %float_1p5 %arrconst
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __type_name_S
    {
      TypeConstructor[not set]{
        __type_name_S
        ScalarConstructor[not set]{1u}
        ScalarConstructor[not set]{1.500000}
        TypeConstructor[not set]{
          __array__u32_2
          ScalarConstructor[not set]{1u}
          ScalarConstructor[not set]{2u}
        }
      }
    }
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StructNullInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %const = OpConstantNull %strct
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __type_name_S
    {
      TypeConstructor[not set]{
        __type_name_S
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0.000000}
        TypeConstructor[not set]{
          __array__u32_2
          ScalarConstructor[not set]{0u}
          ScalarConstructor[not set]{0u}
        }
      }
    }
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StructUndefInitializer) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() + R"(
     %ptr = OpTypePointer Private %strct
     %const = OpUndef %strct
     %200 = OpVariable %ptr Private %const
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());

  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(Variable{
    x_200
    private
    __type_name_S
    {
      TypeConstructor[not set]{
        __type_name_S
        ScalarConstructor[not set]{0u}
        ScalarConstructor[not set]{0.000000}
        TypeConstructor[not set]{
          __array__u32_2
          ScalarConstructor[not set]{0u}
          ScalarConstructor[not set]{0u}
        }
      }
    }
  })"))
      << module_str;

  // This example module emits ok, but is not valid SPIR-V in the first place.
  p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvModuleScopeVarParserTest, LocationDecoration_Valid) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Location 3
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Input %uint
     %myvar = OpVariable %ptr Input
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      LocationDecoration{3}
    }
    myvar
    in
    __u32
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       LocationDecoration_MissingOperandWontAssemble) {
  const auto assembly = Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Location
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Input %uint
     %myvar = OpVariable %ptr Input
  )" + MainBody();
  EXPECT_THAT(test::AssembleFailure(assembly),
              Eq("10:4: Expected operand, found next instruction instead."));
}

TEST_F(SpvModuleScopeVarParserTest,
       LocationDecoration_TwoOperandsWontAssemble) {
  const auto assembly = Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Location 3 4
)" + CommonTypes() + R"(
     %ptr = OpTypePointer Input %uint
     %myvar = OpVariable %ptr Input
  )" + MainBody();
  EXPECT_THAT(
      test::AssembleFailure(assembly),
      Eq("8:34: Expected <opcode> or <result-id> at the beginning of an "
         "instruction, found '4'."));
}

TEST_F(SpvModuleScopeVarParserTest, DescriptorGroupDecoration_Valid) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonLayout() + R"(
     OpDecorate %1 DescriptorSet 3
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %1 = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      GroupDecoration{3}
    }
    x_1
    storage
    __access_control_read_write__type_name_S
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       DescriptorGroupDecoration_MissingOperandWontAssemble) {
  const auto assembly = Preamble() + FragMain() + CommonLayout() + R"(
     OpDecorate %1 DescriptorSet
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %1 = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody();
  EXPECT_THAT(test::AssembleFailure(assembly),
              Eq("13:5: Expected operand, found next instruction instead."));
}

TEST_F(SpvModuleScopeVarParserTest,
       DescriptorGroupDecoration_TwoOperandsWontAssemble) {
  const auto assembly = Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 3 4
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody();
  EXPECT_THAT(
      test::AssembleFailure(assembly),
      Eq("8:39: Expected <opcode> or <result-id> at the beginning of an "
         "instruction, found '4'."));
}

TEST_F(SpvModuleScopeVarParserTest, BindingDecoration_Valid) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 Binding 3
     OpDecorate %strct Block
)" + CommonLayout() + CommonTypes() +
                                 R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %1 = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BindingDecoration{3}
    }
    x_1
    storage
    __access_control_read_write__type_name_S
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BindingDecoration_MissingOperandWontAssemble) {
  const auto assembly = Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Binding
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody();
  EXPECT_THAT(test::AssembleFailure(assembly),
              Eq("9:5: Expected operand, found next instruction instead."));
}

TEST_F(SpvModuleScopeVarParserTest, BindingDecoration_TwoOperandsWontAssemble) {
  const auto assembly = Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Binding 3 4
     OpDecorate %strct Block
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %myvar = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody();
  EXPECT_THAT(
      test::AssembleFailure(assembly),
      Eq("8:33: Expected <opcode> or <result-id> at the beginning of an "
         "instruction, found '4'."));
}

TEST_F(SpvModuleScopeVarParserTest,
       StructMember_NonReadableDecoration_Dropped) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonLayout() + R"(
     OpDecorate %strct Block
     OpMemberDecorate %strct 0 NonReadable
)" + CommonTypes() + R"(
     %ptr_sb_strct = OpTypePointer StorageBuffer %strct
     %1 = OpVariable %ptr_sb_strct StorageBuffer
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Arr -> __array__u32_2_stride_4
  Struct S {
    [[block]]
    StructMember{[[ offset 0 ]] field0: __u32}
    StructMember{[[ offset 4 ]] field1: __f32}
    StructMember{[[ offset 8 ]] field2: __type_name_Arr}
  }
  Variable{
    x_1
    storage
    __access_control_read_write__type_name_S
  }
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ColMajorDecoration_Dropped) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %s Block
     OpMemberDecorate %s 0 ColMajor
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 0 MatrixStride 8
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Struct S {
    [[block]]
    StructMember{[[ offset 0 ]] field0: __mat_2_3__f32}
  }
  Variable{
    myvar
    storage
    __access_control_read_write__type_name_S
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, MatrixStrideDecoration_Dropped) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %s Block
     OpMemberDecorate %s 0 MatrixStride 8
     OpMemberDecorate %s 0 Offset 0
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Struct S {
    [[block]]
    StructMember{[[ offset 0 ]] field0: __mat_2_3__f32}
  }
  Variable{
    myvar
    storage
    __access_control_read_write__type_name_S
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, RowMajorDecoration_IsError) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %s Block
     OpMemberDecorate %s 0 RowMajor
     OpMemberDecorate %s 0 Offset 0
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %v2float = OpTypeVector %float 2
     %m3v2float = OpTypeMatrix %v2float 3

     %s = OpTypeStruct %m3v2float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %myvar = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
  EXPECT_FALSE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_THAT(
      p->error(),
      Eq(R"(WGSL does not support row-major matrices: can't translate member 0 of %3 = OpTypeStruct %8)"))
      << p->error();
}

TEST_F(SpvModuleScopeVarParserTest, StorageBuffer_NonWritable_AllMembers) {
  // Variable should have access(read)
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %s Block
     OpMemberDecorate %s 0 NonWritable
     OpMemberDecorate %s 1 NonWritable
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 1 Offset 4
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32

     %s = OpTypeStruct %float %float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %1 = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Struct S {
    [[block]]
    StructMember{[[ offset 0 ]] field0: __f32}
    StructMember{[[ offset 4 ]] field1: __f32}
  }
  Variable{
    x_1
    storage
    __access_control_read_only__type_name_S
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StorageBuffer_NonWritable_NotAllMembers) {
  // Variable should have access(read_write)
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %s Block
     OpMemberDecorate %s 0 NonWritable
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 1 Offset 4
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32

     %s = OpTypeStruct %float %float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %1 = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Struct S {
    [[block]]
    StructMember{[[ offset 0 ]] field0: __f32}
    StructMember{[[ offset 4 ]] field1: __f32}
  }
  Variable{
    x_1
    storage
    __access_control_read_write__type_name_S
  }
})")) << module_str;
}

TEST_F(
    SpvModuleScopeVarParserTest,
    StorageBuffer_NonWritable_NotAllMembers_DuplicatedOnSameMember) {  // NOLINT
  // Variable should have access(read_write)
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %s Block
     OpMemberDecorate %s 0 NonWritable
     OpMemberDecorate %s 0 NonWritable ; same member. Don't double-count it
     OpMemberDecorate %s 0 Offset 0
     OpMemberDecorate %s 1 Offset 4
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32

     %s = OpTypeStruct %float %float
     %ptr_sb_s = OpTypePointer StorageBuffer %s
     %1 = OpVariable %ptr_sb_s StorageBuffer
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Struct S {
    [[block]]
    StructMember{[[ offset 0 ]] field0: __f32}
    StructMember{[[ offset 4 ]] field1: __f32}
  }
  Variable{
    x_1
    storage
    __access_control_read_write__type_name_S
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_True) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %bool = OpTypeBool
     %c = OpSpecConstantTrue %bool
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  VariableConst{
    Decorations{
      OverrideDecoration{12}
    }
    myconst
    none
    __bool
    {
      ScalarConstructor[not set]{true}
    }
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_False) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %bool = OpTypeBool
     %c = OpSpecConstantFalse %bool
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  VariableConst{
    Decorations{
      OverrideDecoration{12}
    }
    myconst
    none
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_U32) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %uint = OpTypeInt 32 0
     %c = OpSpecConstant %uint 42
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  VariableConst{
    Decorations{
      OverrideDecoration{12}
    }
    myconst
    none
    __u32
    {
      ScalarConstructor[not set]{42u}
    }
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_I32) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %int = OpTypeInt 32 1
     %c = OpSpecConstant %int 42
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  VariableConst{
    Decorations{
      OverrideDecoration{12}
    }
    myconst
    none
    __i32
    {
      ScalarConstructor[not set]{42}
    }
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_F32) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     OpDecorate %c SpecId 12
     %float = OpTypeFloat 32
     %c = OpSpecConstant %float 2.5
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  VariableConst{
    Decorations{
      OverrideDecoration{12}
    }
    myconst
    none
    __f32
    {
      ScalarConstructor[not set]{2.500000}
    }
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       ScalarSpecConstant_DeclareConst_F32_WithoutSpecId) {
  // When we don't have a spec ID, declare an undecorated module-scope constant.
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %c "myconst"
     %float = OpTypeFloat 32
     %c = OpSpecConstant %float 2.5
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  VariableConst{
    myconst
    none
    __f32
    {
      ScalarConstructor[not set]{2.500000}
    }
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_UsedInFunction) {
  const auto assembly = Preamble() + FragMain() + R"(
     OpName %c "myconst"
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %float = OpTypeFloat 32
     %c = OpSpecConstant %float 2.5
     %floatfn = OpTypeFunction %float
     %100 = OpFunction %float None %floatfn
     %entry = OpLabel
     %1 = OpFAdd %float %c %c
     OpReturnValue %1
     OpFunctionEnd
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  auto fe = p->function_emitter(100);
  EXPECT_TRUE(fe.EmitBody()) << p->error();
  EXPECT_TRUE(p->error().empty());

  Program program = p->program();
  const auto got = ToString(program, fe.ast_body());

  EXPECT_THAT(got, HasSubstr(R"(Return{
  {
    Binary[not set]{
      Identifier[not set]{myconst}
      add
      Identifier[not set]{myconst}
    }
  }
})")) << got;
}

// Returns the start of a shader for testing SampleId,
// parameterized by store type of %int or %uint
std::string SampleIdPreamble(std::string store_type) {
  return R"(
    OpCapability Shader
    OpCapability SampleRateShading
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "main" %1
    OpExecutionMode %main OriginUpperLeft
    OpDecorate %1 BuiltIn SampleId
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %ptr_ty = OpTypePointer Input )" +
         store_type + R"(
    %1 = OpVariable %ptr_ty Input
)";
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_I32_Load_Direct) {
  const std::string assembly = SampleIdPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %int %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_I32_Load_CopyObject) {
  const std::string assembly = SampleIdPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_I32_Load_AccessChain) {
  const std::string assembly = SampleIdPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_I32_FunctParam) {
  const std::string assembly = SampleIdPreamble("%int") + R"(
    %helper_ty = OpTypeFunction %int %ptr_ty
    %helper = OpFunction %int None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %int %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %int %helper %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));

  // This example is invalid because you can't pass pointer-to-Input
  // as a function parameter.
  EXPECT_FALSE(p->Parse());
  EXPECT_FALSE(p->success());
  EXPECT_THAT(p->error(),
              HasSubstr("Invalid storage class for pointer operand 1"));
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_U32_Load_Direct) {
  const std::string assembly = SampleIdPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %uint %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_U32_Load_CopyObject) {
  const std::string assembly = SampleIdPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_11
        none
        __ptr_in__u32
        {
          UnaryOp[not set]{
            address-of
            Identifier[not set]{x_1}
          }
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_11}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_U32_Load_AccessChain) {
  const std::string assembly = SampleIdPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleId_U32_FunctParam) {
  const std::string assembly = SampleIdPreamble("%uint") + R"(
    %helper_ty = OpTypeFunction %uint %ptr_ty
    %helper = OpFunction %uint None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %uint %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %uint %helper %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  // This example is invalid because you can't pass pointer-to-Input
  // as a function parameter.
  EXPECT_FALSE(p->Parse());
  EXPECT_THAT(p->error(),
              HasSubstr("Invalid storage class for pointer operand 1"));
}

// Returns the start of a shader for testing SampleMask
// parameterized by store type.
std::string SampleMaskPreamble(std::string store_type) {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "main" %1
    OpExecutionMode %main OriginUpperLeft
    OpDecorate %1 BuiltIn SampleMask
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %int_12 = OpConstant %int 12
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2
    %uarr1 = OpTypeArray %uint %uint_1
    %uarr2 = OpTypeArray %uint %uint_2
    %iarr1 = OpTypeArray %int %uint_1
    %iarr2 = OpTypeArray %int %uint_2
    %iptr_in_ty = OpTypePointer Input %int
    %uptr_in_ty = OpTypePointer Input %uint
    %iptr_out_ty = OpTypePointer Output %int
    %uptr_out_ty = OpTypePointer Output %uint
    %in_ty = OpTypePointer Input )" +
         store_type + R"(
    %out_ty = OpTypePointer Output )" +
         store_type + R"(
)";
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_ArraySize2_Error) {
  const std::string assembly = SampleMaskPreamble("%uarr2") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpLoad %int %2
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              HasSubstr("WGSL supports a sample mask of at most 32 bits. "
                        "SampleMask must be an array of 1 element"))
      << p->error() << assembly;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_U32_Direct) {
  const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpLoad %uint %2
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    in
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_U32_CopyObject) {
  const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpCopyObject %uptr_in_ty %2
    %4 = OpLoad %uint %3
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    in
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_U32_AccessChain) {
  const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_in_ty %1 %uint_0
    %3 = OpAccessChain %uptr_in_ty %2
    %4 = OpLoad %uint %3
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    in
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_I32_Direct) {
  const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_in_ty %1 %uint_0
    %3 = OpLoad %int %2
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    in
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_I32_CopyObject) {
  const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_in_ty %1 %uint_0
    %3 = OpCopyObject %iptr_in_ty %2
    %4 = OpLoad %int %3
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    in
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_I32_AccessChain) {
  const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %in_ty Input

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_in_ty %1 %uint_0
    %3 = OpAccessChain %iptr_in_ty %2
    %4 = OpLoad %int %3
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    in
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_ArraySize2_Error) {
  const std::string assembly = SampleMaskPreamble("%uarr2") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              HasSubstr("WGSL supports a sample mask of at most 32 bits. "
                        "SampleMask must be an array of 1 element"))
      << p->error() << assembly;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_U32_Direct) {
  const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    out
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    Assignment{
      Identifier[not set]{x_1}
      ScalarConstructor[not set]{0u}
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_U32_CopyObject) {
  const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    %3 = OpCopyObject %uptr_out_ty %2
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    out
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    Assignment{
      Identifier[not set]{x_1}
      ScalarConstructor[not set]{0u}
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_U32_AccessChain) {
  const std::string assembly = SampleMaskPreamble("%uarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %uptr_out_ty %1 %uint_0
    %3 = OpAccessChain %uptr_out_ty %2
    OpStore %2 %uint_0
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    out
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
    Assignment{
      Identifier[not set]{x_1}
      ScalarConstructor[not set]{0u}
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_I32_Direct) {
  const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_out_ty %1 %uint_0
    OpStore %2 %int_12
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    out
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
  {
    Assignment{
      Identifier[not set]{x_1}
      TypeConstructor[not set]{
        __u32
        ScalarConstructor[not set]{12}
      }
    }
    Return{}
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_I32_CopyObject) {
  const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_out_ty %1 %uint_0
    %3 = OpCopyObject %iptr_out_ty %2
    OpStore %2 %int_12
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    out
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
  {
    Assignment{
      Identifier[not set]{x_1}
      TypeConstructor[not set]{
        __u32
        ScalarConstructor[not set]{12}
      }
    }
    Return{}
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_I32_AccessChain) {
  const std::string assembly = SampleMaskPreamble("%iarr1") + R"(
    %1 = OpVariable %out_ty Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpAccessChain %iptr_out_ty %1 %uint_0
    %3 = OpAccessChain %iptr_out_ty %2
    OpStore %2 %int_12
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{sample_mask}
    }
    x_1
    out
    __u32
  })"));

  // Correct bodies
  EXPECT_THAT(module_str, HasSubstr(R"(
  {
    Assignment{
      Identifier[not set]{x_1}
      TypeConstructor[not set]{
        __u32
        ScalarConstructor[not set]{12}
      }
    }
    Return{}
  })"))
      << module_str;
}

// Returns the start of a shader for testing VertexIndex,
// parameterized by store type of %int or %uint
std::string VertexIndexPreamble(std::string store_type) {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1
    OpDecorate %1 BuiltIn VertexIndex
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %ptr_ty = OpTypePointer Input )" +
         store_type + R"(
    %1 = OpVariable %ptr_ty Input
)";
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_I32_Load_Direct) {
  const std::string assembly = VertexIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %int %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{vertex_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_I32_Load_CopyObject) {
  const std::string assembly = VertexIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{vertex_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_I32_Load_AccessChain) {
  const std::string assembly = VertexIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{vertex_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_I32_FunctParam) {
  const std::string assembly = VertexIndexPreamble("%int") + R"(
    %helper_ty = OpTypeFunction %int %ptr_ty
    %helper = OpFunction %int None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %int %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %int %helper %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  // TODO(dneto): We can handle this if we make a shadow variable and mutate
  // the parameter type.
  ASSERT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(
      p->error(),
      HasSubstr(
          "unhandled use of a pointer to the VertexIndex builtin, with ID: 1"));
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_U32_Load_Direct) {
  const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %uint %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{vertex_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_U32_Load_CopyObject) {
  const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{vertex_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_11
        none
        __ptr_in__u32
        {
          UnaryOp[not set]{
            address-of
            Identifier[not set]{x_1}
          }
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_11}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_U32_Load_AccessChain) {
  const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{vertex_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, VertexIndex_U32_FunctParam) {
  const std::string assembly = VertexIndexPreamble("%uint") + R"(
    %helper_ty = OpTypeFunction %uint %ptr_ty
    %helper = OpFunction %uint None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %uint %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %uint %helper %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));

  // This example is invalid because you can't pass pointer-to-Input
  // as a function parameter.
  EXPECT_FALSE(p->Parse());
  EXPECT_THAT(p->error(),
              HasSubstr("Invalid storage class for pointer operand 1"));
}

// Returns the start of a shader for testing InstanceIndex,
// parameterized by store type of %int or %uint
std::string InstanceIndexPreamble(std::string store_type) {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1
    OpDecorate %1 BuiltIn InstanceIndex
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %ptr_ty = OpTypePointer Input )" +
         store_type + R"(
    %1 = OpVariable %ptr_ty Input
)";
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_I32_Load_Direct) {
  const std::string assembly = InstanceIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %int %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{instance_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_I32_Load_CopyObject) {
  const std::string assembly = InstanceIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{instance_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_I32_Load_AccessChain) {
  const std::string assembly = InstanceIndexPreamble("%int") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %int %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{instance_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __i32
        {
          TypeConstructor[not set]{
            __i32
            Identifier[not set]{x_1}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_I32_FunctParam) {
  const std::string assembly = InstanceIndexPreamble("%int") + R"(
    %helper_ty = OpTypeFunction %int %ptr_ty
    %helper = OpFunction %int None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %int %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %int %helper %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  // This example is invalid because you can't pass pointer-to-Input
  // as a function parameter.
  EXPECT_FALSE(p->Parse());
  EXPECT_THAT(p->error(),
              HasSubstr("Invalid storage class for pointer operand 1"));
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_U32_Load_Direct) {
  const std::string assembly = InstanceIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad %uint %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{instance_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_U32_Load_CopyObject) {
  const std::string assembly = InstanceIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpCopyObject %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{instance_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_11
        none
        __ptr_in__u32
        {
          UnaryOp[not set]{
            address-of
            Identifier[not set]{x_1}
          }
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_11}
          }
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_U32_Load_AccessChain) {
  const std::string assembly = InstanceIndexPreamble("%uint") + R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %copy_ptr = OpAccessChain %ptr_ty %1
    %2 = OpLoad %uint %copy_ptr
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    Decorations{
      BuiltinDecoration{instance_index}
    }
    x_1
    in
    __u32
  })"));

  // Correct body
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, InstanceIndex_U32_FunctParam) {
  const std::string assembly = InstanceIndexPreamble("%uint") + R"(
    %helper_ty = OpTypeFunction %uint %ptr_ty
    %helper = OpFunction %uint None %helper_ty
    %param = OpFunctionParameter %ptr_ty
    %helper_entry = OpLabel
    %3 = OpLoad %uint %param
    OpReturnValue %3
    OpFunctionEnd

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %result = OpFunctionCall %uint %helper %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  // This example is invalid because you can't pass pointer-to-Input
  // as a function parameter.
  EXPECT_FALSE(p->Parse());
  EXPECT_THAT(p->error(),
              HasSubstr("Invalid storage class for pointer operand 1"));
}

TEST_F(SpvModuleScopeVarParserTest, RegisterInputOutputVars) {
  const std::string assembly =
      R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint GLCompute %1000 "w1000"
    OpEntryPoint GLCompute %1100 "w1100" %1
    OpEntryPoint GLCompute %1200 "w1200" %2 %15
    ; duplication is tolerated prior to SPIR-V 1.4
    OpEntryPoint GLCompute %1300 "w1300" %1 %15 %2 %1
    OpExecutionMode %1000 LocalSize 1 1 1
    OpExecutionMode %1100 LocalSize 1 1 1
    OpExecutionMode %1200 LocalSize 1 1 1
    OpExecutionMode %1300 LocalSize 1 1 1

)" + CommonTypes() +
      R"(

    %ptr_in_uint = OpTypePointer Input %uint
    %ptr_out_uint = OpTypePointer Output %uint

    %1 = OpVariable %ptr_in_uint Input
    %2 = OpVariable %ptr_in_uint Input
    %5 = OpVariable %ptr_in_uint Input
    %11 = OpVariable %ptr_out_uint Output
    %12 = OpVariable %ptr_out_uint Output
    %15 = OpVariable %ptr_out_uint Output

    %100 = OpFunction %void None %voidfn
    %entry_100 = OpLabel
    %load_100 = OpLoad %uint %1
    OpReturn
    OpFunctionEnd

    %200 = OpFunction %void None %voidfn
    %entry_200 = OpLabel
    %load_200 = OpLoad %uint %2
    OpStore %15 %load_200
    OpStore %15 %load_200
    OpReturn
    OpFunctionEnd

    %300 = OpFunction %void None %voidfn
    %entry_300 = OpLabel
    %dummy_300_1 = OpFunctionCall %void %100
    %dummy_300_2 = OpFunctionCall %void %200
    OpReturn
    OpFunctionEnd

    ; Call nothing
    %1000 = OpFunction %void None %voidfn
    %entry_1000 = OpLabel
    OpReturn
    OpFunctionEnd

    ; Call %100
    %1100 = OpFunction %void None %voidfn
    %entry_1100 = OpLabel
    %dummy_1100_1 = OpFunctionCall %void %100
    OpReturn
    OpFunctionEnd

    ; Call %200
    %1200 = OpFunction %void None %voidfn
    %entry_1200 = OpLabel
    %dummy_1200_1 = OpFunctionCall %void %200
    OpReturn
    OpFunctionEnd

    ; Call %300
    %1300 = OpFunction %void None %voidfn
    %entry_1300 = OpLabel
    %dummy_1300_1 = OpFunctionCall %void %300
    OpReturn
    OpFunctionEnd

 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto& info_1000 = p->GetEntryPointInfo(1000);
  EXPECT_EQ(1u, info_1000.size());
  EXPECT_TRUE(info_1000[0].inputs.empty());
  EXPECT_TRUE(info_1000[0].outputs.empty());

  const auto& info_1100 = p->GetEntryPointInfo(1100);
  EXPECT_EQ(1u, info_1100.size());
  EXPECT_THAT(info_1100[0].inputs, ElementsAre(1));
  EXPECT_TRUE(info_1100[0].outputs.empty());

  const auto& info_1200 = p->GetEntryPointInfo(1200);
  EXPECT_EQ(1u, info_1200.size());
  EXPECT_THAT(info_1200[0].inputs, ElementsAre(2));
  EXPECT_THAT(info_1200[0].outputs, ElementsAre(15));

  const auto& info_1300 = p->GetEntryPointInfo(1300);
  EXPECT_EQ(1u, info_1300.size());
  EXPECT_THAT(info_1300[0].inputs, ElementsAre(1, 2));
  EXPECT_THAT(info_1300[0].outputs, ElementsAre(15));
}

TEST_F(SpvModuleScopeVarParserTest, InputVarsConvertedToPrivate) {
  const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %1 = OpVariable %ptr_in_uint Input
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));

  // TODO(crbug.com/tint/508): Remove this when everything is converted
  // to HLSL style pipeline IO.
  p->SetHLSLStylePipelineIO();

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    __u32
  }
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, OutputVarsConvertedToPrivate) {
  const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %ptr_out_uint = OpTypePointer Output %uint
     %1 = OpVariable %ptr_out_uint Output
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));

  // TODO(crbug.com/tint/508): Remove this when everything is converted
  // to HLSL style pipeline IO.
  p->SetHLSLStylePipelineIO();

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    __u32
  }
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_IOLocations) {
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %1 %2 %3 %4
     OpDecorate %1 Location 0
     OpDecorate %2 Location 0
     OpDecorate %3 Location 30
     OpDecorate %4 Location 40
)" + CommonTypes() +
                        R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %ptr_out_uint = OpTypePointer Output %uint
     %1 = OpVariable %ptr_in_uint Input
     %2 = OpVariable %ptr_out_uint Output
     %3 = OpVariable %ptr_in_uint Input
     %4 = OpVariable %ptr_out_uint Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  // TODO(crbug.com/tint/508): Remove this when everything is converted
  // to HLSL style pipeline IO.
  p->SetHLSLStylePipelineIO();

  ASSERT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(
  Struct main_out {
    StructMember{[[ LocationDecoration{0}
 ]] x_2: __u32}
    StructMember{[[ LocationDecoration{40}
 ]] x_4: __u32}
  }
  Variable{
    x_1
    private
    __u32
  }
  Variable{
    x_2
    private
    __u32
  }
  Variable{
    x_3
    private
    __u32
  }
  Variable{
    x_4
    private
    __u32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __struct_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        LocationDecoration{0}
      }
      x_1_param
      none
      __u32
    }
    VariableConst{
      Decorations{
        LocationDecoration{30}
      }
      x_3_param
      none
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Assignment{
      Identifier[not set]{x_3}
      Identifier[not set]{x_3_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __struct_main_out
          Identifier[not set]{x_2}
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

// TODO(dneto): Test passing pointer to SampleMask as function parameter,
// both input case and output case.

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
