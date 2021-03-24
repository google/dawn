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
  //              [[location(1)]] loc1 : f32) -> void {
  //   var col : f32 = (coord.x * loc1);
  // }
  auto* f32 = ty.f32();
  auto* vec4 = ty.vec4<float>();
  auto* coord = Var("coord", vec4, ast::StorageClass::kInput, nullptr,
                    {create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord)});
  auto* loc1 = Var("loc1", f32, ast::StorageClass::kInput, nullptr,
                   {create<ast::LocationDecoration>(1u)});
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
OpName %1 "tint_symbol_1"
OpName %5 "tint_symbol_2"
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
  auto* loc_in = Var("loc_in", u32, ast::StorageClass::kFunction, nullptr,
                     {create<ast::LocationDecoration>(0)});
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
OpEntryPoint Fragment %15 "frag_main" %1 %4
OpExecutionMode %15 OriginUpperLeft
OpName %1 "tint_symbol_1"
OpName %4 "tint_symbol_3"
OpName %10 "tint_symbol_4"
OpName %11 "tint_symbol_2"
OpName %15 "frag_main"
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
%14 = OpTypeFunction %9
%18 = OpConstant %3 10
%20 = OpTypeBool
%24 = OpConstant %6 0.5
%26 = OpConstant %6 1
%10 = OpFunction %9 None %8
%11 = OpFunctionParameter %6
%12 = OpLabel
%13 = OpLoad %6 %11
OpStore %4 %13
OpReturn
OpFunctionEnd
%15 = OpFunction %9 None %14
%16 = OpLabel
%17 = OpLoad %3 %1
%19 = OpUGreaterThan %20 %17 %18
OpSelectionMerge %21 None
OpBranchConditional %19 %22 %21
%22 = OpLabel
%23 = OpFunctionCall %9 %10 %24
OpReturn
%21 = OpLabel
%25 = OpFunctionCall %9 %10 %26
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, EntryPoint_SharedSubStruct) {
  // struct Interface {
  //   [[location(1)]] value : f32;
  // };
  //
  // struct VertexOutput {
  //   [[builtin(position)]] pos : vec4<f32>;
  //   interface : Interface;
  // };
  //
  // struct FragmentInput {
  //   [[location(0)]] mul : f32;
  //   interface : Interface;
  // };
  //
  // [[stage(vertex)]]
  // fn vert_main() -> VertexOutput {
  //   return VertexOutput(vec4<f32>(), Interface(42.0));
  // }
  //
  // [[stage(fragment)]]
  // fn frag_main(inputs : FragmentInput) -> [[builtin(frag_depth)]] f32 {
  //   return inputs.mul * inputs.interface.value;
  // }

  auto* interface =
      Structure("Interface",
                ast::StructMemberList{Member(
                    "value", ty.f32(),
                    ast::DecorationList{create<ast::LocationDecoration>(1u)})});
  auto* vertex_output = Structure(
      "VertexOutput",
      ast::StructMemberList{
          Member("pos", ty.vec4<f32>(),
                 ast::DecorationList{
                     create<ast::BuiltinDecoration>(ast::Builtin::kPosition)}),
          Member("interface", interface)});
  auto* fragment_input = Structure(
      "FragmentInput",
      ast::StructMemberList{
          Member("mul", ty.f32(),
                 ast::DecorationList{create<ast::LocationDecoration>(0u)}),
          Member("interface", interface)});

  auto* vert_retval = Construct(vertex_output, Construct(ty.vec4<f32>()),
                                Construct(interface, 42.f));
  Func("vert_main", ast::VariableList{}, vertex_output,
       ast::StatementList{
           create<ast::ReturnStatement>(vert_retval),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  auto* frag_retval =
      Mul(MemberAccessor(Expr("inputs"), "mul"),
          MemberAccessor(MemberAccessor(Expr("inputs"), "interface"), "value"));
  auto* frag_inputs =
      Var("inputs", fragment_input, ast::StorageClass::kFunction, nullptr);
  Func("frag_main", ast::VariableList{frag_inputs}, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(frag_retval),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       },
       ast::DecorationList{
           create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build()) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %30 "vert_main" %1 %6
OpEntryPoint Fragment %41 "frag_main" %11 %9 %12
OpExecutionMode %41 OriginUpperLeft
OpExecutionMode %41 DepthReplacing
OpName %1 "tint_symbol_9"
OpName %6 "tint_symbol_10"
OpName %9 "tint_symbol_13"
OpName %11 "tint_symbol_14"
OpName %12 "tint_symbol_18"
OpName %15 "VertexOutput"
OpMemberName %15 0 "pos"
OpMemberName %15 1 "interface"
OpName %16 "Interface"
OpMemberName %16 0 "value"
OpName %17 "tint_symbol_11"
OpName %18 "tint_symbol_8"
OpName %30 "vert_main"
OpName %37 "tint_symbol_19"
OpName %38 "tint_symbol_17"
OpName %41 "frag_main"
OpName %45 "tint_symbol_15"
OpName %48 "FragmentInput"
OpMemberName %48 0 "mul"
OpMemberName %48 1 "interface"
OpName %52 "tint_symbol_16"
OpDecorate %1 BuiltIn Position
OpDecorate %6 Location 1
OpDecorate %9 Location 0
OpDecorate %11 Location 1
OpDecorate %12 BuiltIn FragDepth
OpMemberDecorate %15 0 Offset 0
OpMemberDecorate %15 1 Offset 16
OpMemberDecorate %16 0 Offset 0
OpMemberDecorate %48 0 Offset 0
OpMemberDecorate %48 1 Offset 4
%4 = OpTypeFloat 32
%3 = OpTypeVector %4 4
%2 = OpTypePointer Output %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Output %5
%7 = OpTypePointer Output %4
%8 = OpConstantNull %4
%6 = OpVariable %7 Output %8
%10 = OpTypePointer Input %4
%9 = OpVariable %10 Input
%11 = OpVariable %10 Input
%12 = OpVariable %7 Output %8
%14 = OpTypeVoid
%16 = OpTypeStruct %4
%15 = OpTypeStruct %3 %16
%13 = OpTypeFunction %14 %15
%20 = OpTypeInt 32 0
%21 = OpConstant %20 0
%22 = OpTypePointer Function %3
%25 = OpConstant %20 1
%26 = OpTypePointer Function %4
%29 = OpTypeFunction %14
%33 = OpConstant %4 42
%34 = OpConstantComposite %16 %33
%35 = OpConstantComposite %15 %5 %34
%36 = OpTypeFunction %14 %4
%46 = OpTypePointer Function %16
%47 = OpConstantNull %16
%48 = OpTypeStruct %4 %16
%53 = OpTypePointer Function %48
%54 = OpConstantNull %48
%17 = OpFunction %14 None %13
%18 = OpFunctionParameter %15
%19 = OpLabel
%23 = OpAccessChain %22 %18 %21
%24 = OpLoad %3 %23
OpStore %1 %24
%27 = OpAccessChain %26 %18 %25 %21
%28 = OpLoad %4 %27
OpStore %6 %28
OpReturn
OpFunctionEnd
%30 = OpFunction %14 None %29
%31 = OpLabel
%32 = OpFunctionCall %14 %17 %35
OpReturn
OpFunctionEnd
%37 = OpFunction %14 None %36
%38 = OpFunctionParameter %4
%39 = OpLabel
%40 = OpLoad %4 %38
OpStore %12 %40
OpReturn
OpFunctionEnd
%41 = OpFunction %14 None %29
%42 = OpLabel
%45 = OpVariable %46 Function %47
%52 = OpVariable %53 Function %54
%43 = OpLoad %4 %11
%44 = OpCompositeConstruct %16 %43
OpStore %45 %44
%49 = OpLoad %4 %9
%50 = OpLoad %16 %45
%51 = OpCompositeConstruct %48 %49 %50
OpStore %52 %51
%56 = OpAccessChain %26 %52 %21
%57 = OpLoad %4 %56
%58 = OpAccessChain %26 %52 %25 %21
%59 = OpLoad %4 %58
%60 = OpFMul %4 %57 %59
%55 = OpFunctionCall %14 %37 %60
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
