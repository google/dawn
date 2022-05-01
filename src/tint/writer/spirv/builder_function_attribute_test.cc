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

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Attribute_Stage) {
    auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                      ast::AttributeList{
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
using Attribute_StageTest = TestParamHelper<FunctionStageData>;
TEST_P(Attribute_StageTest, Emit) {
    auto params = GetParam();

    const ast::Variable* var = nullptr;
    const ast::Type* ret_type = nullptr;
    ast::AttributeList ret_type_attrs;
    ast::StatementList body;
    if (params.stage == ast::PipelineStage::kVertex) {
        ret_type = ty.vec4<f32>();
        ret_type_attrs.push_back(Builtin(ast::Builtin::kPosition));
        body.push_back(Return(Construct(ty.vec4<f32>())));
    } else {
        ret_type = ty.void_();
    }

    auto deco_list = ast::AttributeList{Stage(params.stage)};
    if (params.stage == ast::PipelineStage::kCompute) {
        deco_list.push_back(WorkgroupSize(1));
    }

    auto* func = Func("main", {}, ret_type, body, deco_list, ret_type_attrs);

    spirv::Builder& b = Build();

    if (var) {
        ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
    }
    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    auto preamble = b.entry_points();
    ASSERT_GE(preamble.size(), 1u);
    EXPECT_EQ(preamble[0].opcode(), spv::Op::OpEntryPoint);

    ASSERT_GE(preamble[0].operands().size(), 3u);
    EXPECT_EQ(std::get<uint32_t>(preamble[0].operands()[0]), static_cast<uint32_t>(params.model));
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    Attribute_StageTest,
    testing::Values(FunctionStageData{ast::PipelineStage::kVertex, SpvExecutionModelVertex},
                    FunctionStageData{ast::PipelineStage::kFragment, SpvExecutionModelFragment},
                    FunctionStageData{ast::PipelineStage::kCompute, SpvExecutionModelGLCompute}));

TEST_F(BuilderTest, Decoration_ExecutionMode_Fragment_OriginUpperLeft) {
    auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                      ast::AttributeList{
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
                      ast::AttributeList{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.error();
    EXPECT_EQ(DumpInstructions(b.execution_modes()),
              R"(OpExecutionMode %3 LocalSize 1 1 1
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_Literals) {
    auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                      ast::AttributeList{
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
                      ast::AttributeList{
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
    Override("width", ty.i32(), Construct(ty.i32(), 2), {Id(7u)});
    Override("height", ty.i32(), Construct(ty.i32(), 3), {Id(8u)});
    Override("depth", ty.i32(), Construct(ty.i32(), 4), {Id(9u)});
    auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                      ast::AttributeList{
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
    Override("height", ty.i32(), Construct(ty.i32(), 2), {Id(7u)});
    GlobalConst("depth", ty.i32(), Construct(ty.i32(), 3));
    auto* func = Func("main", {}, ty.void_(), ast::StatementList{},
                      ast::AttributeList{
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
                       ast::AttributeList{
                           Stage(ast::PipelineStage::kFragment),
                       });

    auto* func2 = Func("main2", {}, ty.void_(), ast::StatementList{},
                       ast::AttributeList{
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
    Func("main", ast::VariableList{}, ty.f32(),
         ast::StatementList{
             Return(Expr(1.f)),
         },
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)},
         ast::AttributeList{
             Builtin(ast::Builtin::kFragDepth),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.execution_modes()),
              R"(OpExecutionMode %11 OriginUpperLeft
OpExecutionMode %11 DepthReplacing
)");
}

}  // namespace
}  // namespace tint::writer::spirv
