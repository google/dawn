// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, Attribute_Stage) {
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kFragment),
                      });

    Builder& b = Build();

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
    StringStream str;
    str << data.stage;
    out << str.str();
    return out;
}
using Attribute_StageTest = TestParamHelper<FunctionStageData>;
TEST_P(Attribute_StageTest, Emit) {
    auto params = GetParam();

    const ast::Variable* var = nullptr;
    ast::Type ret_type;
    Vector<const ast::Attribute*, 2> ret_type_attrs;
    Vector<const ast::Statement*, 2> body;
    if (params.stage == ast::PipelineStage::kVertex) {
        ret_type = ty.vec4<f32>();
        ret_type_attrs.Push(Builtin(core::BuiltinValue::kPosition));
        body.Push(Return(Call<vec4<f32>>()));
    }

    Vector<const ast::Attribute*, 2> deco_list{Stage(params.stage)};
    if (params.stage == ast::PipelineStage::kCompute) {
        deco_list.Push(WorkgroupSize(1_i));
    }

    auto* func = Func("main", tint::Empty, ret_type, body, deco_list, ret_type_attrs);

    Builder& b = Build();

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
    SpirvASTPrinterTest,
    Attribute_StageTest,
    testing::Values(FunctionStageData{ast::PipelineStage::kVertex, SpvExecutionModelVertex},
                    FunctionStageData{ast::PipelineStage::kFragment, SpvExecutionModelFragment},
                    FunctionStageData{ast::PipelineStage::kCompute, SpvExecutionModelGLCompute}));

TEST_F(SpirvASTPrinterTest, Decoration_ExecutionMode_Fragment_OriginUpperLeft) {
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          Stage(ast::PipelineStage::kFragment),
                      });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %3 OriginUpperLeft
)");
}

TEST_F(SpirvASTPrinterTest, Decoration_ExecutionMode_WorkgroupSize_Default) {
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %3 LocalSize 1 1 1
)");
}

TEST_F(SpirvASTPrinterTest, Decoration_ExecutionMode_WorkgroupSize_Literals) {
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          WorkgroupSize(2_i, 4_i, 6_i),
                          Stage(ast::PipelineStage::kCompute),
                      });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %3 LocalSize 2 4 6
)");
}

TEST_F(SpirvASTPrinterTest, Decoration_ExecutionMode_WorkgroupSize_Const) {
    GlobalConst("width", ty.i32(), Call<i32>(2_i));
    GlobalConst("height", ty.i32(), Call<i32>(3_i));
    GlobalConst("depth", ty.i32(), Call<i32>(4_i));
    auto* func = Func("main", tint::Empty, ty.void_(), tint::Empty,
                      Vector{
                          WorkgroupSize("width", "height", "depth"),
                          Stage(ast::PipelineStage::kCompute),
                      });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateExecutionModes(func, 3)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %3 LocalSize 2 3 4
)");
}

TEST_F(SpirvASTPrinterTest, Decoration_ExecutionMode_WorkgroupSize_OverridableConst) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder pb;
            pb.Override("width", pb.ty.i32(), pb.Call<i32>(2_i), pb.Id(7_u));
            pb.Override("height", pb.ty.i32(), pb.Call<i32>(3_i), pb.Id(8_u));
            pb.Override("depth", pb.ty.i32(), pb.Call<i32>(4_i), pb.Id(9_u));
            auto* func = pb.Func("main", tint::Empty, pb.ty.void_(), tint::Empty,
                                 Vector{
                                     pb.WorkgroupSize("width", "height", "depth"),
                                     pb.Stage(ast::PipelineStage::kCompute),
                                 });
            auto program = resolver::Resolve(pb);
            Builder b(program);

            b.GenerateExecutionModes(func, 3);
        },
        "override-expressions should have been removed with the SubstituteOverride transform");
}

TEST_F(SpirvASTPrinterTest, Decoration_ExecutionMode_WorkgroupSize_LiteralAndConst) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder pb;

            pb.Override("height", pb.ty.i32(), pb.Call<i32>(2_i), pb.Id(7_u));
            pb.GlobalConst("depth", pb.ty.i32(), pb.Call<i32>(3_i));
            auto* func = pb.Func("main", tint::Empty, pb.ty.void_(), tint::Empty,
                                 Vector{
                                     pb.WorkgroupSize(4_i, "height", "depth"),
                                     pb.Stage(ast::PipelineStage::kCompute),
                                 });

            auto program = resolver::Resolve(pb);
            Builder b(program);

            b.GenerateExecutionModes(func, 3);
        },
        "override-expressions should have been removed with the SubstituteOverride transform");
}

TEST_F(SpirvASTPrinterTest, Decoration_ExecutionMode_MultipleFragment) {
    auto* func1 = Func("main1", tint::Empty, ty.void_(), tint::Empty,
                       Vector{
                           Stage(ast::PipelineStage::kFragment),
                       });

    auto* func2 = Func("main2", tint::Empty, ty.void_(), tint::Empty,
                       Vector{
                           Stage(ast::PipelineStage::kFragment),
                       });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func1)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func2)) << b.Diagnostics();
    EXPECT_EQ(DumpModule(b.Module()),
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

TEST_F(SpirvASTPrinterTest, Decoration_ExecutionMode_FragDepth) {
    Func("main", tint::Empty, ty.f32(),
         Vector{
             Return(Expr(1_f)),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Builtin(core::BuiltinValue::kFragDepth),
         });

    Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().ExecutionModes()),
              R"(OpExecutionMode %11 OriginUpperLeft
OpExecutionMode %11 DepthReplacing
)");
}

}  // namespace
}  // namespace tint::spirv::writer
