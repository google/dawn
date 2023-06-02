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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* loop = b.CreateLoop();

    loop->Body()->Append(b.Continue(loop));
    loop->Continuing()->Append(b.BreakIf(b.Constant(true), loop));
    loop->Merge()->Append(b.Return(func));

    func->StartTarget()->Append(loop);

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* loop = b.CreateLoop();

    loop->Body()->Append(b.ExitLoop(loop));
    loop->Merge()->Append(b.Return(func));

    func->StartTarget()->Append(loop);

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* loop = b.CreateLoop();

    auto* cond_break = b.CreateIf(b.Constant(true));
    cond_break->True()->Append(b.ExitLoop(loop));
    cond_break->False()->Append(b.ExitIf(cond_break));
    cond_break->Merge()->Append(b.Continue(loop));

    loop->Body()->Append(cond_break);
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Merge()->Append(b.Return(func));

    func->StartTarget()->Append(loop);

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* loop = b.CreateLoop();

    auto* cond_break = b.CreateIf(b.Constant(true));
    cond_break->True()->Append(b.Continue(loop));
    cond_break->False()->Append(b.ExitIf(cond_break));
    cond_break->Merge()->Append(b.ExitLoop(loop));

    loop->Body()->Append(cond_break);
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Merge()->Append(b.Return(func));

    func->StartTarget()->Append(loop);

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* loop = b.CreateLoop();

    loop->Body()->Append(b.Return(func));

    func->StartTarget()->Append(loop);

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* loop = b.CreateLoop();

    auto* result = b.Equal(ty.i32(), b.Constant(1_i), b.Constant(2_i));

    loop->Body()->Append(result);
    loop->Continuing()->Append(b.BreakIf(result, loop));
    loop->Merge()->Append(b.Return(func));

    func->StartTarget()->Append(loop);

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* outer_loop = b.CreateLoop();
    auto* inner_loop = b.CreateLoop();

    inner_loop->Body()->Append(b.ExitLoop(inner_loop));
    inner_loop->Continuing()->Append(b.NextIteration(inner_loop));
    inner_loop->Merge()->Append(b.Continue(outer_loop));

    outer_loop->Body()->Append(inner_loop);
    outer_loop->Continuing()->Append(b.BreakIf(b.Constant(true), outer_loop));
    outer_loop->Merge()->Append(b.Return(func));

    func->StartTarget()->Append(outer_loop);

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* outer_loop = b.CreateLoop();
    auto* inner_loop = b.CreateLoop();

    inner_loop->Body()->Append(b.Continue(inner_loop));
    inner_loop->Continuing()->Append(b.BreakIf(b.Constant(true), inner_loop));
    inner_loop->Merge()->Append(b.BreakIf(b.Constant(true), outer_loop));

    outer_loop->Body()->Append(b.Continue(outer_loop));
    outer_loop->Continuing()->Append(inner_loop);
    outer_loop->Merge()->Append(b.Return(func));

    func->StartTarget()->Append(outer_loop);

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

TEST_F(SpvGeneratorImplTest, Loop_Phi_SingleValue) {
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* l = b.CreateLoop(utils::Vector{b.Constant(1_i)});
    func->StartTarget()->Append(l);

    auto* loop_param = b.BlockParam(b.ir.Types().i32());
    l->Body()->SetParams(utils::Vector{loop_param});
    auto* inc = b.Add(b.ir.Types().i32(), loop_param, b.Constant(1_i));
    l->Body()->Append(inc);
    l->Body()->Append(b.Continue(l, utils::Vector{inc}));

    auto* cont_param = b.BlockParam(b.ir.Types().i32());
    l->Continuing()->SetParams(utils::Vector{cont_param});
    auto* cmp = b.GreaterThan(b.ir.Types().bool_(), cont_param, b.Constant(5_i));
    l->Continuing()->Append(cmp);
    l->Continuing()->Append(b.BreakIf(cmp, l, utils::Vector{cont_param}));

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeInt 32 1
%11 = OpConstant %9 1
%16 = OpTypeBool
%17 = OpConstant %9 5
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
%10 = OpPhi %9 %11 %12 %13 %7
%14 = OpIAdd %9 %10 %11
OpBranch %7
%7 = OpLabel
%13 = OpPhi %9 %14 %5
%15 = OpSGreaterThan %16 %13 %17
OpBranchConditional %15 %8 %5
%8 = OpLabel
OpUnreachable
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Loop_Phi_MultipleValue) {
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* l = b.CreateLoop(utils::Vector{b.Constant(1_i), b.Constant(false)});
    func->StartTarget()->Append(l);

    auto* loop_param_a = b.BlockParam(b.ir.Types().i32());
    auto* loop_param_b = b.BlockParam(b.ir.Types().bool_());
    l->Body()->SetParams(utils::Vector{loop_param_a, loop_param_b});
    auto* inc = b.Add(b.ir.Types().i32(), loop_param_a, b.Constant(1_i));
    l->Body()->Append(inc);
    l->Body()->Append(b.Continue(l, utils::Vector{inc, loop_param_b}));

    auto* cont_param_a = b.BlockParam(b.ir.Types().i32());
    auto* cont_param_b = b.BlockParam(b.ir.Types().bool_());
    l->Continuing()->SetParams(utils::Vector{cont_param_a, cont_param_b});
    auto* cmp = b.GreaterThan(b.ir.Types().bool_(), cont_param_a, b.Constant(5_i));
    l->Continuing()->Append(cmp);
    auto* not_b = b.Not(b.ir.Types().bool_(), cont_param_b);
    l->Continuing()->Append(not_b);
    l->Continuing()->Append(b.BreakIf(cmp, l, utils::Vector{cont_param_a, not_b}));

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeInt 32 1
%11 = OpConstant %9 1
%14 = OpTypeBool
%16 = OpConstantFalse %14
%21 = OpConstant %9 5
%1 = OpFunction %2 None %3
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %6
%6 = OpLabel
%10 = OpPhi %9 %11 %12 %13 %7
%15 = OpPhi %14 %16 %12 %17 %7
%18 = OpIAdd %9 %10 %11
OpBranch %7
%7 = OpLabel
%13 = OpPhi %9 %18 %5
%19 = OpPhi %14 %15 %5
%20 = OpSGreaterThan %14 %13 %21
%17 = OpLogicalEqual %14 %19 %16
OpBranchConditional %20 %8 %5
%8 = OpLabel
OpUnreachable
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
