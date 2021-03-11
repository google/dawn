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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/builtin.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/location_decoration.h"
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
       ast::FunctionDecorationList{
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

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
