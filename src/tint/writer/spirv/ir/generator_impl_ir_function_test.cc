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

#include "src/tint/writer/spirv/ir/test_helper_ir.h"

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Function_Empty) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

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
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    generator_.EmitFunction(func);
    generator_.EmitFunction(func);
    generator_.EmitFunction(func);
    EXPECT_EQ(DumpTypes(), R"(%2 = OpTypeVoid
%3 = OpTypeFunction %2
)");
}

TEST_F(SpvGeneratorImplTest, Function_EntryPoint_Compute) {
    auto* func = b.CreateFunction(mod.symbols.Register("main"), mod.types.Get<type::Void>(),
                                  ir::Function::PipelineStage::kCompute, {{32, 4, 1}});
    func->StartTarget()->BranchTo(func->EndTarget());

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
    auto* func = b.CreateFunction(mod.symbols.Register("main"), mod.types.Get<type::Void>(),
                                  ir::Function::PipelineStage::kFragment);
    func->StartTarget()->BranchTo(func->EndTarget());

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
    auto* func = b.CreateFunction(mod.symbols.Register("main"), mod.types.Get<type::Void>(),
                                  ir::Function::PipelineStage::kVertex);
    func->StartTarget()->BranchTo(func->EndTarget());

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
    auto* f1 = b.CreateFunction(mod.symbols.Register("main1"), mod.types.Get<type::Void>(),
                                ir::Function::PipelineStage::kCompute, {{32, 4, 1}});
    f1->StartTarget()->BranchTo(f1->EndTarget());

    auto* f2 = b.CreateFunction(mod.symbols.Register("main2"), mod.types.Get<type::Void>(),
                                ir::Function::PipelineStage::kCompute, {{8, 2, 16}});
    f2->StartTarget()->BranchTo(f2->EndTarget());

    auto* f3 = b.CreateFunction(mod.symbols.Register("main3"), mod.types.Get<type::Void>(),
                                ir::Function::PipelineStage::kFragment);
    f3->StartTarget()->BranchTo(f3->EndTarget());

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
