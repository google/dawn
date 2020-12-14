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
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, FunctionDecoration_Stage) {
  ast::type::Void void_type;

  ast::Function func(
      Source{}, mod->RegisterSymbol("main"), "main", {}, &void_type,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kVertex),
      });

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  EXPECT_EQ(DumpInstructions(b.entry_points()),
            R"(OpEntryPoint Vertex %3 "main"
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
using FunctionDecoration_StageTest = TestParamHelper<FunctionStageData>;
TEST_P(FunctionDecoration_StageTest, Emit) {
  auto params = GetParam();

  ast::type::Void void_type;

  ast::Function func(
      Source{}, mod->RegisterSymbol("main"), "main", {}, &void_type,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, params.stage),
      });

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
  ast::type::F32 f32;
  ast::type::Void void_type;

  ast::Function func(
      Source{}, mod->RegisterSymbol("main"), "main", {}, &void_type,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kVertex),
      });

  auto* v_in =
      create<ast::Variable>(Source{},                        // source
                            "my_in",                         // name
                            ast::StorageClass::kInput,       // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* v_out =
      create<ast::Variable>(Source{},                        // source
                            "my_out",                        // name
                            ast::StorageClass::kOutput,      // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* v_wg =
      create<ast::Variable>(Source{},                        // source
                            "my_wg",                         // name
                            ast::StorageClass::kWorkgroup,   // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  EXPECT_TRUE(b.GenerateGlobalVariable(v_in)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_out)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_wg)) << b.error();

  mod->AddGlobalVariable(v_in);
  mod->AddGlobalVariable(v_out);
  mod->AddGlobalVariable(v_wg);

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
  ast::type::F32 f32;
  ast::type::Void void_type;

  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod->RegisterSymbol("my_out"), "my_out"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod->RegisterSymbol("my_in"), "my_in")),
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod->RegisterSymbol("my_wg"), "my_wg"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod->RegisterSymbol("my_wg"), "my_wg")),
                    // Add duplicate usages so we show they don't get output
                    // multiple times.
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod->RegisterSymbol("my_out"), "my_out"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod->RegisterSymbol("my_in"), "my_in")),
                });

  ast::Function func(
      Source{}, mod->RegisterSymbol("main"), "main", {}, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kVertex),
      });

  auto* v_in =
      create<ast::Variable>(Source{},                        // source
                            "my_in",                         // name
                            ast::StorageClass::kInput,       // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* v_out =
      create<ast::Variable>(Source{},                        // source
                            "my_out",                        // name
                            ast::StorageClass::kOutput,      // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* v_wg =
      create<ast::Variable>(Source{},                        // source
                            "my_wg",                         // name
                            ast::StorageClass::kWorkgroup,   // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  td.RegisterVariableForTesting(v_in);
  td.RegisterVariableForTesting(v_out);
  td.RegisterVariableForTesting(v_wg);

  ASSERT_TRUE(td.DetermineFunction(&func)) << td.error();

  EXPECT_TRUE(b.GenerateGlobalVariable(v_in)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_out)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_wg)) << b.error();

  mod->AddGlobalVariable(v_in);
  mod->AddGlobalVariable(v_out);
  mod->AddGlobalVariable(v_wg);

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
  ast::type::Void void_type;

  ast::Function func(
      Source{}, mod->RegisterSymbol("main"), "main", {}, &void_type,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kFragment),
      });

  ASSERT_TRUE(b.GenerateExecutionModes(&func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 OriginUpperLeft
)");
}

TEST_F(BuilderTest, FunctionDecoration_WorkgroupSize_Default) {
  ast::type::Void void_type;

  ast::Function func(
      Source{}, mod->RegisterSymbol("main"), "main", {}, &void_type,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kCompute),
      });

  ASSERT_TRUE(b.GenerateExecutionModes(&func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 LocalSize 1 1 1
)");
}

TEST_F(BuilderTest, FunctionDecoration_WorkgroupSize) {
  ast::type::Void void_type;

  ast::Function func(
      Source{}, mod->RegisterSymbol("main"), "main", {}, &void_type,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::WorkgroupDecoration>(Source{}, 2u, 4u, 6u),
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kCompute),
      });

  ASSERT_TRUE(b.GenerateExecutionModes(&func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 LocalSize 2 4 6
)");
}

TEST_F(BuilderTest, FunctionDecoration_ExecutionMode_MultipleFragment) {
  ast::type::Void void_type;

  ast::Function func1(
      Source{}, mod->RegisterSymbol("main1"), "main1", {}, &void_type,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kFragment),
      });

  ast::Function func2(
      Source{}, mod->RegisterSymbol("main2"), "main2", {}, &void_type,
      create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(Source{}, ast::PipelineStage::kFragment),
      });

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
OpReturn
OpFunctionEnd
%5 = OpFunction %2 None %1
%6 = OpLabel
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
