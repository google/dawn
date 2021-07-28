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
    OpCapability SampleRateShading
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
    %int_14 = OpConstant %int 14
    %uint_2 = OpConstant %uint 2

    %v2bool = OpTypeVector %bool 2
    %v2uint = OpTypeVector %uint 2
    %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
    %v4float = OpTypeVector %float 4
    %m3v2float = OpTypeMatrix %v2float 3

    %arr2uint = OpTypeArray %uint %uint_2
  )";
}

std::string StructTypes() {
  return R"(
    %strct = OpTypeStruct %uint %float %arr2uint
)";
}

// Returns layout annotations for types in StructTypes()
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
    undefined
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
    undefined
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
    undefined
    __f32
  })"));
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinVertexIndex) {
  // This is the simple case for the vertex_index builtin,
  // where the SPIR-V uses the same store type as in WGSL.
  // See later for tests where the SPIR-V store type is signed
  // integer, as in GLSL.
  auto p = parser(test::Assemble(Preamble() + R"(
    OpEntryPoint Vertex %main "main" %52 %position
    OpName %position "position"
    OpDecorate %position BuiltIn Position
    OpDecorate %52 BuiltIn VertexIndex
    %uint = OpTypeInt 32 0
    %ptr = OpTypePointer Input %uint
    %52 = OpVariable %ptr Input
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %posty = OpTypePointer Output %v4float
    %position = OpVariable %posty Output
  )" + MainBody()));

  EXPECT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    x_52
    private
    undefined
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
  %1000 = OpCopyObject %11 %1
  OpReturn
  OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("operations producing a pointer to a per-vertex structure are "
                 "not supported: %1000 = OpCopyObject %11 %1"))
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
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] gl_Position: __vec_4__f32}
  }
  Variable{
    gl_Position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{gl_Position}
        }
      }
    }
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
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] gl_Position: __vec_4__f32}
  }
  Variable{
    x_900
    private
    undefined
    __f32
  }
  Variable{
    gl_Position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      Identifier[not set]{x_900}
      ScalarConstructor[not set]{1.000000}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{gl_Position}
        }
      }
    }
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
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] gl_Position: __vec_4__f32}
  }
  Variable{
    gl_Position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{gl_Position}
        }
      }
    }
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
)" + (stage == "Vertex" ? " %2 " : "") +
         +(stage == "Fragment" ? "OpExecutionMode %500 OriginUpperLeft" : "") +
         +(stage == "Vertex" ? " OpDecorate %2 BuiltIn Position " : "") +
         R"(
    OpDecorate %1 BuiltIn PointSize
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %11 = OpTypePointer Output %float
    %1 = OpVariable %11 Output
    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output
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
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
        }
      }
    }
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
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Variable{
    x_900
    private
    undefined
    __f32
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      Identifier[not set]{x_900}
      ScalarConstructor[not set]{1.000000}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
        }
      }
    }
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
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
        }
      }
    }
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
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
        }
      }
    }
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
    undefined
    __bool
    {
      ScalarConstructor[not set]{true}
    }
  }
  Variable{
    x_2
    private
    undefined
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
  Variable{
    x_3
    private
    undefined
    __i32
    {
      ScalarConstructor[not set]{-1}
    }
  }
  Variable{
    x_4
    private
    undefined
    __u32
    {
      ScalarConstructor[not set]{1u}
    }
  }
  Variable{
    x_5
    private
    undefined
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
    undefined
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
  Variable{
    x_2
    private
    undefined
    __i32
    {
      ScalarConstructor[not set]{0}
    }
  }
  Variable{
    x_3
    private
    undefined
    __u32
    {
      ScalarConstructor[not set]{0u}
    }
  }
  Variable{
    x_4
    private
    undefined
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
    undefined
    __bool
    {
      ScalarConstructor[not set]{false}
    }
  }
  Variable{
    x_2
    private
    undefined
    __i32
    {
      ScalarConstructor[not set]{0}
    }
  }
  Variable{
    x_3
    private
    undefined
    __u32
    {
      ScalarConstructor[not set]{0u}
    }
  }
  Variable{
    x_4
    private
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() +
                                 StructTypes() + R"(
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
    undefined
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
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() +
                                 StructTypes() + R"(
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
    undefined
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
  auto p = parser(test::Assemble(Preamble() + FragMain() + CommonTypes() +
                                 StructTypes() + R"(
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
    undefined
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
     OpDecorate %1 Binding 9 ; Required to pass WGSL validation
     OpDecorate %strct Block
)" + CommonTypes() + StructTypes() +
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
      GroupDecoration{3}
      BindingDecoration{9}
    }
    x_1
    storage
    read_write
    __type_name_S
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       DescriptorGroupDecoration_MissingOperandWontAssemble) {
  const auto assembly = Preamble() + FragMain() + CommonLayout() + R"(
     OpDecorate %1 DescriptorSet
     OpDecorate %strct Block
)" + CommonTypes() + StructTypes() +
                        R"(
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
)" + CommonTypes() + StructTypes() +
                        R"(
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
     OpDecorate %1 DescriptorSet 0 ; WGSL validation requires this already
     OpDecorate %1 Binding 3
     OpDecorate %strct Block
)" + CommonLayout() + CommonTypes() +
                                 StructTypes() +
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
      GroupDecoration{0}
      BindingDecoration{3}
    }
    x_1
    storage
    read_write
    __type_name_S
  })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest,
       BindingDecoration_MissingOperandWontAssemble) {
  const auto assembly = Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar Binding
     OpDecorate %strct Block
)" + CommonTypes() + StructTypes() +
                        R"(
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
)" + CommonTypes() + StructTypes() +
                        R"(
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
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
     OpDecorate %strct Block
     OpMemberDecorate %strct 0 NonReadable
)" + CommonLayout() + CommonTypes() +
                                 StructTypes() + R"(
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
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_1
    storage
    read_write
    __type_name_S
  }
)")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ColMajorDecoration_Dropped) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0
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
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    myvar
    storage
    read_write
    __type_name_S
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, MatrixStrideDecoration_Natural_Dropped) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0
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
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    myvar
    storage
    read_write
    __type_name_S
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, MatrixStrideDecoration) {
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpName %myvar "myvar"
     OpDecorate %myvar DescriptorSet 0
     OpDecorate %myvar Binding 0
     OpDecorate %s Block
     OpMemberDecorate %s 0 MatrixStride 64
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
    StructMember{[[ stride 64 tint_internal(disable_validation__ignore_stride) offset 0 ]] field0: __mat_2_3__f32}
  }
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    myvar
    storage
    read_write
    __type_name_S
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
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
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
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_1
    storage
    read
    __type_name_S
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, StorageBuffer_NonWritable_NotAllMembers) {
  // Variable should have access(read_write)
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
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
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_1
    storage
    read_write
    __type_name_S
  }
})")) << module_str;
}

TEST_F(
    SpvModuleScopeVarParserTest,
    StorageBuffer_NonWritable_NotAllMembers_DuplicatedOnSameMember) {  // NOLINT
  // Variable should have access(read_write)
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %s Block
     OpDecorate %1 DescriptorSet 0
     OpDecorate %1 Binding 0
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
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_1
    storage
    read_write
    __type_name_S
  }
})")) << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, ScalarSpecConstant_DeclareConst_Id_TooBig) {
  // Override IDs must be between 0 and 65535
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 SpecId 65536
     %bool = OpTypeBool
     %1 = OpSpecConstantTrue %bool
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  EXPECT_FALSE(p->Parse());
  EXPECT_EQ(p->error(),
            "SpecId too large. WGSL override IDs must be between 0 and 65535: "
            "ID %1 has SpecId 65536");
}

TEST_F(SpvModuleScopeVarParserTest,
       ScalarSpecConstant_DeclareConst_Id_MaxValid) {
  // Override IDs must be between 0 and 65535
  auto p = parser(test::Assemble(Preamble() + FragMain() + R"(
     OpDecorate %1 SpecId 65535
     %bool = OpTypeBool
     %1 = OpSpecConstantTrue %bool
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
  )" + MainBody()));
  EXPECT_TRUE(p->Parse());
  EXPECT_EQ(p->error(), "");
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
    undefined
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
  const std::string expected =
      R"(Module{
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __i32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected =
      R"(Module{
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_11
        none
        undefined
        __ptr_private__i32
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
        undefined
        __i32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_14}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
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
    x_1
    private
    undefined
    __i32
  })"))
      << module_str;

  // Correct creation of value
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __i32
        {
          Identifier[not set]{x_1}
        }
      }
    })"));

  // Correct parameter on entry point
  EXPECT_THAT(module_str, HasSubstr(R"(
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
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
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_11
        none
        undefined
        __ptr_private__u32
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
        undefined
        __u32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_11}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
std::string SampleMaskPreamble(std::string store_type, uint32_t stride = 0u) {
  return std::string(R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "main" %1
    OpExecutionMode %main OriginUpperLeft
    OpDecorate %1 BuiltIn SampleMask
)") +
         (stride > 0u ? R"(
    OpDecorate %uarr1 ArrayStride 4
    OpDecorate %uarr2 ArrayStride 4
    OpDecorate %iarr1 ArrayStride 4
    OpDecorate %iarr2 ArrayStride 4
)"
                      : "") +
         R"(
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
    x_1
    private
    undefined
    __array__u32_1
  })"))
      << module_str;

  // Correct creation of value
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        undefined
        __u32
        {
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    })"));

  // Correct parameter on entry point
  EXPECT_THAT(module_str, HasSubstr(R"(
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
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
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __array__u32_1
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        undefined
        __u32
        {
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
    x_1
    private
    undefined
    __array__u32_1
  })"))
      << module_str;

  // Correct creation of value
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        undefined
        __u32
        {
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    })"));

  // Correct parameter on entry point
  EXPECT_THAT(module_str, HasSubstr(R"(
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
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
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __array__i32_1
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        undefined
        __i32
        {
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __array__i32_1
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        undefined
        __i32
        {
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __array__i32_1
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_4
        none
        undefined
        __i32
        {
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __array__u32_1
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      ScalarConstructor[not set]{0u}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __array__u32_1
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      ScalarConstructor[not set]{0u}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __array__u32_1
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      ScalarConstructor[not set]{0u}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __array__i32_1
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      ScalarConstructor[not set]{12}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Bitcast[not set]<__u32>{
            ArrayAccessor[not set]{
              Identifier[not set]{x_1}
              ScalarConstructor[not set]{0}
            }
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __array__i32_1
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      ScalarConstructor[not set]{12}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Bitcast[not set]<__u32>{
            ArrayAccessor[not set]{
              Identifier[not set]{x_1}
              ScalarConstructor[not set]{0}
            }
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __array__i32_1
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      ScalarConstructor[not set]{12}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Bitcast[not set]<__u32>{
            ArrayAccessor[not set]{
              Identifier[not set]{x_1}
              ScalarConstructor[not set]{0}
            }
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_In_WithStride) {
  const std::string assembly = SampleMaskPreamble("%uarr1", 4u) + R"(
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

  EXPECT_THAT(module_str, HasSubstr(R"(
  Arr -> __array__u32_1_stride_4
)")) << module_str;

  // Correct declaration
  EXPECT_THAT(module_str, HasSubstr(R"(
  Variable{
    x_1
    private
    undefined
    __type_name_Arr
  })"))
      << module_str;

  // Correct creation of value
  EXPECT_THAT(module_str, HasSubstr(R"(
    VariableDeclStatement{
      VariableConst{
        x_3
        none
        undefined
        __u32
        {
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    })"));

  // Correct parameter on entry point
  EXPECT_THAT(module_str, HasSubstr(R"(
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
    })"))
      << module_str;
}

TEST_F(SpvModuleScopeVarParserTest, SampleMask_Out_WithStride) {
  const std::string assembly = SampleMaskPreamble("%uarr1", 4u) + R"(
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
  const std::string expected = R"(Module{
  Arr -> __array__u32_1_stride_4
  Arr_1 -> __array__u32_2_stride_4
  Arr_2 -> __array__i32_1_stride_4
  Arr_3 -> __array__i32_2_stride_4
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __type_name_Arr
  }
  Function main_1 -> __void
  ()
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      ScalarConstructor[not set]{0u}
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
}

// Returns the start of a shader for testing VertexIndex,
// parameterized by store type of %int or %uint
std::string VertexIndexPreamble(std::string store_type) {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %position %1
    OpDecorate %position BuiltIn Position
    OpDecorate %1 BuiltIn VertexIndex
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %ptr_ty = OpTypePointer Input )" +
         store_type + R"(
    %1 = OpVariable %ptr_ty Input
    %v4float = OpTypeVector %float 4
    %posty = OpTypePointer Output %v4float
    %position = OpVariable %posty Output
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_4_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Variable{
    x_4
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __i32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{vertex_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_4_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Variable{
    x_4
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_14
        none
        undefined
        __ptr_private__i32
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
        undefined
        __i32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_14}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{vertex_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_4_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Variable{
    x_4
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __i32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{vertex_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_4_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    x_4
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{vertex_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_4_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    x_4
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_14
        none
        undefined
        __ptr_private__u32
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
        undefined
        __u32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_14}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{vertex_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_4_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    x_4
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{vertex_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
    OpEntryPoint Vertex %main "main" %position %1
    OpName %position "position"
    OpDecorate %position BuiltIn Position
    OpDecorate %1 BuiltIn InstanceIndex
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %ptr_ty = OpTypePointer Input )" +
         store_type + R"(
    %1 = OpVariable %ptr_ty Input
    %v4float = OpTypeVector %float 4
    %posty = OpTypePointer Output %v4float
    %position = OpVariable %posty Output
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] position_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Variable{
    position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __i32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{instance_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{position}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] position_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Variable{
    position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_14
        none
        undefined
        __ptr_private__i32
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
        undefined
        __i32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_14}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{instance_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{position}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] position_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Variable{
    position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __i32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{instance_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{position}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] position_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{instance_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{position}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] position_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_14
        none
        undefined
        __ptr_private__u32
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
        undefined
        __u32
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_14}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{instance_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{position}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] position_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{instance_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{position}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(module_str, expected);
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

// Returns the start of a shader for testing LocalInvocationIndex,
// parameterized by store type of %int or %uint
std::string ComputeBuiltinInputPreamble(std::string builtin,
                                        std::string store_type) {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint GLCompute %main "main" %1
    OpExecutionMode %main LocalSize 1 1 1
    OpDecorate %1 BuiltIn )" +
         builtin + R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %v3uint = OpTypeVector %uint 3
    %v3int = OpTypeVector %int 3
    %ptr_ty = OpTypePointer Input )" +
         store_type + R"(
    %1 = OpVariable %ptr_ty Input
)";
}

struct ComputeBuiltinInputCase {
  std::string spirv_builtin;
  std::string spirv_store_type;
  std::string wgsl_builtin;
};
inline std::ostream& operator<<(std::ostream& o, ComputeBuiltinInputCase c) {
  return o << "ComputeBuiltinInputCase(" << c.spirv_builtin << " "
           << c.spirv_store_type << " " << c.wgsl_builtin << ")";
}

std::string WgslType(std::string spirv_type) {
  if (spirv_type == "%uint") {
    return "__u32";
  }
  if (spirv_type == "%int") {
    return "__i32";
  }
  if (spirv_type == "%v3uint") {
    return "__vec_3__u32";
  }
  if (spirv_type == "%v3int") {
    return "__vec_3__i32";
  }
  return "error";
}

std::string UnsignedWgslType(std::string wgsl_type) {
  if (wgsl_type == "__u32") {
    return "__u32";
  }
  if (wgsl_type == "__i32") {
    return "__u32";
  }
  if (wgsl_type == "__vec_3__u32") {
    return "__vec_3__u32";
  }
  if (wgsl_type == "__vec_3__i32") {
    return "__vec_3__u32";
  }
  return "error";
}

std::string SignedWgslType(std::string wgsl_type) {
  if (wgsl_type == "__u32") {
    return "__i32";
  }
  if (wgsl_type == "__i32") {
    return "__i32";
  }
  if (wgsl_type == "__vec_3__u32") {
    return "__vec_3__i32";
  }
  if (wgsl_type == "__vec_3__i32") {
    return "__vec_3__i32";
  }
  return "error";
}

using SpvModuleScopeVarParserTest_ComputeBuiltin =
    SpvParserTestBase<::testing::TestWithParam<ComputeBuiltinInputCase>>;

TEST_P(SpvModuleScopeVarParserTest_ComputeBuiltin, Load_Direct) {
  const auto wgsl_type = WgslType(GetParam().spirv_store_type);
  const auto unsigned_wgsl_type = UnsignedWgslType(wgsl_type);
  const auto signed_wgsl_type = SignedWgslType(wgsl_type);
  const std::string assembly =
      ComputeBuiltinInputPreamble(GetParam().spirv_builtin,
                                  GetParam().spirv_store_type) +
      R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %2 = OpLoad )" +
      GetParam().spirv_store_type + R"( %1
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    )" + wgsl_type + R"(
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        )" + wgsl_type + R"(
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{1}
    ScalarConstructor[not set]{1}
    ScalarConstructor[not set]{1}
  }
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{)" + GetParam().wgsl_builtin +
                               R"(}
      }
      x_1_param
      none
      undefined
      )" + unsigned_wgsl_type + R"(
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1})" +
                               (wgsl_type == unsigned_wgsl_type ?
                                                                R"(
      Identifier[not set]{x_1_param})"
                                                                :
                                                                R"(
      Bitcast[not set]<)" + signed_wgsl_type + R"(>{
        Identifier[not set]{x_1_param}
      })") + R"(
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
}

TEST_P(SpvModuleScopeVarParserTest_ComputeBuiltin, Load_CopyObject) {
  const auto wgsl_type = WgslType(GetParam().spirv_store_type);
  const auto unsigned_wgsl_type = UnsignedWgslType(wgsl_type);
  const auto signed_wgsl_type = SignedWgslType(wgsl_type);
  const std::string assembly =
      ComputeBuiltinInputPreamble(GetParam().spirv_builtin,
                                  GetParam().spirv_store_type) +
      R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %13 = OpCopyObject %ptr_ty %1
    %2 = OpLoad )" +
      GetParam().spirv_store_type + R"( %13
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    )" + wgsl_type + R"(
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_13
        none
        undefined
        __ptr_private)" + wgsl_type +
                               R"(
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
        undefined
        )" + wgsl_type + R"(
        {
          UnaryOp[not set]{
            indirection
            Identifier[not set]{x_13}
          }
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{1}
    ScalarConstructor[not set]{1}
    ScalarConstructor[not set]{1}
  }
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{)" + GetParam().wgsl_builtin +
                               R"(}
      }
      x_1_param
      none
      undefined
      )" + unsigned_wgsl_type + R"(
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1})" +
                               (wgsl_type == unsigned_wgsl_type ?
                                                                R"(
      Identifier[not set]{x_1_param})"
                                                                :
                                                                R"(
      Bitcast[not set]<)" + signed_wgsl_type + R"(>{
        Identifier[not set]{x_1_param}
      })") + R"(
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
}

TEST_P(SpvModuleScopeVarParserTest_ComputeBuiltin, Load_AccessChain) {
  const auto wgsl_type = WgslType(GetParam().spirv_store_type);
  const auto unsigned_wgsl_type = UnsignedWgslType(wgsl_type);
  const auto signed_wgsl_type = SignedWgslType(wgsl_type);
  const std::string assembly =
      ComputeBuiltinInputPreamble(GetParam().spirv_builtin,
                                  GetParam().spirv_store_type) +
      R"(
    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    %13 = OpAccessChain %ptr_ty %1
    %2 = OpLoad )" +
      GetParam().spirv_store_type + R"( %13
    OpReturn
    OpFunctionEnd
 )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto module_str = p->program().to_str();
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    )" + wgsl_type + R"(
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        )" + wgsl_type + R"(
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __void
  StageDecoration{compute}
  WorkgroupDecoration{
    ScalarConstructor[not set]{1}
    ScalarConstructor[not set]{1}
    ScalarConstructor[not set]{1}
  }
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{)" + GetParam().wgsl_builtin +
                               R"(}
      }
      x_1_param
      none
      undefined
      )" + unsigned_wgsl_type + R"(
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1})" +
                               (wgsl_type == unsigned_wgsl_type ?
                                                                R"(
      Identifier[not set]{x_1_param})"
                                                                :
                                                                R"(
      Bitcast[not set]<)" + signed_wgsl_type + R"(>{
        Identifier[not set]{x_1_param}
      })") + R"(
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(module_str, expected) << module_str;
}

INSTANTIATE_TEST_SUITE_P(
    Samples,
    SpvModuleScopeVarParserTest_ComputeBuiltin,
    ::testing::ValuesIn(std::vector<ComputeBuiltinInputCase>{
        {"LocalInvocationIndex", "%uint", "local_invocation_index"},
        {"LocalInvocationIndex", "%int", "local_invocation_index"},
        {"LocalInvocationId", "%v3uint", "local_invocation_id"},
        {"LocalInvocationId", "%v3int", "local_invocation_id"},
        {"GlobalInvocationId", "%v3uint", "global_invocation_id"},
        {"GlobalInvocationId", "%v3int", "global_invocation_id"},
        {"WorkgroupId", "%v3uint", "workgroup_id"},
        {"WorkgroupId", "%v3int", "workgroup_id"}}));

// TODO(dneto): crbug.com/tint/752
// NumWorkgroups support is blocked by crbug.com/tint/752
// When the AST supports NumWorkgroups, add these cases:
//        {"NumWorkgroups", "%uint", "num_workgroups"}
//        {"NumWorkgroups", "%int", "num_workgroups"}

TEST_F(SpvModuleScopeVarParserTest, RegisterInputOutputVars) {
  const std::string assembly =
      R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %1000 "w1000"
    OpEntryPoint Fragment %1100 "w1100" %1
    OpEntryPoint Fragment %1200 "w1200" %2 %15
    ; duplication is tolerated prior to SPIR-V 1.4
    OpEntryPoint Fragment %1300 "w1300" %1 %15 %2 %1
    OpExecutionMode %1000 OriginUpperLeft
    OpExecutionMode %1100 OriginUpperLeft
    OpExecutionMode %1200 OriginUpperLeft
    OpExecutionMode %1300 OriginUpperLeft

    OpDecorate %1 Location 1
    OpDecorate %2 Location 2
    OpDecorate %5 Location 5
    OpDecorate %11 Location 1
    OpDecorate %12 Location 2
    OpDecorate %15 Location 5

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

  // Validation incorrectly reports an overlap for the duplicated variable %1 on
  // shader %1300
  p->SkipDumpingPending(
      "https://github.com/KhronosGroup/SPIRV-Tools/issues/4403");
}

TEST_F(SpvModuleScopeVarParserTest, InputVarsConvertedToPrivate) {
  const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %1 = OpVariable %ptr_in_uint Input
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    undefined
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

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    undefined
    __u32
  }
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       OutputVarsConvertedToPrivate_WithInitializer) {
  const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %ptr_out_uint = OpTypePointer Output %uint
     %1 = OpVariable %ptr_out_uint Output %uint_1
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    undefined
    __u32
    {
      ScalarConstructor[not set]{1u}
    }
  }
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       Builtin_Output_Initializer_SameSignednessAsWGSL) {
  // Only outputs can have initializers.
  // WGSL sample_mask store type is u32.
  const auto assembly = Preamble() + FragMain() + R"(
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() + R"(
     %arr_ty = OpTypeArray %uint %uint_1
     %ptr_ty = OpTypePointer Output %arr_ty
     %arr_init = OpConstantComposite %arr_ty %uint_2
     %1 = OpVariable %ptr_ty Output %arr_init
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    undefined
    __array__u32_1
    {
      TypeConstructor[not set]{
        __array__u32_1
        ScalarConstructor[not set]{2u}
      }
    }
  }
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       Builtin_Output_Initializer_OppositeSignednessAsWGSL) {
  // Only outputs can have initializers.
  // WGSL sample_mask store type is u32.  Use i32 in SPIR-V
  const auto assembly = Preamble() + FragMain() + R"(
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() + R"(
     %arr_ty = OpTypeArray %int %uint_1
     %ptr_ty = OpTypePointer Output %arr_ty
     %arr_init = OpConstantComposite %arr_ty %int_14
     %1 = OpVariable %ptr_ty Output %arr_init
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    undefined
    __array__i32_1
    {
      TypeConstructor[not set]{
        __array__i32_1
        ScalarConstructor[not set]{14}
      }
    }
  }
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Builtin_Input_SameSignednessAsWGSL) {
  // WGSL vertex_index store type is u32.
  const auto assembly = Preamble() + FragMain() + R"(
     OpDecorate %1 BuiltIn VertexIndex
)" + CommonTypes() + R"(
     %ptr_ty = OpTypePointer Input %uint
     %1 = OpVariable %ptr_ty Input
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    undefined
    __u32
  }
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Builtin_Input_OppositeSignednessAsWGSL) {
  // WGSL vertex_index store type is u32.  Use i32 in SPIR-V.
  const auto assembly = Preamble() + FragMain() + R"(
     OpDecorate %1 BuiltIn VertexIndex
)" + CommonTypes() + R"(
     %ptr_ty = OpTypePointer Input %int
     %1 = OpVariable %ptr_ty Input
  )" + MainBody();
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Variable{
    x_1
    private
    undefined
    __i32
  }
)";
  EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvModuleScopeVarParserTest, EntryPointWrapping_IOLocations) {
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2 %3 %4
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 0
     OpDecorate %2 Location 0
     OpDecorate %3 Location 30
     OpDecorate %4 Location 6
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

  ASSERT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(
  Struct main_out {
    StructMember{[[ LocationDecoration{0}
 ]] x_2_1: __u32}
    StructMember{[[ LocationDecoration{6}
 ]] x_4_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    x_2
    private
    undefined
    __u32
  }
  Variable{
    x_3
    private
    undefined
    __u32
  }
  Variable{
    x_4
    private
    undefined
    __u32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        LocationDecoration{0}
      }
      x_1_param
      none
      undefined
      __u32
    }
    VariableConst{
      Decorations{
        LocationDecoration{30}
      }
      x_3_param
      none
      undefined
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
          __type_name_main_out
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

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_Input_SameSignedness) {
  // instance_index is u32 in WGSL. Use uint in SPIR-V.
  // No bitcasts are used for parameter formation or return value.
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %1 %position
     OpDecorate %position BuiltIn Position
     OpDecorate %1 BuiltIn InstanceIndex
)" + CommonTypes() +
                        R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %1 = OpVariable %ptr_in_uint Input
     %posty = OpTypePointer Output %v4float
     %position = OpVariable %posty Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpLoad %uint %1 ; load same signedness
     ;;;; %3 = OpLoad %int %1 ; loading different signedness is invalid.
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_4_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    x_4
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __u32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{instance_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_Input_OppositeSignedness) {
  // instance_index is u32 in WGSL. Use int in SPIR-V.
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %position %1
     OpDecorate %position BuiltIn Position
     OpDecorate %1 BuiltIn InstanceIndex
)" + CommonTypes() +
                        R"(
     %ptr_in_int = OpTypePointer Input %int
     %1 = OpVariable %ptr_in_int Input
     %posty = OpTypePointer Output %v4float
     %position = OpVariable %posty Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %2 = OpLoad %int %1 ; load same signedness
     ;;; %3 = OpLoad %uint %1 ; loading different signedness is invalid
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_4_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __i32
  }
  Variable{
    x_4
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    VariableDeclStatement{
      VariableConst{
        x_2
        none
        undefined
        __i32
        {
          Identifier[not set]{x_1}
        }
      }
    }
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{instance_index}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_4}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

// SampleMask is an array in Vulkan SPIR-V, but a scalar in WGSL.
TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_SampleMask_In_Unsigned) {
  // SampleMask is u32 in WGSL.
  // Use unsigned array element in Vulkan.
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() +
                        R"(
     %arr = OpTypeArray %uint %uint_1
     %ptr_ty = OpTypePointer Input %arr
     %1 = OpVariable %ptr_ty Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __array__u32_1
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_SampleMask_In_Signed) {
  // SampleMask is u32 in WGSL.
  // Use signed array element in Vulkan.
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() +
                        R"(
     %arr = OpTypeArray %int %uint_1
     %ptr_ty = OpTypePointer Input %arr
     %1 = OpVariable %ptr_ty Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Variable{
    x_1
    private
    undefined
    __array__i32_1
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        BuiltinDecoration{sample_mask}
      }
      x_1_param
      none
      undefined
      __u32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Bitcast[not set]<__i32>{
        Identifier[not set]{x_1_param}
      }
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_SampleMask_Out_Unsigned_Initializer) {
  // SampleMask is u32 in WGSL.
  // Use unsigned array element in Vulkan.
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() +
                        R"(
     %arr = OpTypeArray %uint %uint_1
     %ptr_ty = OpTypePointer Output %arr
     %zero = OpConstantNull %arr
     %1 = OpVariable %ptr_ty Output %zero

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __array__u32_1
    {
      TypeConstructor[not set]{
        __array__u32_1
        ScalarConstructor[not set]{0u}
      }
    }
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_SampleMask_Out_Signed_Initializer) {
  // SampleMask is u32 in WGSL.
  // Use signed array element in Vulkan.
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn SampleMask
)" + CommonTypes() +
                        R"(
     %arr = OpTypeArray %int %uint_1
     %ptr_ty = OpTypePointer Output %arr
     %zero = OpConstantNull %arr
     %1 = OpVariable %ptr_ty Output %zero

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{sample_mask}
 ]] x_1_1: __u32}
  }
  Variable{
    x_1
    private
    undefined
    __array__i32_1
    {
      TypeConstructor[not set]{
        __array__i32_1
        ScalarConstructor[not set]{0}
      }
    }
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Bitcast[not set]<__u32>{
            ArrayAccessor[not set]{
              Identifier[not set]{x_1}
              ScalarConstructor[not set]{0}
            }
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_BuiltinVar_FragDepth_Out_Initializer) {
  // FragDepth does not require conversion, because it's f32.
  // The member of the return type is just the identifier corresponding
  // to the module-scope private variable.
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 BuiltIn FragDepth
)" + CommonTypes() +
                        R"(
     %ptr_ty = OpTypePointer Output %float
     %1 = OpVariable %ptr_ty Output %float_0

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{frag_depth}
 ]] x_1_1: __f32}
  }
  Variable{
    x_1
    private
    undefined
    __f32
    {
      ScalarConstructor[not set]{0.000000}
    }
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_1}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, BuiltinPosition_BuiltIn_Position) {
  // In Vulkan SPIR-V, Position is the first member of gl_PerVertex
  const std::string assembly = PerVertexPreamble() + R"(
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] gl_Position: __vec_4__f32}
  }
  Variable{
    gl_Position
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{gl_Position}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       BuiltinPosition_BuiltIn_Position_Initializer) {
  const std::string assembly = R"(
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
    %v4float = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %arr = OpTypeArray %float %uint_1
    %10 = OpTypeStruct %v4float %float %arr %arr
    %11 = OpTypePointer Output %10

    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7

    %init_pos = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
    %init_clip = OpConstantComposite %arr %float_6
    %init_cull = OpConstantComposite %arr %float_7
    %init_per_vertex = OpConstantComposite %10 %init_pos %float_5 %init_clip %init_cull

    %1 = OpVariable %11 Output %init_per_vertex

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] gl_Position: __vec_4__f32}
  }
  Variable{
    gl_Position
    private
    undefined
    __vec_4__f32
    {
      TypeConstructor[not set]{
        __vec_4__f32
        ScalarConstructor[not set]{1.000000}
        ScalarConstructor[not set]{2.000000}
        ScalarConstructor[not set]{3.000000}
        ScalarConstructor[not set]{4.000000}
      }
    }
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{gl_Position}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Input_FlattenArray_OneLevel) {
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 4
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_3 = OpConstant %uint 3
    %arr = OpTypeArray %float %uint_3
    %11 = OpTypePointer Input %arr

    %1 = OpVariable %11 Input

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __array__f32_3
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        LocationDecoration{4}
      }
      x_1_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{5}
      }
      x_1_param_1
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{6}
      }
      x_1_param_2
      none
      undefined
      __f32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
    }
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{1}
      }
      Identifier[not set]{x_1_param_1}
    }
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{2}
      }
      Identifier[not set]{x_1_param_2}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Input_FlattenMatrix) {
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 9
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %m2v4float = OpTypeMatrix %v4float 2
    %uint = OpTypeInt 32 0

    %11 = OpTypePointer Input %m2v4float

    %1 = OpVariable %11 Input

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __mat_4_2__f32
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        LocationDecoration{9}
      }
      x_1_param
      none
      undefined
      __vec_4__f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{10}
      }
      x_1_param_1
      none
      undefined
      __vec_4__f32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
    }
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{1}
      }
      Identifier[not set]{x_1_param_1}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Input_FlattenStruct_LocOnVariable) {
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2

    OpName %strct "Communicators"
    OpMemberName %strct 0 "alice"
    OpMemberName %strct 1 "bob"

    OpDecorate %1 Location 9
    OpDecorate %2 BuiltIn Position


    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %strct = OpTypeStruct %float %v4float

    %11 = OpTypePointer Input %strct

    %1 = OpVariable %11 Input

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct Communicators {
    StructMember{alice: __f32}
    StructMember{bob: __vec_4__f32}
  }
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __type_name_Communicators
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        LocationDecoration{9}
      }
      x_1_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{10}
      }
      x_1_param_1
      none
      undefined
      __vec_4__f32
    }
  )
  {
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{alice}
      }
      Identifier[not set]{x_1_param}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{bob}
      }
      Identifier[not set]{x_1_param_1}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Input_FlattenNested) {
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 7
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %m2v4float = OpTypeMatrix %v4float 2
    %uint = OpTypeInt 32 0
    %uint_2 = OpConstant %uint 2

    %arr = OpTypeArray %m2v4float %uint_2

    %11 = OpTypePointer Input %arr
    %1 = OpVariable %11 Input

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __array__mat_4_2__f32_2
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        LocationDecoration{7}
      }
      x_1_param
      none
      undefined
      __vec_4__f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{8}
      }
      x_1_param_1
      none
      undefined
      __vec_4__f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{9}
      }
      x_1_param_2
      none
      undefined
      __vec_4__f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{10}
      }
      x_1_param_3
      none
      undefined
      __vec_4__f32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        ArrayAccessor[not set]{
          Identifier[not set]{x_1}
          ScalarConstructor[not set]{0}
        }
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
    }
    Assignment{
      ArrayAccessor[not set]{
        ArrayAccessor[not set]{
          Identifier[not set]{x_1}
          ScalarConstructor[not set]{0}
        }
        ScalarConstructor[not set]{1}
      }
      Identifier[not set]{x_1_param_1}
    }
    Assignment{
      ArrayAccessor[not set]{
        ArrayAccessor[not set]{
          Identifier[not set]{x_1}
          ScalarConstructor[not set]{1}
        }
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param_2}
    }
    Assignment{
      ArrayAccessor[not set]{
        ArrayAccessor[not set]{
          Identifier[not set]{x_1}
          ScalarConstructor[not set]{1}
        }
        ScalarConstructor[not set]{1}
      }
      Identifier[not set]{x_1_param_3}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Output_FlattenArray_OneLevel) {
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 4
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %uint = OpTypeInt 32 0
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_3 = OpConstant %uint 3
    %arr = OpTypeArray %float %uint_3
    %11 = OpTypePointer Output %arr

    %1 = OpVariable %11 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ LocationDecoration{4}
 ]] x_1_1: __f32}
    StructMember{[[ LocationDecoration{5}
 ]] x_1_2: __f32}
    StructMember{[[ LocationDecoration{6}
 ]] x_1_3: __f32}
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __array__f32_3
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{1}
          }
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{2}
          }
          Identifier[not set]{x_2}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Output_FlattenMatrix) {
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2
    OpDecorate %1 Location 9
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %m2v4float = OpTypeMatrix %v4float 2
    %uint = OpTypeInt 32 0

    %11 = OpTypePointer Output %m2v4float

    %1 = OpVariable %11 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct main_out {
    StructMember{[[ LocationDecoration{9}
 ]] x_1_1: __vec_4__f32}
    StructMember{[[ LocationDecoration{10}
 ]] x_1_2: __vec_4__f32}
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __mat_4_2__f32
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{0}
          }
          ArrayAccessor[not set]{
            Identifier[not set]{x_1}
            ScalarConstructor[not set]{1}
          }
          Identifier[not set]{x_2}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, Output_FlattenStruct_LocOnVariable) {
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2

    OpName %strct "Communicators"
    OpMemberName %strct 0 "alice"
    OpMemberName %strct 1 "bob"

    OpDecorate %1 Location 9
    OpDecorate %2 BuiltIn Position


    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %strct = OpTypeStruct %float %v4float

    %11 = OpTypePointer Output %strct

    %1 = OpVariable %11 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct Communicators {
    StructMember{alice: __f32}
    StructMember{bob: __vec_4__f32}
  }
  Struct main_out {
    StructMember{[[ LocationDecoration{9}
 ]] x_1_1: __f32}
    StructMember{[[ LocationDecoration{10}
 ]] x_1_2: __vec_4__f32}
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __type_name_Communicators
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          MemberAccessor[not set]{
            Identifier[not set]{x_1}
            Identifier[not set]{alice}
          }
          MemberAccessor[not set]{
            Identifier[not set]{x_1}
            Identifier[not set]{bob}
          }
          Identifier[not set]{x_2}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, FlattenStruct_LocOnMembers) {
  // Block-decorated struct may have its members decorated with Location.
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2 %3

    OpName %strct "Communicators"
    OpMemberName %strct 0 "alice"
    OpMemberName %strct 1 "bob"

    OpMemberDecorate %strct 0 Location 9
    OpMemberDecorate %strct 1 Location 11
    OpDecorate %strct Block
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %strct = OpTypeStruct %float %v4float

    %11 = OpTypePointer Input %strct
    %13 = OpTypePointer Output %strct

    %1 = OpVariable %11 Input
    %3 = OpVariable %13 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->Parse()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty());

  const auto got = p->program().to_str();
  const std::string expected = R"(Module{
  Struct Communicators {
    StructMember{alice: __f32}
    StructMember{bob: __vec_4__f32}
  }
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_2_1: __vec_4__f32}
    StructMember{[[ LocationDecoration{9}
 ]] x_3_1: __f32}
    StructMember{[[ LocationDecoration{11}
 ]] x_3_2: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __type_name_Communicators
  }
  Variable{
    x_3
    private
    undefined
    __type_name_Communicators
  }
  Variable{
    x_2
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        LocationDecoration{9}
      }
      x_1_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{11}
      }
      x_1_param_1
      none
      undefined
      __vec_4__f32
    }
  )
  {
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{alice}
      }
      Identifier[not set]{x_1_param}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{bob}
      }
      Identifier[not set]{x_1_param_1}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_2}
          MemberAccessor[not set]{
            Identifier[not set]{x_3}
            Identifier[not set]{alice}
          }
          MemberAccessor[not set]{
            Identifier[not set]{x_3}
            Identifier[not set]{bob}
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest, FlattenStruct_LocOnStruct) {
  const std::string assembly = R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Vertex %main "main" %1 %2 %3

    OpName %strct "Communicators"
    OpMemberName %strct 0 "alice"
    OpMemberName %strct 1 "bob"

    OpDecorate %strct Location 9
    OpDecorate %strct Block
    OpDecorate %2 BuiltIn Position

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %strct = OpTypeStruct %float %v4float

    %11 = OpTypePointer Input %strct
    %13 = OpTypePointer Output %strct

    %1 = OpVariable %11 Input
    %3 = OpVariable %13 Output

    %12 = OpTypePointer Output %v4float
    %2 = OpVariable %12 Output

    %main = OpFunction %void None %voidfn
    %entry = OpLabel
    OpReturn
    OpFunctionEnd
)";
  auto p = parser(test::Assemble(assembly));

  // The validator rejects this because Location decorations
  // can only go on OpVariable or members of a structure type.
  ASSERT_FALSE(p->Parse()) << p->error() << assembly;
  EXPECT_THAT(p->error(),
              HasSubstr("Location decoration can only be applied to a variable "
                        "or member of a structure type"));
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_Interpolation_Flat_Vertex_In) {
  // Flat decorations are dropped for integral
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %1 %2 %3 %4 %5 %6 %10
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6
     OpDecorate %1 Flat
     OpDecorate %2 Flat
     OpDecorate %3 Flat
     OpDecorate %4 Flat
     OpDecorate %5 Flat
     OpDecorate %6 Flat
     OpDecorate %10 BuiltIn Position
)" + CommonTypes() +
                        R"(
     %ptr_in_uint = OpTypePointer Input %uint
     %ptr_in_v2uint = OpTypePointer Input %v2uint
     %ptr_in_int = OpTypePointer Input %int
     %ptr_in_v2int = OpTypePointer Input %v2int
     %ptr_in_float = OpTypePointer Input %float
     %ptr_in_v2float = OpTypePointer Input %v2float
     %1 = OpVariable %ptr_in_uint Input
     %2 = OpVariable %ptr_in_v2uint Input
     %3 = OpVariable %ptr_in_int Input
     %4 = OpVariable %ptr_in_v2int Input
     %5 = OpVariable %ptr_in_float Input
     %6 = OpVariable %ptr_in_v2float Input

     %ptr_out_v4float = OpTypePointer Output %v4float
     %10 = OpVariable %ptr_out_v4float Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Module{
  Struct main_out {
    StructMember{[[ BuiltinDecoration{position}
 ]] x_10_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    x_2
    private
    undefined
    __vec_2__u32
  }
  Variable{
    x_3
    private
    undefined
    __i32
  }
  Variable{
    x_4
    private
    undefined
    __vec_2__i32
  }
  Variable{
    x_5
    private
    undefined
    __f32
  }
  Variable{
    x_6
    private
    undefined
    __vec_2__f32
  }
  Variable{
    x_10
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  (
    VariableConst{
      Decorations{
        LocationDecoration{1}
      }
      x_1_param
      none
      undefined
      __u32
    }
    VariableConst{
      Decorations{
        LocationDecoration{2}
      }
      x_2_param
      none
      undefined
      __vec_2__u32
    }
    VariableConst{
      Decorations{
        LocationDecoration{3}
      }
      x_3_param
      none
      undefined
      __i32
    }
    VariableConst{
      Decorations{
        LocationDecoration{4}
      }
      x_4_param
      none
      undefined
      __vec_2__i32
    }
    VariableConst{
      Decorations{
        LocationDecoration{5}
        InterpolateDecoration{flat none}
      }
      x_5_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{6}
        InterpolateDecoration{flat none}
      }
      x_6_param
      none
      undefined
      __vec_2__f32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Assignment{
      Identifier[not set]{x_2}
      Identifier[not set]{x_2_param}
    }
    Assignment{
      Identifier[not set]{x_3}
      Identifier[not set]{x_3_param}
    }
    Assignment{
      Identifier[not set]{x_4}
      Identifier[not set]{x_4_param}
    }
    Assignment{
      Identifier[not set]{x_5}
      Identifier[not set]{x_5_param}
    }
    Assignment{
      Identifier[not set]{x_6}
      Identifier[not set]{x_6_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_10}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_Interpolation_Flat_Vertex_Output) {
  // Flat decorations are dropped for integral
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Vertex %main "main" %1 %2 %3 %4 %5 %6 %10
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6
     OpDecorate %1 Flat
     OpDecorate %2 Flat
     OpDecorate %3 Flat
     OpDecorate %4 Flat
     OpDecorate %5 Flat
     OpDecorate %6 Flat
     OpDecorate %10 BuiltIn Position
)" + CommonTypes() +
                        R"(
     %ptr_out_uint = OpTypePointer Output %uint
     %ptr_out_v2uint = OpTypePointer Output %v2uint
     %ptr_out_int = OpTypePointer Output %int
     %ptr_out_v2int = OpTypePointer Output %v2int
     %ptr_out_float = OpTypePointer Output %float
     %ptr_out_v2float = OpTypePointer Output %v2float
     %1 = OpVariable %ptr_out_uint Output
     %2 = OpVariable %ptr_out_v2uint Output
     %3 = OpVariable %ptr_out_int Output
     %4 = OpVariable %ptr_out_v2int Output
     %5 = OpVariable %ptr_out_float Output
     %6 = OpVariable %ptr_out_v2float Output

     %ptr_out_v4float = OpTypePointer Output %v4float
     %10 = OpVariable %ptr_out_v4float Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Module{
  Struct main_out {
    StructMember{[[ LocationDecoration{1}
 ]] x_1_1: __u32}
    StructMember{[[ LocationDecoration{2}
 ]] x_2_1: __vec_2__u32}
    StructMember{[[ LocationDecoration{3}
 ]] x_3_1: __i32}
    StructMember{[[ LocationDecoration{4}
 ]] x_4_1: __vec_2__i32}
    StructMember{[[ LocationDecoration{5}
 InterpolateDecoration{flat none}
 ]] x_5_1: __f32}
    StructMember{[[ LocationDecoration{6}
 InterpolateDecoration{flat none}
 ]] x_6_1: __vec_2__f32}
    StructMember{[[ BuiltinDecoration{position}
 ]] x_10_1: __vec_4__f32}
  }
  Variable{
    x_1
    private
    undefined
    __u32
  }
  Variable{
    x_2
    private
    undefined
    __vec_2__u32
  }
  Variable{
    x_3
    private
    undefined
    __i32
  }
  Variable{
    x_4
    private
    undefined
    __vec_2__i32
  }
  Variable{
    x_5
    private
    undefined
    __f32
  }
  Variable{
    x_6
    private
    undefined
    __vec_2__f32
  }
  Variable{
    x_10
    private
    undefined
    __vec_4__f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{vertex}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_1}
          Identifier[not set]{x_2}
          Identifier[not set]{x_3}
          Identifier[not set]{x_4}
          Identifier[not set]{x_5}
          Identifier[not set]{x_6}
          Identifier[not set]{x_10}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_Flatten_Interpolation_Flat_Fragment_In) {
  // Flat decorations are dropped for integral
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 1
     OpDecorate %2 Location 5
     OpDecorate %1 Flat
     OpDecorate %2 Flat
)" + CommonTypes() +
                        R"(
     %arr = OpTypeArray %float %uint_2
     %strct = OpTypeStruct %float %float
     %ptr_in_arr = OpTypePointer Input %arr
     %ptr_in_strct = OpTypePointer Input %strct
     %1 = OpVariable %ptr_in_arr Input
     %2 = OpVariable %ptr_in_strct Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Module{
  Struct S {
    StructMember{field0: __f32}
    StructMember{field1: __f32}
  }
  Variable{
    x_1
    private
    undefined
    __array__f32_2
  }
  Variable{
    x_2
    private
    undefined
    __type_name_S
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        LocationDecoration{1}
        InterpolateDecoration{flat none}
      }
      x_1_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{2}
        InterpolateDecoration{flat none}
      }
      x_1_param_1
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{5}
        InterpolateDecoration{flat none}
      }
      x_2_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{6}
        InterpolateDecoration{flat none}
      }
      x_2_param_1
      none
      undefined
      __f32
    }
  )
  {
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{0}
      }
      Identifier[not set]{x_1_param}
    }
    Assignment{
      ArrayAccessor[not set]{
        Identifier[not set]{x_1}
        ScalarConstructor[not set]{1}
      }
      Identifier[not set]{x_1_param_1}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_2}
        Identifier[not set]{field0}
      }
      Identifier[not set]{x_2_param}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_2}
        Identifier[not set]{field1}
      }
      Identifier[not set]{x_2_param_1}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_Interpolation_Floating_Fragment_In) {
  // Flat decorations are dropped for integral
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2 %3 %4 %5 %6
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6

     ; %1 perspective center

     OpDecorate %2 Centroid ; perspective centroid

     OpDecorate %3 Sample ; perspective sample

     OpDecorate %4 NoPerspective; linear center

     OpDecorate %5 NoPerspective ; linear centroid
     OpDecorate %5 Centroid

     OpDecorate %6 NoPerspective ; linear sample
     OpDecorate %6 Sample

)" + CommonTypes() +
                        R"(
     %ptr_in_float = OpTypePointer Input %float
     %1 = OpVariable %ptr_in_float Input
     %2 = OpVariable %ptr_in_float Input
     %3 = OpVariable %ptr_in_float Input
     %4 = OpVariable %ptr_in_float Input
     %5 = OpVariable %ptr_in_float Input
     %6 = OpVariable %ptr_in_float Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Module{
  Variable{
    x_1
    private
    undefined
    __f32
  }
  Variable{
    x_2
    private
    undefined
    __f32
  }
  Variable{
    x_3
    private
    undefined
    __f32
  }
  Variable{
    x_4
    private
    undefined
    __f32
  }
  Variable{
    x_5
    private
    undefined
    __f32
  }
  Variable{
    x_6
    private
    undefined
    __f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        LocationDecoration{1}
      }
      x_1_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{2}
        InterpolateDecoration{perspective centroid}
      }
      x_2_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{3}
        InterpolateDecoration{perspective sample}
      }
      x_3_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{4}
        InterpolateDecoration{linear none}
      }
      x_4_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{5}
        InterpolateDecoration{linear centroid}
      }
      x_5_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{6}
        InterpolateDecoration{linear sample}
      }
      x_6_param
      none
      undefined
      __f32
    }
  )
  {
    Assignment{
      Identifier[not set]{x_1}
      Identifier[not set]{x_1_param}
    }
    Assignment{
      Identifier[not set]{x_2}
      Identifier[not set]{x_2_param}
    }
    Assignment{
      Identifier[not set]{x_3}
      Identifier[not set]{x_3_param}
    }
    Assignment{
      Identifier[not set]{x_4}
      Identifier[not set]{x_4_param}
    }
    Assignment{
      Identifier[not set]{x_5}
      Identifier[not set]{x_5_param}
    }
    Assignment{
      Identifier[not set]{x_6}
      Identifier[not set]{x_6_param}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_Flatten_Interpolation_Floating_Fragment_In) {
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 1

     ; member 0 perspective center

     OpMemberDecorate %10 1 Centroid ; perspective centroid

     OpMemberDecorate %10 2 Sample ; perspective sample

     OpMemberDecorate %10 3 NoPerspective; linear center

     OpMemberDecorate %10 4 NoPerspective ; linear centroid
     OpMemberDecorate %10 4 Centroid

     OpMemberDecorate %10 5 NoPerspective ; linear sample
     OpMemberDecorate %10 5 Sample

)" + CommonTypes() +
                        R"(

     %10 = OpTypeStruct %float %float %float %float %float %float
     %ptr_in_strct = OpTypePointer Input %10
     %1 = OpVariable %ptr_in_strct Input

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModule()) << assembly << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Module{
  Struct S {
    StructMember{field0: __f32}
    StructMember{field1: __f32}
    StructMember{field2: __f32}
    StructMember{field3: __f32}
    StructMember{field4: __f32}
    StructMember{field5: __f32}
  }
  Variable{
    x_1
    private
    undefined
    __type_name_S
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __void
  StageDecoration{fragment}
  (
    VariableConst{
      Decorations{
        LocationDecoration{1}
      }
      x_1_param
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{2}
        InterpolateDecoration{perspective centroid}
      }
      x_1_param_1
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{3}
        InterpolateDecoration{perspective sample}
      }
      x_1_param_2
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{4}
        InterpolateDecoration{linear none}
      }
      x_1_param_3
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{5}
        InterpolateDecoration{linear centroid}
      }
      x_1_param_4
      none
      undefined
      __f32
    }
    VariableConst{
      Decorations{
        LocationDecoration{6}
        InterpolateDecoration{linear sample}
      }
      x_1_param_5
      none
      undefined
      __f32
    }
  )
  {
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{field0}
      }
      Identifier[not set]{x_1_param}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{field1}
      }
      Identifier[not set]{x_1_param_1}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{field2}
      }
      Identifier[not set]{x_1_param_2}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{field3}
      }
      Identifier[not set]{x_1_param_3}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{field4}
      }
      Identifier[not set]{x_1_param_4}
    }
    Assignment{
      MemberAccessor[not set]{
        Identifier[not set]{x_1}
        Identifier[not set]{field5}
      }
      Identifier[not set]{x_1_param_5}
    }
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_Interpolation_Floating_Fragment_Out) {
  // Flat decorations are dropped for integral
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1 %2 %3 %4 %5 %6
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %1 Location 1
     OpDecorate %2 Location 2
     OpDecorate %3 Location 3
     OpDecorate %4 Location 4
     OpDecorate %5 Location 5
     OpDecorate %6 Location 6

     ; %1 perspective center

     OpDecorate %2 Centroid ; perspective centroid

     OpDecorate %3 Sample ; perspective sample

     OpDecorate %4 NoPerspective; linear center

     OpDecorate %5 NoPerspective ; linear centroid
     OpDecorate %5 Centroid

     OpDecorate %6 NoPerspective ; linear sample
     OpDecorate %6 Sample

)" + CommonTypes() +
                        R"(
     %ptr_out_float = OpTypePointer Output %float
     %1 = OpVariable %ptr_out_float Output
     %2 = OpVariable %ptr_out_float Output
     %3 = OpVariable %ptr_out_float Output
     %4 = OpVariable %ptr_out_float Output
     %5 = OpVariable %ptr_out_float Output
     %6 = OpVariable %ptr_out_float Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Module{
  Struct main_out {
    StructMember{[[ LocationDecoration{1}
 ]] x_1_1: __f32}
    StructMember{[[ LocationDecoration{2}
 InterpolateDecoration{perspective centroid}
 ]] x_2_1: __f32}
    StructMember{[[ LocationDecoration{3}
 InterpolateDecoration{perspective sample}
 ]] x_3_1: __f32}
    StructMember{[[ LocationDecoration{4}
 InterpolateDecoration{linear none}
 ]] x_4_1: __f32}
    StructMember{[[ LocationDecoration{5}
 InterpolateDecoration{linear centroid}
 ]] x_5_1: __f32}
    StructMember{[[ LocationDecoration{6}
 InterpolateDecoration{linear sample}
 ]] x_6_1: __f32}
  }
  Variable{
    x_1
    private
    undefined
    __f32
  }
  Variable{
    x_2
    private
    undefined
    __f32
  }
  Variable{
    x_3
    private
    undefined
    __f32
  }
  Variable{
    x_4
    private
    undefined
    __f32
  }
  Variable{
    x_5
    private
    undefined
    __f32
  }
  Variable{
    x_6
    private
    undefined
    __f32
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          Identifier[not set]{x_1}
          Identifier[not set]{x_2}
          Identifier[not set]{x_3}
          Identifier[not set]{x_4}
          Identifier[not set]{x_5}
          Identifier[not set]{x_6}
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

TEST_F(SpvModuleScopeVarParserTest,
       EntryPointWrapping_Flatten_Interpolation_Floating_Fragment_Out) {
  const auto assembly = CommonCapabilities() + R"(
     OpEntryPoint Fragment %main "main" %1
     OpExecutionMode %main OriginUpperLeft

     OpDecorate %1 Location 1

     ; member 0 perspective center

     OpMemberDecorate %10 1 Centroid ; perspective centroid

     OpMemberDecorate %10 2 Sample ; perspective sample

     OpMemberDecorate %10 3 NoPerspective; linear center

     OpMemberDecorate %10 4 NoPerspective ; linear centroid
     OpMemberDecorate %10 4 Centroid

     OpMemberDecorate %10 5 NoPerspective ; linear sample
     OpMemberDecorate %10 5 Sample

)" + CommonTypes() +
                        R"(

     %10 = OpTypeStruct %float %float %float %float %float %float
     %ptr_in_strct = OpTypePointer Output %10
     %1 = OpVariable %ptr_in_strct Output

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));

  ASSERT_TRUE(p->BuildAndParseInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto got = p->program().to_str();
  const std::string expected =
      R"(Module{
  Struct S {
    StructMember{field0: __f32}
    StructMember{field1: __f32}
    StructMember{field2: __f32}
    StructMember{field3: __f32}
    StructMember{field4: __f32}
    StructMember{field5: __f32}
  }
  Struct main_out {
    StructMember{[[ LocationDecoration{1}
 ]] x_1_1: __f32}
    StructMember{[[ LocationDecoration{2}
 InterpolateDecoration{perspective centroid}
 ]] x_1_2: __f32}
    StructMember{[[ LocationDecoration{3}
 InterpolateDecoration{perspective sample}
 ]] x_1_3: __f32}
    StructMember{[[ LocationDecoration{4}
 InterpolateDecoration{linear none}
 ]] x_1_4: __f32}
    StructMember{[[ LocationDecoration{5}
 InterpolateDecoration{linear centroid}
 ]] x_1_5: __f32}
    StructMember{[[ LocationDecoration{6}
 InterpolateDecoration{linear sample}
 ]] x_1_6: __f32}
  }
  Variable{
    x_1
    private
    undefined
    __type_name_S
  }
  Function main_1 -> __void
  ()
  {
    Return{}
  }
  Function main -> __type_name_main_out
  StageDecoration{fragment}
  ()
  {
    Call[not set]{
      Identifier[not set]{main_1}
      (
      )
    }
    Return{
      {
        TypeConstructor[not set]{
          __type_name_main_out
          MemberAccessor[not set]{
            Identifier[not set]{x_1}
            Identifier[not set]{field0}
          }
          MemberAccessor[not set]{
            Identifier[not set]{x_1}
            Identifier[not set]{field1}
          }
          MemberAccessor[not set]{
            Identifier[not set]{x_1}
            Identifier[not set]{field2}
          }
          MemberAccessor[not set]{
            Identifier[not set]{x_1}
            Identifier[not set]{field3}
          }
          MemberAccessor[not set]{
            Identifier[not set]{x_1}
            Identifier[not set]{field4}
          }
          MemberAccessor[not set]{
            Identifier[not set]{x_1}
            Identifier[not set]{field5}
          }
        }
      }
    }
  }
}
)";
  EXPECT_EQ(got, expected) << got;
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
