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

#include "gmock/gmock.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Attribute_Stage) {
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{
                          Stage(ast::PipelineStage::kFragment),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().EntryPoints()),
              R"(OpEntryPoint Fragment %3 "main"
)");
}

struct FunctionStageData {
    ast::PipelineStage stage;
    SpvExecutionModel model;
};
inline std::ostream& operator<<(std::ostream& out, FunctionStageData data) {
    utils::StringStream str;
    str << data.stage;
    out << str.str();
    return out;
}
using Attribute_StageTest = TestParamHelper<FunctionStageData>;
TEST_P(Attribute_StageTest, Emit) {
    auto params = GetParam();

    const ast::Variable* var = nullptr;
    ast::Type ret_type;
    utils::Vector<const ast::Attribute*, 2> ret_type_attrs;
    utils::Vector<const ast::Statement*, 2> body;
    if (params.stage == ast::PipelineStage::kVertex) {
        ret_type = ty.vec4<f32>();
        ret_type_attrs.Push(Builtin(builtin::BuiltinValue::kPosition));
        body.Push(Return(Call(ty.vec4<f32>())));
    }

    utils::Vector<const ast::Attribute*, 2> deco_list{Stage(params.stage)};
    if (params.stage == ast::PipelineStage::kCompute) {
        deco_list.Push(WorkgroupSize(1_i));
    }

    auto* func = Func("main", utils::Empty, ret_type, body, deco_list, ret_type_attrs);

    spirv::Builder& b = Build();

    if (var) {
        ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    }
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto preamble = b.Module().EntryPoints();
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
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{
                          Stage(ast::PipelineStage::kFragment),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %3 OriginUpperLeft
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_Default) {
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %3 LocalSize 1 1 1
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_Literals) {
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{
                          WorkgroupSize(2_i, 4_i, 6_i),
                          Stage(ast::PipelineStage::kCompute),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %3 LocalSize 2 4 6
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_Const) {
    GlobalConst("width", ty.i32(), Call<i32>(2_i));
    GlobalConst("height", ty.i32(), Call<i32>(3_i));
    GlobalConst("depth", ty.i32(), Call<i32>(4_i));
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{
                          WorkgroupSize("width", "height", "depth"),
                          Stage(ast::PipelineStage::kCompute),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %3 LocalSize 2 3 4
)");
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_OverridableConst) {
    Override("width", ty.i32(), Call<i32>(2_i), Id(7_u));
    Override("height", ty.i32(), Call<i32>(3_i), Id(8_u));
    Override("depth", ty.i32(), Call<i32>(4_i), Id(9_u));
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{
                          WorkgroupSize("width", "height", "depth"),
                          Stage(ast::PipelineStage::kCompute),
                      });

    spirv::Builder& b = Build();

    tint::SetInternalCompilerErrorReporter(nullptr);

    EXPECT_FALSE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_THAT(
        b.Diagnostics().str(),
        ::testing::HasSubstr(
            "override-expressions should have been removed with the SubstituteOverride transform"));
}

TEST_F(BuilderTest, Decoration_ExecutionMode_WorkgroupSize_LiteralAndConst) {
    Override("height", ty.i32(), Call<i32>(2_i), Id(7_u));
    GlobalConst("depth", ty.i32(), Call<i32>(3_i));
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{
                          WorkgroupSize(4_i, "height", "depth"),
                          Stage(ast::PipelineStage::kCompute),
                      });

    spirv::Builder& b = Build();

    tint::SetInternalCompilerErrorReporter(nullptr);

    EXPECT_FALSE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_THAT(
        b.Diagnostics().str(),
        ::testing::HasSubstr(
            "override-expressions should have been removed with the SubstituteOverride transform"));
}

TEST_F(BuilderTest, Decoration_ExecutionMode_MultipleFragment) {
    auto* func1 = Func("main1", utils::Empty, ty.void_(), utils::Empty,
                       utils::Vector{
                           Stage(ast::PipelineStage::kFragment),
                       });

    auto* func2 = Func("main2", utils::Empty, ty.void_(), utils::Empty,
                       utils::Vector{
                           Stage(ast::PipelineStage::kFragment),
                       });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func1)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func2)) << b.Diagnostics();
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
    Func("main", utils::Empty, ty.f32(),
         utils::Vector{
             Return(Expr(1_f)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Builtin(builtin::BuiltinValue::kFragDepth),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %11 OriginUpperLeft
OpExecutionMode %11 DepthReplacing
)");
}

}  // namespace
}  // namespace tint::writer::spirv
