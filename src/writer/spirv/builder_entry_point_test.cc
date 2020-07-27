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
#include "src/ast/entry_point.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, EntryPoint) {
  ast::type::VoidType void_type;

  ast::Function func("frag_main", {}, &void_type);
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "main", "frag_main");

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateEntryPoint(&ep)) << b.error();

  EXPECT_EQ(DumpInstructions(b.preamble()), R"(OpEntryPoint Fragment %3 "main"
)");
}

TEST_F(BuilderTest, EntryPoint_WithoutName) {
  ast::type::VoidType void_type;

  ast::Function func("compute_main", {}, &void_type);
  ast::EntryPoint ep(ast::PipelineStage::kCompute, "", "compute_main");

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateEntryPoint(&ep)) << b.error();

  EXPECT_EQ(DumpInstructions(b.preamble()),
            R"(OpEntryPoint GLCompute %3 "compute_main"
)");
}

TEST_F(BuilderTest, EntryPoint_BadFunction) {
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "main", "frag_main");

  ast::Module mod;
  Builder b(&mod);
  EXPECT_FALSE(b.GenerateEntryPoint(&ep));
  EXPECT_EQ(b.error(), "unable to find ID for function: frag_main");
}

struct EntryPointStageData {
  ast::PipelineStage stage;
  SpvExecutionModel model;
};
inline std::ostream& operator<<(std::ostream& out, EntryPointStageData data) {
  out << data.stage;
  return out;
}
using EntryPointStageTest = testing::TestWithParam<EntryPointStageData>;
TEST_P(EntryPointStageTest, Emit) {
  auto params = GetParam();

  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  ast::EntryPoint ep(params.stage, "", "main");

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateEntryPoint(&ep)) << b.error();

  auto preamble = b.preamble();
  ASSERT_EQ(preamble.size(), 1u);
  EXPECT_EQ(preamble[0].opcode(), spv::Op::OpEntryPoint);

  ASSERT_GE(preamble[0].operands().size(), 3u);
  EXPECT_EQ(preamble[0].operands()[0].to_i(), params.model);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    EntryPointStageTest,
    testing::Values(EntryPointStageData{ast::PipelineStage::kVertex,
                                        SpvExecutionModelVertex},
                    EntryPointStageData{ast::PipelineStage::kFragment,
                                        SpvExecutionModelFragment},
                    EntryPointStageData{ast::PipelineStage::kCompute,
                                        SpvExecutionModelGLCompute}));

TEST_F(BuilderTest, EntryPoint_WithUnusedInterfaceIds) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);

  auto v_in =
      std::make_unique<ast::Variable>("my_in", ast::StorageClass::kInput, &f32);
  auto v_out = std::make_unique<ast::Variable>(
      "my_out", ast::StorageClass::kOutput, &f32);
  auto v_wg = std::make_unique<ast::Variable>(
      "my_wg", ast::StorageClass::kWorkgroup, &f32);
  ast::EntryPoint ep(ast::PipelineStage::kVertex, "", "main");

  ast::Module mod;
  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(v_in.get())) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_out.get())) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_wg.get())) << b.error();

  mod.AddGlobalVariable(std::move(v_in));
  mod.AddGlobalVariable(std::move(v_out));
  mod.AddGlobalVariable(std::move(v_wg));

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateEntryPoint(&ep)) << b.error();
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
  EXPECT_EQ(DumpInstructions(b.preamble()),
            R"(OpEntryPoint Vertex %11 "main"
)");
}

TEST_F(BuilderTest, EntryPoint_WithUsedInterfaceIds) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);

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
  ast::EntryPoint ep(ast::PipelineStage::kVertex, "", "main");

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
  ASSERT_TRUE(b.GenerateEntryPoint(&ep)) << b.error();
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
  EXPECT_EQ(DumpInstructions(b.preamble()),
            R"(OpEntryPoint Vertex %11 "main" %4 %1
)");
}

TEST_F(BuilderTest, ExecutionModel_Fragment_OriginUpperLeft) {
  ast::type::VoidType void_type;

  ast::Function func("frag_main", {}, &void_type);
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "main", "frag_main");

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateExecutionModes(&ep));

  EXPECT_EQ(DumpInstructions(b.preamble()),
            R"(OpExecutionMode %3 OriginUpperLeft
)");
}

TEST_F(BuilderTest, ExecutionModel_Compute_LocalSize) {
  ast::type::VoidType void_type;

  ast::Function func("main", {}, &void_type);
  ast::EntryPoint ep(ast::PipelineStage::kCompute, "main", "main");

  ast::Module mod;
  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateExecutionModes(&ep));

  EXPECT_EQ(DumpInstructions(b.preamble()),
            R"(OpExecutionMode %3 LocalSize 1 1 1
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
