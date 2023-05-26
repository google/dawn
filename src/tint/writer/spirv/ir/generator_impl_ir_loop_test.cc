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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Loop_BreakIf) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* loop = b.CreateLoop();

    loop->Start()->Instructions().Push(b.Continue(loop));
    loop->Continuing()->Instructions().Push(b.BreakIf(b.Constant(true), loop));
    loop->Merge()->Instructions().Push(b.Return(func));

    func->StartTarget()->Instructions().Push(loop);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%10 = OpTypeBool
%9 = OpConstantTrue %10
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
OpBranch %7
%7 = OpLabel
OpBranchConditional %9 %8 %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

// Test that we still emit the continuing block with a back-edge, even when it is unreachable.
TEST_F(SpvGeneratorImplTest, Loop_UnconditionalBreakInBody) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* loop = b.CreateLoop();

    loop->Start()->Instructions().Push(b.ExitLoop(loop));
    loop->Merge()->Instructions().Push(b.Return(func));

    func->StartTarget()->Instructions().Push(loop);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
OpBranch %8
%7 = OpLabel
OpBranch %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Loop_ConditionalBreakInBody) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* loop = b.CreateLoop();

    auto* cond_break = b.CreateIf(b.Constant(true));
    cond_break->True()->Instructions().Push(b.ExitLoop(loop));
    cond_break->False()->Instructions().Push(b.ExitIf(cond_break));
    cond_break->Merge()->Instructions().Push(b.Continue(loop));

    loop->Start()->Instructions().Push(cond_break);
    loop->Continuing()->Instructions().Push(b.NextIteration(loop));
    loop->Merge()->Instructions().Push(b.Return(func));

    func->StartTarget()->Instructions().Push(loop);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%12 = OpTypeBool
%11 = OpConstantTrue %12
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
OpSelectionMerge %9 None
OpBranchConditional %11 %10 %9
%10 = OpLabel
OpBranch %8
%9 = OpLabel
OpBranch %7
%7 = OpLabel
OpBranch %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Loop_ConditionalContinueInBody) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* loop = b.CreateLoop();

    auto* cond_break = b.CreateIf(b.Constant(true));
    cond_break->True()->Instructions().Push(b.Continue(loop));
    cond_break->False()->Instructions().Push(b.ExitIf(cond_break));
    cond_break->Merge()->Instructions().Push(b.ExitLoop(loop));

    loop->Start()->Instructions().Push(cond_break);
    loop->Continuing()->Instructions().Push(b.NextIteration(loop));
    loop->Merge()->Instructions().Push(b.Return(func));

    func->StartTarget()->Instructions().Push(loop);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%12 = OpTypeBool
%11 = OpConstantTrue %12
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
OpSelectionMerge %9 None
OpBranchConditional %11 %10 %9
%10 = OpLabel
OpBranch %7
%9 = OpLabel
OpBranch %8
%7 = OpLabel
OpBranch %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

// Test that we still emit the continuing block with a back-edge, and the merge block, even when
// they are unreachable.
TEST_F(SpvGeneratorImplTest, Loop_UnconditionalReturnInBody) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* loop = b.CreateLoop();

    loop->Start()->Instructions().Push(b.Return(func));

    func->StartTarget()->Instructions().Push(loop);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
OpReturn
%7 = OpLabel
OpBranch %5
%8 = OpLabel
OpUnreachable
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Loop_UseResultFromBodyInContinuing) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* loop = b.CreateLoop();

    auto* result = b.Equal(mod.Types().i32(), b.Constant(1_i), b.Constant(2_i));

    loop->Start()->Instructions().Push(result);
    loop->Continuing()->Instructions().Push(b.BreakIf(result, loop));
    loop->Merge()->Instructions().Push(b.Return(func));

    func->StartTarget()->Instructions().Push(loop);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%10 = OpTypeInt 32 1
%11 = OpConstant %10 1
%12 = OpConstant %10 2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
%9 = OpIEqual %10 %11 %12
%7 = OpLabel
OpBranchConditional %9 %8 %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Loop_NestedLoopInBody) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* outer_loop = b.CreateLoop();
    auto* inner_loop = b.CreateLoop();

    inner_loop->Start()->Instructions().Push(b.ExitLoop(inner_loop));
    inner_loop->Continuing()->Instructions().Push(b.NextIteration(inner_loop));
    inner_loop->Merge()->Instructions().Push(b.Continue(outer_loop));

    outer_loop->Start()->Instructions().Push(inner_loop);
    outer_loop->Continuing()->Instructions().Push(b.BreakIf(b.Constant(true), outer_loop));
    outer_loop->Merge()->Instructions().Push(b.Return(func));

    func->StartTarget()->Instructions().Push(outer_loop);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%14 = OpTypeBool
%13 = OpConstantTrue %14
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %12 %11 None
OpBranch %10
%10 = OpLabel
OpBranch %12
%11 = OpLabel
OpBranch %9
%12 = OpLabel
OpBranch %7
%7 = OpLabel
OpBranchConditional %13 %8 %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Loop_NestedLoopInContinuing) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* outer_loop = b.CreateLoop();
    auto* inner_loop = b.CreateLoop();

    inner_loop->Start()->Instructions().Push(b.Continue(inner_loop));
    inner_loop->Continuing()->Instructions().Push(b.BreakIf(b.Constant(true), inner_loop));
    inner_loop->Merge()->Instructions().Push(b.BreakIf(b.Constant(true), outer_loop));

    outer_loop->Start()->Instructions().Push(b.Continue(outer_loop));
    outer_loop->Continuing()->Instructions().Push(inner_loop);
    outer_loop->Merge()->Instructions().Push(b.Return(func));

    func->StartTarget()->Instructions().Push(outer_loop);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%14 = OpTypeBool
%13 = OpConstantTrue %14
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
OpBranch %7
%7 = OpLabel
OpBranch %9
%9 = OpLabel
OpLoopMerge %12 %11 None
OpBranch %10
%10 = OpLabel
OpBranch %11
%11 = OpLabel
OpBranchConditional %13 %12 %9
%12 = OpLabel
OpBranchConditional %13 %8 %5
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
