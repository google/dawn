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
  auto* coord =
      Param("coord", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)});
  auto* loc1 = Param("loc1", ty.f32(), {Location(1u)});
  auto* mul = Mul(Expr(MemberAccessor("coord", "x")), Expr("loc1"));
  auto* col = Var("col", ty.f32(), ast::StorageClass::kNone, mul);
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
OpEntryPoint Fragment %19 "frag_main" %1 %5
OpExecutionMode %19 OriginUpperLeft
OpName %1 "coord_1"
OpName %5 "loc1_1"
OpName %9 "frag_main_inner"
OpName %10 "coord"
OpName %11 "loc1"
OpName %15 "col"
OpName %19 "frag_main"
OpDecorate %1 BuiltIn FragCoord
OpDecorate %5 Location 1
%4 = OpTypeFloat 32
%3 = OpTypeVector %4 4
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
%6 = OpTypePointer Input %4
%5 = OpVariable %6 Input
%8 = OpTypeVoid
%7 = OpTypeFunction %8 %3 %4
%16 = OpTypePointer Function %4
%17 = OpConstantNull %4
%18 = OpTypeFunction %8
%9 = OpFunction %8 None %7
%10 = OpFunctionParameter %3
%11 = OpFunctionParameter %4
%12 = OpLabel
%15 = OpVariable %16 Function %17
%13 = OpCompositeExtract %4 %10 0
%14 = OpFMul %4 %13 %11
OpStore %15 %14
OpReturn
OpFunctionEnd
%19 = OpFunction %8 None %18
%20 = OpLabel
%22 = OpLoad %3 %1
%23 = OpLoad %4 %5
%21 = OpFunctionCall %8 %9 %22 %23
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
  auto* loc_in = Param("loc_in", ty.u32(), {Location(0)});
  auto* cond = create<ast::BinaryExpression>(ast::BinaryOp::kGreaterThan,
                                             Expr("loc_in"), Expr(10u));
  Func("frag_main", ast::VariableList{loc_in}, ty.f32(),
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
OpEntryPoint Fragment %21 "frag_main" %1 %4
OpExecutionMode %21 OriginUpperLeft
OpName %1 "loc_in_1"
OpName %4 "value"
OpName %9 "frag_main_inner"
OpName %10 "loc_in"
OpName %21 "frag_main"
OpDecorate %1 Location 0
OpDecorate %1 Flat
OpDecorate %4 Location 0
%3 = OpTypeInt 32 0
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
%6 = OpTypeFloat 32
%5 = OpTypePointer Output %6
%7 = OpConstantNull %6
%4 = OpVariable %5 Output %7
%8 = OpTypeFunction %6 %3
%12 = OpConstant %3 10
%14 = OpTypeBool
%17 = OpConstant %6 0.5
%18 = OpConstant %6 1
%20 = OpTypeVoid
%19 = OpTypeFunction %20
%9 = OpFunction %6 None %8
%10 = OpFunctionParameter %3
%11 = OpLabel
%13 = OpUGreaterThan %14 %10 %12
OpSelectionMerge %15 None
OpBranchConditional %13 %16 %15
%16 = OpLabel
OpReturnValue %17
%15 = OpLabel
OpReturnValue %18
OpFunctionEnd
%21 = OpFunction %20 None %19
%22 = OpLabel
%24 = OpLoad %3 %1
%23 = OpFunctionCall %6 %9 %24
OpStore %4 %23
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

  auto* interface = Structure(
      "Interface",
      {
          Member("value", ty.f32(), ast::DecorationList{Location(1u)}),
          Member("pos", ty.vec4<f32>(),
                 ast::DecorationList{Builtin(ast::Builtin::kPosition)}),
      });

  auto* vert_retval =
      Construct(ty.Of(interface), 42.f, Construct(ty.vec4<f32>()));
  Func("vert_main", ast::VariableList{}, ty.Of(interface),
       {Return(vert_retval)}, {Stage(ast::PipelineStage::kVertex)});

  auto* frag_inputs = Param("inputs", ty.Of(interface));
  Func("frag_main", ast::VariableList{frag_inputs}, ty.f32(),
       {Return(MemberAccessor(Expr("inputs"), "value"))},
       {Stage(ast::PipelineStage::kFragment)},
       {Builtin(ast::Builtin::kFragDepth)});

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build()) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %22 "vert_main" %1 %5
OpEntryPoint Fragment %32 "frag_main" %9 %11 %13
OpExecutionMode %32 OriginUpperLeft
OpExecutionMode %32 DepthReplacing
OpName %1 "value_1"
OpName %5 "pos_1"
OpName %9 "value_2"
OpName %11 "pos_2"
OpName %13 "value_3"
OpName %15 "Interface"
OpMemberName %15 0 "value"
OpMemberName %15 1 "pos"
OpName %16 "vert_main_inner"
OpName %22 "vert_main"
OpName %28 "frag_main_inner"
OpName %29 "inputs"
OpName %32 "frag_main"
OpDecorate %1 Location 1
OpDecorate %5 BuiltIn Position
OpDecorate %9 Location 1
OpDecorate %11 BuiltIn FragCoord
OpDecorate %13 BuiltIn FragDepth
OpMemberDecorate %15 0 Offset 0
OpMemberDecorate %15 1 Offset 16
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
%15 = OpTypeStruct %3 %7
%14 = OpTypeFunction %15
%18 = OpConstant %3 42
%19 = OpConstantComposite %15 %18 %8
%21 = OpTypeVoid
%20 = OpTypeFunction %21
%27 = OpTypeFunction %3 %15
%16 = OpFunction %15 None %14
%17 = OpLabel
OpReturnValue %19
OpFunctionEnd
%22 = OpFunction %21 None %20
%23 = OpLabel
%24 = OpFunctionCall %15 %16
%25 = OpCompositeExtract %3 %24 0
OpStore %1 %25
%26 = OpCompositeExtract %7 %24 1
OpStore %5 %26
OpReturn
OpFunctionEnd
%28 = OpFunction %3 None %27
%29 = OpFunctionParameter %15
%30 = OpLabel
%31 = OpCompositeExtract %3 %29 0
OpReturnValue %31
OpFunctionEnd
%32 = OpFunction %21 None %20
%33 = OpLabel
%35 = OpLoad %3 %9
%36 = OpLoad %7 %11
%37 = OpCompositeConstruct %15 %35 %36
%34 = OpFunctionCall %3 %28 %37
OpStore %13 %34
OpReturn
OpFunctionEnd
)");

  Validate(b);
}

TEST_F(BuilderTest, SampleIndex_SampleRateShadingCapability) {
  Func("main",
       {Param("sample_index", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)})},
       ty.void_(), {}, {Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build()) << b.error();

  // Make sure we generate the SampleRateShading capability.
  EXPECT_EQ(DumpInstructions(b.capabilities()),
            "OpCapability Shader\n"
            "OpCapability SampleRateShading\n");
  EXPECT_EQ(DumpInstructions(b.annots()), "OpDecorate %1 BuiltIn SampleId\n");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
