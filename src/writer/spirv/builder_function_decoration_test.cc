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

#include "src/ast/stage_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Decoration_Stage) {
  auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kFragment),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();
  EXPECT_EQ(DumpInstructions(b.entry_points()),
            R"(OpEntryPoint Fragment %3 "main"
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
using Decoration_StageTest = TestParamHelper<FunctionStageData>;
TEST_P(Decoration_StageTest, Emit) {
  auto params = GetParam();

  ast::Variable* var = nullptr;
  ast::StatementList body;
  if (params.stage == ast::PipelineStage::kVertex) {
    var = Global("pos", ty.vec4<f32>(), ast::StorageClass::kOutput, nullptr,
                 ast::DecorationList{Builtin(ast::Builtin::kPosition)});
    body.push_back(Assign("pos", Construct(ty.vec4<f32>())));
  }

  auto* func = Func("main", {}, ty.void_(), body,
                    ast::DecorationList{
                        Stage(params.stage),
                    });

  spirv::Builder& b = Build();

  if (var) {
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  }
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  auto preamble = b.entry_points();
  ASSERT_GE(preamble.size(), 1u);
  EXPECT_EQ(preamble[0].opcode(), spv::Op::OpEntryPoint);

  ASSERT_GE(preamble[0].operands().size(), 3u);
  EXPECT_EQ(preamble[0].operands()[0].to_i(),
            static_cast<uint32_t>(params.model));
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    Decoration_StageTest,
    testing::Values(FunctionStageData{ast::PipelineStage::kVertex,
                                      SpvExecutionModelVertex},
                    FunctionStageData{ast::PipelineStage::kFragment,
                                      SpvExecutionModelFragment},
                    FunctionStageData{ast::PipelineStage::kCompute,
                                      SpvExecutionModelGLCompute}));

TEST_F(BuilderTest, Decoration_Stage_WithUnusedInterfaceIds) {
  auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kFragment),
                    });

  auto* v_in = Global("my_in", ty.f32(), ast::StorageClass::kInput);
  auto* v_out = Global("my_out", ty.f32(), ast::StorageClass::kOutput);
  auto* v_wg = Global("my_wg", ty.f32(), ast::StorageClass::kWorkgroup);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v_in)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_out)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_wg)) << b.error();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();
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
            R"(OpEntryPoint Fragment %11 "main"
)");
}

TEST_F(BuilderTest, Decoration_Stage_WithUsedInterfaceIds) {
  auto* v_in = Global("my_in", ty.f32(), ast::StorageClass::kInput);
  auto* v_out = Global("my_out", ty.f32(), ast::StorageClass::kOutput);
  auto* v_wg = Global("my_wg", ty.f32(), ast::StorageClass::kWorkgroup);

  auto* func = Func(
      "main", {}, ty.void_(),
      ast::StatementList{Assign("my_out", "my_in"), Assign("my_wg", "my_wg"),
                         // Add duplicate usages so we show they
                         // don't get output multiple times.
                         Assign("my_out", "my_in")},
      ast::DecorationList{
          Stage(ast::PipelineStage::kFragment),
      });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v_in)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_out)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(v_wg)) << b.error();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();
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
            R"(OpEntryPoint Fragment %11 "main" %4 %1
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_Fragment_OriginUpperLeft) {
  auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kFragment),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 OriginUpperLeft
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_Default) {
  auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kCompute),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 LocalSize 1 1 1
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_Literals) {
  auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{
                        WorkgroupSize(2, 4, 6),
                        Stage(ast::PipelineStage::kCompute),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 LocalSize 2 4 6
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_Const) {
  GlobalConst("width", ty.i32(), Construct(ty.i32(), 2));
  GlobalConst("height", ty.i32(), Construct(ty.i32(), 3));
  GlobalConst("depth", ty.i32(), Construct(ty.i32(), 4));
  auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{
                        WorkgroupSize("width", "height", "depth"),
                        Stage(ast::PipelineStage::kCompute),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 LocalSize 2 3 4
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_OverridableConst) {
  GlobalConst("width", ty.i32(), Construct(ty.i32(), 2), {Override(7u)});
  GlobalConst("height", ty.i32(), Construct(ty.i32(), 3), {Override(8u)});
  GlobalConst("depth", ty.i32(), Construct(ty.i32(), 4), {Override(9u)});
  auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{
                        WorkgroupSize("width", "height", "depth"),
                        Stage(ast::PipelineStage::kCompute),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()), "");
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 3
%4 = OpSpecConstant %2 2
%5 = OpSpecConstant %2 3
%6 = OpSpecConstant %2 4
%3 = OpSpecConstantComposite %1 %4 %5 %6
)");
  EXPECT_EQ(DumpInstructions(b.annots()),
            R"(OpDecorate %4 SpecId 7
OpDecorate %5 SpecId 8
OpDecorate %6 SpecId 9
OpDecorate %3 BuiltIn WorkgroupSize
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_LiteralAndConst) {
  GlobalConst("height", ty.i32(), Construct(ty.i32(), 2), {Override(7u)});
  GlobalConst("depth", ty.i32(), Construct(ty.i32(), 3));
  auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                    ast::DecorationList{
                        WorkgroupSize(4, "height", "depth"),
                        Stage(ast::PipelineStage::kCompute),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()), "");
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeInt 32 0
%1 = OpTypeVector %2 3
%4 = OpConstant %2 4
%5 = OpSpecConstant %2 2
%6 = OpConstant %2 3
%3 = OpSpecConstantComposite %1 %4 %5 %6
)");
  EXPECT_EQ(DumpInstructions(b.annots()),
            R"(OpDecorate %5 SpecId 7
OpDecorate %3 BuiltIn WorkgroupSize
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_MultipleFragment) {
  auto* func1 = Func("main1", {}, ty.void_(), ast::StatementList{},
                     ast::DecorationList{
                         Stage(ast::PipelineStage::kFragment),
                     });

  auto* func2 = Func("main2", {}, ty.void_(), ast::StatementList{},
                     ast::DecorationList{
                         Stage(ast::PipelineStage::kFragment),
                     });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func1)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func2)) << b.error();
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

TEST_F(BuilderTest, Decoration_ExecutionMode_FragDepth) {
  Global("fragdepth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Builtin(ast::Builtin::kFragDepth),
         });

  auto* func = Func("main", ast::VariableList{}, ty.void_(),
                    ast::StatementList{
                        Assign("fragdepth", Expr(1.f)),
                    },
                    ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.error();
  EXPECT_EQ(DumpInstructions(b.execution_modes()),
            R"(OpExecutionMode %3 DepthReplacing
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
