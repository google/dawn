// Copyright 2021 The Tint Authors.
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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/builtin.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/storage_class.h"
#include "src/ast/variable.h"
#include "src/program.h"
#include "src/type/f32_type.h"
#include "src/type/vector_type.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, EntryPoint_Parameters) {
  // [[stage(fragment)]]
  // fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>,
  //              [[location(1)]] loc1 : f32) {
  //   var col : f32 = (coord.x * loc1);
  // }
  auto* f32 = ty.f32();
  auto* vec4 = ty.vec4<float>();
  auto* coord =
      Param("coord", vec4,
            {create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord)});
  auto* loc1 = Param("loc1", f32, {create<ast::LocationDecoration>(1u)});
  auto* mul = Mul(Expr(MemberAccessor("coord", "x")), Expr("loc1"));
  auto* col = Var("col", f32, ast::StorageClass::kFunction, mul, {});
  Func("frag_main", ast::VariableList{coord, loc1}, ty.void_(),
       ast::StatementList{WrapInStatement(col)},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());

  // Test that "coord" and "loc1" get hoisted out to global variables with the
  // Input storage class, retaining their decorations.
  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %9 "frag_main" %1 %5
OpExecutionMode %9 OriginUpperLeft
OpName %1 "tint_symbol"
OpName %5 "tint_symbol_1"
OpName %9 "frag_main"
OpName %17 "col"
OpDecorate %1 BuiltIn FragCoord
OpDecorate %5 Location 1
%4 = OpTypeFloat 32
%3 = OpTypeVector %4 4
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
%6 = OpTypePointer Input %4
%5 = OpVariable %6 Input
%8 = OpTypeVoid
%7 = OpTypeFunction %8
%11 = OpTypeInt 32 0
%12 = OpConstant %11 0
%18 = OpTypePointer Function %4
%19 = OpConstantNull %4
%9 = OpFunction %8 None %7
%10 = OpLabel
%17 = OpVariable %18 Function %19
%13 = OpAccessChain %6 %1 %12
%14 = OpLoad %4 %13
%15 = OpLoad %4 %5
%16 = OpFMul %4 %14 %15
OpStore %17 %16
OpReturn
OpFunctionEnd
)");

  Validate(b);
}

TEST_F(BuilderTest, EntryPoint_ReturnValue) {
  // [[stage(fragment)]]
  // fn frag_main([[location(0)]] loc_in : u32) -> [[location(0)]] f32 {
  //   if (loc_in > 10) {
  //     return 0.5;
  //   }
  //   return 1.0;
  // }
  auto* f32 = ty.f32();
  auto* u32 = ty.u32();
  auto* loc_in = Param("loc_in", u32, {create<ast::LocationDecoration>(0)});
  auto* cond = create<ast::BinaryExpression>(ast::BinaryOp::kGreaterThan,
                                             Expr("loc_in"), Expr(10u));
  Func("frag_main", ast::VariableList{loc_in}, f32,
       ast::StatementList{
           If(cond, Block(create<ast::ReturnStatement>(Expr(0.5f)))),
           create<ast::ReturnStatement>(Expr(1.0f)),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       },
       ast::DecorationList{create<ast::LocationDecoration>(0)});

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());

  // Test that the return value gets hoisted out to a global variable with the
  // Output storage class, and the return statements are replaced with stores.
  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %14 "frag_main" %1 %4
OpExecutionMode %14 OriginUpperLeft
OpName %1 "tint_symbol"
OpName %4 "tint_symbol_2"
OpName %10 "tint_symbol_3"
OpName %11 "tint_symbol_1"
OpName %14 "frag_main"
OpDecorate %1 Location 0
OpDecorate %4 Location 0
%3 = OpTypeInt 32 0
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
%6 = OpTypeFloat 32
%5 = OpTypePointer Output %6
%7 = OpConstantNull %6
%4 = OpVariable %5 Output %7
%9 = OpTypeVoid
%8 = OpTypeFunction %9 %6
%13 = OpTypeFunction %9
%17 = OpConstant %3 10
%19 = OpTypeBool
%23 = OpConstant %6 0.5
%25 = OpConstant %6 1
%10 = OpFunction %9 None %8
%11 = OpFunctionParameter %6
%12 = OpLabel
OpStore %4 %11
OpReturn
OpFunctionEnd
%14 = OpFunction %9 None %13
%15 = OpLabel
%16 = OpLoad %3 %1
%18 = OpUGreaterThan %19 %16 %17
OpSelectionMerge %20 None
OpBranchConditional %18 %21 %20
%21 = OpLabel
%22 = OpFunctionCall %9 %10 %23
OpReturn
%20 = OpLabel
%24 = OpFunctionCall %9 %10 %25
OpReturn
OpFunctionEnd
)");

  Validate(b);
}

TEST_F(BuilderTest, EntryPoint_SharedStruct) {
  // struct Interface {
  //   [[location(1)]] value : f32;
  // };
  //
  // [[stage(vertex)]]
  // fn vert_main() -> Interface {
  //   return Interface(42.0);
  // }
  //
  // [[stage(fragment)]]
  // fn frag_main(inputs : Interface) -> [[builtin(frag_depth)]] f32 {
  //   return inputs.value;
  // }

  auto* interface = Structure(
      "Interface",
      {Member("value", ty.f32(),
              ast::DecorationList{create<ast::LocationDecoration>(1u)})});

  auto* vert_retval = Construct(interface, 42.f);
  Func("vert_main", ast::VariableList{}, interface,
       {create<ast::ReturnStatement>(vert_retval)},
       {create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  auto* frag_inputs = Param("inputs", interface);
  Func("frag_main", ast::VariableList{frag_inputs}, ty.f32(),
       {create<ast::ReturnStatement>(MemberAccessor(Expr("inputs"), "value"))},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)},
       {create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build()) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %16 "vert_main" %1
OpEntryPoint Fragment %25 "frag_main" %5 %7
OpExecutionMode %25 OriginUpperLeft
OpExecutionMode %25 DepthReplacing
OpName %1 "tint_symbol_1"
OpName %5 "tint_symbol_3"
OpName %7 "tint_symbol_6"
OpName %10 "Interface"
OpMemberName %10 0 "value"
OpName %11 "tint_symbol_2"
OpName %12 "tint_symbol"
OpName %16 "vert_main"
OpName %22 "tint_symbol_7"
OpName %23 "tint_symbol_5"
OpName %25 "frag_main"
OpDecorate %1 Location 1
OpDecorate %5 Location 1
OpDecorate %7 BuiltIn FragDepth
OpMemberDecorate %10 0 Offset 0
%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
%6 = OpTypePointer Input %3
%5 = OpVariable %6 Input
%7 = OpVariable %2 Output %4
%9 = OpTypeVoid
%10 = OpTypeStruct %3
%8 = OpTypeFunction %9 %10
%15 = OpTypeFunction %9
%19 = OpConstant %3 42
%20 = OpConstantComposite %10 %19
%21 = OpTypeFunction %9 %3
%11 = OpFunction %9 None %8
%12 = OpFunctionParameter %10
%13 = OpLabel
%14 = OpCompositeExtract %3 %12 0
OpStore %1 %14
OpReturn
OpFunctionEnd
%16 = OpFunction %9 None %15
%17 = OpLabel
%18 = OpFunctionCall %9 %11 %20
OpReturn
OpFunctionEnd
%22 = OpFunction %9 None %21
%23 = OpFunctionParameter %3
%24 = OpLabel
OpStore %7 %23
OpReturn
OpFunctionEnd
%25 = OpFunction %9 None %15
%26 = OpLabel
%27 = OpLoad %3 %5
%28 = OpCompositeConstruct %10 %27
%30 = OpCompositeExtract %3 %28 0
%29 = OpFunctionCall %9 %22 %30
OpReturn
OpFunctionEnd
)");

  Validate(b);
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
