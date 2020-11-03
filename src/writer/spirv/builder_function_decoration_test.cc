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

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/ast/assignment_statement.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/workgroup_decoration.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, FunctionDecoration_Stage) {
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  func.add_decoration(std::make_unique<ast::StageDecoration>(
      ast::PipelineStage::kVertex, Source{}));

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  EXPECT_EQ(DumpInstructions(b.entry_points()), R"(OpEntryPoint Vertex %3 "main"
)");
}

struct FunctionStageData {
  ast::PipelineStage stage;
  SpvExecutionModel model;
};
inline std::ostream& operator<<(std::ostream& out, FunctionStageData data) {
  out << data.stage;
  return out;
}
using FunctionDecoration_StageTest = testing::TestWithParam<FunctionStageData>;
TEST_P(FunctionDecoration_StageTest, Emit) {
  auto params = GetParam();

  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  func.add_decoration(
      std::make_unique<ast::StageDecoration>(params.stage, Source{}));

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  auto preamble = b.entry_points();
  ASSERT_GE(preamble.size(), 1u);
  EXPECT_EQ(preamble[0].opcode(), spv::Op::OpEntryPoint);

  ASSERT_GE(preamble[0].operands().size(), 3u);
  EXPECT_EQ(preamble[0].operands()[0].to_i(), params.model);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    FunctionDecoration_StageTest,
    testing::Values(FunctionStageData{ast::PipelineStage::kVertex,
                                      SpvExecutionModelVertex},
                    FunctionStageData{ast::PipelineStage::kFragment,
                                      SpvExecutionModelFragment},
                    FunctionStageData{ast::PipelineStage::kCompute,
                                      SpvExecutionModelGLCompute}));

TEST_F(BuilderTest, FunctionDecoration_Stage_WithUnusedInterfaceIds) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  func.add_decoration(std::make_unique<ast::StageDecoration>(
      ast::PipelineStage::kVertex, Source{}));
  auto v_in =
      std::make_unique<ast::Variable>("my_in", ast::StorageClass::kInput, &f32);
  auto v_out = std::make_unique<ast::Variable>(
      "my_out", ast::StorageClass::kOutput, &f32);
  auto v_wg = std::make_unique<ast::Variable>(
      "my_wg", ast::StorageClass::kWorkgroup, &f32);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(v_in.get())) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_out.get())) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_wg.get())) << b.error();

  mod.AddGlobalVariable(std::move(v_in));
  mod.AddGlobalVariable(std::move(v_out));
  mod.AddGlobalVariable(std::move(v_wg));

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "my_in"
OpName %4 "my_out"
OpName %7 "my_wg"
OpName %11 "main"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
%5 = OpTypePointer Output %3
%6 = OpConstantNull %3
%4 = OpVariable %5 Output %6
%8 = OpTypePointer Workgroup %3
%7 = OpVariable %8 Workgroup
%10 = OpTypeVoid
%9 = OpTypeFunction %10
)");
  EXPECT_EQ(DumpInstructions(b.entry_points()),
            R"(OpEntryPoint Vertex %11 "main"
)");
}

TEST_F(BuilderTest, FunctionDecoration_Stage_WithUsedInterfaceIds) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  func.add_decoration(std::make_unique<ast::StageDecoration>(
      ast::PipelineStage::kVertex, Source{}));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("my_out"),
      std::make_unique<ast::IdentifierExpression>("my_in")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("my_wg"),
      std::make_unique<ast::IdentifierExpression>("my_wg")));
  // Add duplicate usages so we show they don't get output multiple times.
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("my_out"),
      std::make_unique<ast::IdentifierExpression>("my_in")));
  func.set_body(std::move(body));

  auto v_in =
      std::make_unique<ast::Variable>("my_in", ast::StorageClass::kInput, &f32);
  auto v_out = std::make_unique<ast::Variable>(
      "my_out", ast::StorageClass::kOutput, &f32);
  auto v_wg = std::make_unique<ast::Variable>(
      "my_wg", ast::StorageClass::kWorkgroup, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(v_in.get());
  td.RegisterVariableForTesting(v_out.get());
  td.RegisterVariableForTesting(v_wg.get());

  ASSERT_TRUE(td.DetermineFunction(&func)) << td.error();

  Builder b(&mod);

  EXPECT_TRUE(b.GenerateGlobalVariable(v_in.get())) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_out.get())) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_wg.get())) << b.error();

  mod.AddGlobalVariable(std::move(v_in));
  mod.AddGlobalVariable(std::move(v_out));
  mod.AddGlobalVariable(std::move(v_wg));

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "my_in"
OpName %4 "my_out"
OpName %7 "my_wg"
OpName %11 "main"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
%5 = OpTypePointer Output %3
%6 = OpConstantNull %3
%4 = OpVariable %5 Output %6
%8 = OpTypePointer Workgroup %3
%7 = OpVariable %8 Workgroup
%10 = OpTypeVoid
%9 = OpTypeFunction %10
)");
  EXPECT_EQ(DumpInstructions(b.entry_points()),
            R"(OpEntryPoint Vertex %11 "main" %4 %1
)");
}

TEST_F(BuilderTest, FunctionDecoration_ExecutionMode_Fragment_OriginUpperLeft) {
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  func.add_decoration(std::make_unique<ast::StageDecoration>(
      ast::PipelineStage::kFragment, Source{}));

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateExecutionModes(&func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 OriginUpperLeft
)");
}

TEST_F(BuilderTest, FunctionDecoration_WorkgroupSize_Default) {
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  func.add_decoration(std::make_unique<ast::StageDecoration>(
      ast::PipelineStage::kCompute, Source{}));

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateExecutionModes(&func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 LocalSize 1 1 1
)");
}

TEST_F(BuilderTest, FunctionDecoration_WorkgroupSize) {
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  func.add_decoration(
      std::make_unique<ast::WorkgroupDecoration>(2u, 4u, 6u, Source{}));
  func.add_decoration(std::make_unique<ast::StageDecoration>(
      ast::PipelineStage::kCompute, Source{}));

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateExecutionModes(&func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 LocalSize 2 4 6
)");
}

TEST_F(BuilderTest, FunctionDecoration_ExecutionMode_MultipleFragment) {
  ast::type::VoidType void_type;

  ast::Function func1("main1", {}, &void_type);
  func1.add_decoration(std::make_unique<ast::StageDecoration>(
      ast::PipelineStage::kFragment, Source{}));

  ast::Function func2("main2", {}, &void_type);
  func2.add_decoration(std::make_unique<ast::StageDecoration>(
      ast::PipelineStage::kFragment, Source{}));

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func1)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func2)) << b.error();
  EXPECT_EQ(DumpBuilder(b),
            R"(OpEntryPoint Fragment %3 "main1"
OpEntryPoint Fragment %5 "main2"
OpExecutionMode %3 OriginUpperLeft
OpExecutionMode %5 OriginUpperLeft
OpName %3 "main1"
OpName %5 "main2"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpFunctionEnd
%5 = OpFunction %2 None %1
%6 = OpLabel
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
