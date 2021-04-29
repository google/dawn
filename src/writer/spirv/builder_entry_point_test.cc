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
#include "src/sem/f32_type.h"
#include "src/sem/vector_type.h"
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
  // fn frag_main([[builtin(position)]] coord : vec4<f32>,
  //              [[location(1)]] loc1 : f32) {
  //   var col : f32 = (coord.x * loc1);
  // }
  auto f32 = ty.f32();
  auto vec4 = ty.vec4<float>();
  auto* coord = Param("coord", vec4, {Builtin(ast::Builtin::kPosition)});
  auto* loc1 = Param("loc1", f32, {Location(1u)});
  auto* mul = Mul(Expr(MemberAccessor("coord", "x")), Expr("loc1"));
  auto* col = Var("col", f32, ast::StorageClass::kFunction, mul, {});
  Func("frag_main", ast::VariableList{coord, loc1}, ty.void_(),
       ast::StatementList{WrapInStatement(col)},
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
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
  auto f32 = ty.f32();
  auto u32 = ty.u32();
  auto* loc_in = Param("loc_in", u32, {Location(0)});
  auto* cond = create<ast::BinaryExpression>(ast::BinaryOp::kGreaterThan,
                                             Expr("loc_in"), Expr(10u));
  Func("frag_main", ast::VariableList{loc_in}, f32,
       ast::StatementList{
           If(cond, Block(Return(0.5f))),
           Return(1.0f),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       },
       ast::DecorationList{Location(0)});

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
  //   [[builtin(position)]] pos : vec4<f32>;
  // };
  //
  // [[stage(vertex)]]
  // fn vert_main() -> Interface {
  //   return Interface(42.0, vec4<f32>());
  // }
  //
  // [[stage(fragment)]]
  // fn frag_main(inputs : Interface) -> [[builtin(frag_depth)]] f32 {
  //   return inputs.value;
  // }

  auto interface = Structure(
      "Interface",
      {
          Member("value", ty.f32(), ast::DecorationList{Location(1u)}),
          Member("pos", ty.vec4<f32>(),
                 ast::DecorationList{Builtin(ast::Builtin::kPosition)}),
      });

  auto* vert_retval = Construct(interface, 42.f, Construct(ty.vec4<f32>()));
  Func("vert_main", ast::VariableList{}, interface, {Return(vert_retval)},
       {Stage(ast::PipelineStage::kVertex)});

  auto* frag_inputs = Param("inputs", interface);
  Func("frag_main", ast::VariableList{frag_inputs}, ty.f32(),
       {Return(MemberAccessor(Expr("inputs"), "value"))},
       {Stage(ast::PipelineStage::kFragment)},
       {Builtin(ast::Builtin::kFragDepth)});

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build()) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %23 "vert_main" %1 %5
OpEntryPoint Fragment %32 "frag_main" %9 %11 %13
OpExecutionMode %32 OriginUpperLeft
OpExecutionMode %32 DepthReplacing
OpName %1 "tint_symbol_1"
OpName %5 "tint_symbol_2"
OpName %9 "tint_symbol_4"
OpName %11 "tint_symbol_5"
OpName %13 "tint_symbol_8"
OpName %16 "Interface"
OpMemberName %16 0 "value"
OpMemberName %16 1 "pos"
OpName %17 "tint_symbol_3"
OpName %18 "tint_symbol"
OpName %23 "vert_main"
OpName %29 "tint_symbol_9"
OpName %30 "tint_symbol_7"
OpName %32 "frag_main"
OpDecorate %1 Location 1
OpDecorate %5 BuiltIn Position
OpDecorate %9 Location 1
OpDecorate %11 BuiltIn FragCoord
OpDecorate %13 BuiltIn FragDepth
OpMemberDecorate %16 0 Offset 0
OpMemberDecorate %16 1 Offset 16
%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
%7 = OpTypeVector %3 4
%6 = OpTypePointer Output %7
%8 = OpConstantNull %7
%5 = OpVariable %6 Output %8
%10 = OpTypePointer Input %3
%9 = OpVariable %10 Input
%12 = OpTypePointer Input %7
%11 = OpVariable %12 Input
%13 = OpVariable %2 Output %4
%15 = OpTypeVoid
%16 = OpTypeStruct %3 %7
%14 = OpTypeFunction %15 %16
%22 = OpTypeFunction %15
%26 = OpConstant %3 42
%27 = OpConstantComposite %16 %26 %8
%28 = OpTypeFunction %15 %3
%17 = OpFunction %15 None %14
%18 = OpFunctionParameter %16
%19 = OpLabel
%20 = OpCompositeExtract %3 %18 0
OpStore %1 %20
%21 = OpCompositeExtract %7 %18 1
OpStore %5 %21
OpReturn
OpFunctionEnd
%23 = OpFunction %15 None %22
%24 = OpLabel
%25 = OpFunctionCall %15 %17 %27
OpReturn
OpFunctionEnd
%29 = OpFunction %15 None %28
%30 = OpFunctionParameter %3
%31 = OpLabel
OpStore %13 %30
OpReturn
OpFunctionEnd
%32 = OpFunction %15 None %22
%33 = OpLabel
%34 = OpLoad %3 %9
%35 = OpLoad %7 %11
%36 = OpCompositeConstruct %16 %34 %35
%38 = OpCompositeExtract %3 %36 0
%37 = OpFunctionCall %15 %29 %38
OpReturn
OpFunctionEnd
)");

  Validate(b);
}

// Test that stores to the PointSize builtin have an RHS which is constant 1.0.
TEST_F(BuilderTest, EntryPoint_ReturnPointSize) {
  // struct VertexOut {
  //   [[builtin(position)]] pos : vec4<f32>;
  //   [[builtin(pointsize)]] pointsize : f32;
  // };
  //
  // [[stage(vertex)]]
  // fn vert_main() -> VertexOutput {
  //   if (false) {
  //     return VertexOutput(vec4<f32>(), 1.0);
  //   }
  //   return VertexOutput(vec4<f32>(1.0, 2.0, 3.0, 0.0), 1.0);
  // }
  auto vertex_out = Structure(
      "VertexOut",
      {
          Member("pos", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)}),
          Member("pointsize", ty.f32(), {Builtin(ast::Builtin::kPointSize)}),
      });
  Func("vert_main", {}, vertex_out,
       {
           If(Expr(false), Block(Return(Construct(
                               vertex_out, Construct(ty.vec4<f32>()), 1.f)))),
           Return(Construct(
               vertex_out, Construct(ty.vec4<f32>(), 1.f, 2.f, 3.f, 0.f), 1.f)),
       },
       {Stage(ast::PipelineStage::kVertex)});

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());

  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %18 "vert_main" %1 %6
OpName %1 "tint_symbol_1"
OpName %6 "tint_symbol_2"
OpName %11 "VertexOut"
OpMemberName %11 0 "pos"
OpMemberName %11 1 "pointsize"
OpName %12 "tint_symbol_3"
OpName %13 "tint_symbol"
OpName %18 "vert_main"
OpDecorate %1 BuiltIn Position
OpDecorate %6 BuiltIn PointSize
OpMemberDecorate %11 0 Offset 0
OpMemberDecorate %11 1 Offset 16
%4 = OpTypeFloat 32
%3 = OpTypeVector %4 4
%2 = OpTypePointer Output %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Output %5
%7 = OpTypePointer Output %4
%8 = OpConstantNull %4
%6 = OpVariable %7 Output %8
%10 = OpTypeVoid
%11 = OpTypeStruct %3 %4
%9 = OpTypeFunction %10 %11
%16 = OpConstant %4 1
%17 = OpTypeFunction %10
%20 = OpTypeBool
%21 = OpConstantFalse %20
%25 = OpConstantComposite %11 %5 %16
%27 = OpConstant %4 2
%28 = OpConstant %4 3
%29 = OpConstant %4 0
%30 = OpConstantComposite %3 %16 %27 %28 %29
%31 = OpConstantComposite %11 %30 %16
%12 = OpFunction %10 None %9
%13 = OpFunctionParameter %11
%14 = OpLabel
%15 = OpCompositeExtract %3 %13 0
OpStore %1 %15
OpStore %6 %16
OpReturn
OpFunctionEnd
%18 = OpFunction %10 None %17
%19 = OpLabel
OpSelectionMerge %22 None
OpBranchConditional %21 %23 %22
%23 = OpLabel
%24 = OpFunctionCall %10 %12 %25
OpReturn
%22 = OpLabel
%26 = OpFunctionCall %10 %12 %31
OpReturn
OpFunctionEnd
)");

  Validate(b);
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
