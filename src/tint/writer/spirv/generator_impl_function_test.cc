// Copyright 2023 The Tint Authors.
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

#include "src/tint/writer/spirv/test_helper_ir.h"

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Function_Empty) {
    auto* func = CreateFunction();
    func->name = ir.symbols.Register("foo");
    func->return_type = ir.types.Get<type::Void>();
    func->start_target->branch.target = func->end_target;

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

// Test that we do not emit the same function type more than once.
TEST_F(SpvGeneratorImplTest, Function_DeduplicateType) {
    auto* func = CreateFunction();
    func->return_type = ir.types.Get<type::Void>();
    func->start_target->branch.target = func->end_target;

    generator_.EmitFunction(func);
    generator_.EmitFunction(func);
    generator_.EmitFunction(func);
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeVoid
%3 = OpTypeFunction %2
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Compute) {
    auto* func = CreateFunction();
    func->name = ir.symbols.Register("main");
    func->return_type = ir.types.Get<type::Void>();
    func->pipeline_stage = ir::Function::PipelineStage::kCompute;
    func->workgroup_size = {32, 4, 1};
    func->start_target->branch.target = func->end_target;

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpEntryPoint GLCompute %1 "main"
OpExecutionMode %1 LocalSize 32 4 1
OpName %1 "main"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Fragment) {
    auto* func = CreateFunction();
    func->name = ir.symbols.Register("main");
    func->return_type = ir.types.Get<type::Void>();
    func->pipeline_stage = ir::Function::PipelineStage::kFragment;
    func->start_target->branch.target = func->end_target;

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpEntryPoint Fragment %1 "main"
OpExecutionMode %1 OriginUpperLeft
OpName %1 "main"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Vertex) {
    auto* func = CreateFunction();
    func->name = ir.symbols.Register("main");
    func->return_type = ir.types.Get<type::Void>();
    func->pipeline_stage = ir::Function::PipelineStage::kVertex;
    func->start_target->branch.target = func->end_target;

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpEntryPoint Vertex %1 "main"
OpName %1 "main"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Multiple) {
    auto* f1 = CreateFunction();
    f1->name = ir.symbols.Register("main1");
    f1->return_type = ir.types.Get<type::Void>();
    f1->pipeline_stage = ir::Function::PipelineStage::kCompute;
    f1->workgroup_size = {32, 4, 1};
    f1->start_target->branch.target = f1->end_target;

    auto* f2 = CreateFunction();
    f2->name = ir.symbols.Register("main2");
    f2->return_type = ir.types.Get<type::Void>();
    f2->pipeline_stage = ir::Function::PipelineStage::kCompute;
    f2->workgroup_size = {8, 2, 16};
    f2->start_target->branch.target = f2->end_target;

    auto* f3 = CreateFunction();
    f3->name = ir.symbols.Register("main3");
    f3->return_type = ir.types.Get<type::Void>();
    f3->pipeline_stage = ir::Function::PipelineStage::kFragment;
    f3->start_target->branch.target = f3->end_target;

    generator_.EmitFunction(f1);
    generator_.EmitFunction(f2);
    generator_.EmitFunction(f3);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpEntryPoint GLCompute %1 "main1"
OpEntryPoint GLCompute %5 "main2"
OpEntryPoint Fragment %7 "main3"
OpExecutionMode %1 LocalSize 32 4 1
OpExecutionMode %5 LocalSize 8 2 16
OpExecutionMode %7 OriginUpperLeft
OpName %1 "main1"
OpName %5 "main2"
OpName %7 "main3"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
%5 = OpFunction %2 None %3
%6 = OpLabel
OpReturn
OpFunctionEnd
%7 = OpFunction %2 None %3
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
