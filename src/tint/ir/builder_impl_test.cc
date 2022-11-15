// Copyright 2022 The Tint Authors.
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

#include "src/tint/ir/test_helper.h"

#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IRBuilderImplTest = TestHelper;

TEST_F(IRBuilderImplTest, Func) {
    // func -> start -> end

    Func("f", utils::Empty, ty.void_(), utils::Empty);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    ASSERT_EQ(0u, m.entry_points.Length());
    ASSERT_EQ(1u, m.functions.Length());

    auto* f = m.functions[0];
    EXPECT_NE(f->start_target, nullptr);
    EXPECT_NE(f->end_target, nullptr);

    EXPECT_EQ(1u, f->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, f->end_target->inbound_branches.Length());

    EXPECT_EQ(f->start_target->branch_target, f->end_target);
}

TEST_F(IRBuilderImplTest, EntryPoint) {
    Func("f", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    ASSERT_EQ(1u, m.entry_points.Length());
    EXPECT_EQ(m.functions[0], m.entry_points[0]);
}

TEST_F(IRBuilderImplTest, IfStatement) {
    // func -> start -> if -> true block
    //                     -> false block
    //
    //   [true block]  -> if merge
    //   [false block] -> if merge
    //   [if merge]    -> func end
    //
    auto* ast_if = If(true, Block(), Else(Block()));
    WrapInFunction(ast_if);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    // TODO(dsinclair): check condition

    auto* flow = ir_if->As<ir::If>();
    ASSERT_NE(flow->true_target, nullptr);
    ASSERT_NE(flow->false_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_target->inbound_branches.Length());
    EXPECT_EQ(2u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->true_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->false_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, IfStatement_TrueReturns) {
    // func -> start -> if -> true block
    //                     -> false block
    //
    //   [true block]  -> func end
    //   [false block] -> if merge
    //   [if merge]    -> func end
    //
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_if);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* flow = ir_if->As<ir::If>();
    ASSERT_NE(flow->true_target, nullptr);
    ASSERT_NE(flow->false_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->true_target->branch_target, func->end_target);
    EXPECT_EQ(flow->false_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, IfStatement_FalseReturns) {
    // func -> start -> if -> true block
    //                     -> false block
    //
    //   [true block]  -> if merge
    //   [false block] -> func end
    //   [if merge]    -> func end
    //
    auto* ast_if = If(true, Block(), Else(Block(Return())));
    WrapInFunction(ast_if);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* flow = ir_if->As<ir::If>();
    ASSERT_NE(flow->true_target, nullptr);
    ASSERT_NE(flow->false_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->true_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->false_target->branch_target, func->end_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, IfStatement_BothReturn) {
    // func -> start -> if -> true block
    //                     -> false block
    //
    //   [true block]  -> func end
    //   [false block] -> func end
    //   [if merge]    -> nullptr
    //
    auto* ast_if = If(true, Block(Return()), Else(Block(Return())));
    WrapInFunction(ast_if);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* flow = ir_if->As<ir::If>();
    ASSERT_NE(flow->true_target, nullptr);
    ASSERT_NE(flow->false_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->true_target->branch_target, func->end_target);
    EXPECT_EQ(flow->false_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, IfStatement_JumpChainToMerge) {
    // if (true) {
    //   loop {
    //     break;
    //   }
    // }
    //
    // func -> start -> if true
    //               -> if false
    //
    //   [if true] -> loop
    //   [if false] -> if merge
    //   [if merge] -> func end
    //   [loop] ->  loop start
    //   [loop start] -> loop merge
    //   [loop continuing] -> loop start
    //   [loop merge] -> if merge
    //
    auto* ast_loop = Loop(Block(Break()));
    auto* ast_if = If(true, Block(ast_loop));
    WrapInFunction(ast_if);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* if_flow = ir_if->As<ir::If>();
    ASSERT_NE(if_flow->true_target, nullptr);
    ASSERT_NE(if_flow->false_target, nullptr);
    ASSERT_NE(if_flow->merge_target, nullptr);

    auto* ir_loop = b.FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start_target, nullptr);
    ASSERT_NE(loop_flow->continuing_target, nullptr);
    ASSERT_NE(loop_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(func->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, loop_flow);
    EXPECT_EQ(loop_flow->start_target->branch_target, loop_flow->merge_target);
    EXPECT_EQ(loop_flow->merge_target->branch_target, if_flow->merge_target);
    EXPECT_EQ(loop_flow->continuing_target->branch_target, loop_flow->start_target);
    EXPECT_EQ(if_flow->false_target->branch_target, if_flow->merge_target);
    EXPECT_EQ(if_flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Loop_WithBreak) {
    // func -> start -> loop -> loop start -> loop merge -> func end
    //
    //   [continuing] -> loop start
    //
    auto* ast_loop = Loop(Block(Break()));
    WrapInFunction(ast_loop);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_loop = b.FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(flow->start_target, nullptr);
    ASSERT_NE(flow->continuing_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start_target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->start_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->continuing_target->branch_target, flow->start_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Loop_WithContinue) {
    // func -> start -> loop -> loop start -> if -> true block
    //                                           -> false block
    //
    //   [if true]  -> loop merge
    //   [if false] -> if merge
    //   [if merge] -> loop continuing
    //   [loop continuing] -> loop start
    //   [loop merge] -> func end
    //
    auto* ast_if = If(true, Block(Break()));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_loop = b.FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start_target, nullptr);
    ASSERT_NE(loop_flow->continuing_target, nullptr);
    ASSERT_NE(loop_flow->merge_target, nullptr);

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    ASSERT_TRUE(ir_if->Is<ir::If>());

    auto* if_flow = ir_if->As<ir::If>();
    ASSERT_NE(if_flow->true_target, nullptr);
    ASSERT_NE(if_flow->false_target, nullptr);
    ASSERT_NE(if_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, loop_flow);
    EXPECT_EQ(loop_flow->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, loop_flow->merge_target);
    EXPECT_EQ(if_flow->false_target->branch_target, if_flow->merge_target);
    EXPECT_EQ(loop_flow->continuing_target->branch_target, loop_flow->start_target);
    EXPECT_EQ(loop_flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Loop_WithContinuing_BreakIf) {
    // func -> start -> loop -> loop start -> continuing
    //
    //   [loop continuing] -> if -> true branch
    //                           -> false branch
    //   [if true] -> loop merge
    //   [if false] -> if merge
    //   [if merge] -> loop start
    //   [loop merge] -> func end
    //
    auto* ast_break_if = BreakIf(true);
    auto* ast_loop = Loop(Block(), Block(ast_break_if));
    WrapInFunction(ast_loop);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_loop = b.FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start_target, nullptr);
    ASSERT_NE(loop_flow->continuing_target, nullptr);
    ASSERT_NE(loop_flow->merge_target, nullptr);

    auto* ir_break_if = b.FlowNodeForAstNode(ast_break_if);
    ASSERT_NE(ir_break_if, nullptr);
    ASSERT_TRUE(ir_break_if->Is<ir::If>());

    auto* break_if_flow = ir_break_if->As<ir::If>();
    ASSERT_NE(break_if_flow->true_target, nullptr);
    ASSERT_NE(break_if_flow->false_target, nullptr);
    ASSERT_NE(break_if_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, break_if_flow->inbound_branches.Length());
    EXPECT_EQ(1u, break_if_flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, break_if_flow->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, break_if_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, loop_flow);
    EXPECT_EQ(loop_flow->start_target->branch_target, loop_flow->continuing_target);
    EXPECT_EQ(loop_flow->continuing_target->branch_target, break_if_flow);
    EXPECT_EQ(break_if_flow->true_target->branch_target, loop_flow->merge_target);
    EXPECT_EQ(break_if_flow->false_target->branch_target, break_if_flow->merge_target);
    EXPECT_EQ(break_if_flow->merge_target->branch_target, loop_flow->start_target);
    EXPECT_EQ(loop_flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Loop_WithReturn) {
    // func -> start -> loop -> loop start -> if -> true block
    //                                           -> false block
    //
    //   [if true]  -> func end
    //   [if false] -> if merge
    //   [if merge] -> loop continuing
    //   [loop continuing] -> loop start
    //   [loop merge] -> nullptr
    //
    auto* ast_if = If(true, Block(Return()));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_loop = b.FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start_target, nullptr);
    ASSERT_NE(loop_flow->continuing_target, nullptr);
    ASSERT_NE(loop_flow->merge_target, nullptr);

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    ASSERT_TRUE(ir_if->Is<ir::If>());

    auto* if_flow = ir_if->As<ir::If>();
    ASSERT_NE(if_flow->true_target, nullptr);
    ASSERT_NE(if_flow->false_target, nullptr);
    ASSERT_NE(if_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(loop_flow->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, func->end_target);
    EXPECT_EQ(if_flow->false_target->branch_target, if_flow->merge_target);
    EXPECT_EQ(loop_flow->continuing_target->branch_target, loop_flow->start_target);

    EXPECT_EQ(func->start_target->branch_target, ir_loop);
    EXPECT_EQ(loop_flow->merge_target->branch_target, nullptr);
}

TEST_F(IRBuilderImplTest, Loop_WithOnlyReturn) {
    // {
    //   loop {
    //     return;
    //     continue;
    //   }
    //   if true { return; }
    // }
    //
    // func -> start -> loop -> loop start -> return -> func end
    //
    //   [loop continuing] -> loop start
    //   [loop merge] -> nullptr
    //
    // Note, the continue; is here is a dead call, so we won't emit a branch to the continuing block
    // so the inbound_branches will be zero for continuing.
    //
    // The `if` after the `loop` is also eliminated as there is no control-flow path reaching the
    // block.
    auto* ast_loop = Loop(Block(Return(), Continue()));
    WrapInFunction(ast_loop, If(true, Block(Return())));
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_loop = b.FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start_target, nullptr);
    ASSERT_NE(loop_flow->continuing_target, nullptr);
    ASSERT_NE(loop_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start_target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(loop_flow->start_target->branch_target, func->end_target);
    EXPECT_EQ(loop_flow->continuing_target->branch_target, loop_flow->start_target);

    EXPECT_EQ(func->start_target->branch_target, ir_loop);
}

TEST_F(IRBuilderImplTest, Loop_WithOnlyReturn_ContinuingBreakIf) {
    // {
    //   loop {
    //     return;
    //     continuing {
    //       break if true;
    //     }
    //   }
    //   if (true) { return; }
    // }
    //
    // func -> start -> loop -> loop start -> return -> func end
    //
    //   [loop continuing] -> break if true
    //                     -> break if false
    //   [break if true] -> loop merge
    //   [break if false] -> if merge
    //   [break if merge] -> loop start
    //   [loop merge] -> nullptr
    //
    // In this case, the continuing block is dead code, but we don't really know that when parsing
    // so we end up with a branch into the loop merge target. The loop merge can tell it's dead code
    // so we can drop the if ater the loop.
    auto* ast_break_if = BreakIf(true);
    auto* ast_loop = Loop(Block(Return()), Block(ast_break_if));
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(Block(ast_loop, ast_if));
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_loop = b.FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start_target, nullptr);
    ASSERT_NE(loop_flow->continuing_target, nullptr);
    ASSERT_NE(loop_flow->merge_target, nullptr);

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    EXPECT_EQ(ir_if, nullptr);

    auto* ir_break_if = b.FlowNodeForAstNode(ast_break_if);
    ASSERT_NE(ir_break_if, nullptr);
    EXPECT_TRUE(ir_break_if->Is<ir::If>());

    auto* break_if_flow = ir_break_if->As<ir::If>();
    ASSERT_NE(break_if_flow->true_target, nullptr);
    ASSERT_NE(break_if_flow->false_target, nullptr);
    ASSERT_NE(break_if_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start_target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    // This is 1 because only the loop branch happens. The subsequent if return is dead code.
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(loop_flow->start_target->branch_target, func->end_target);
    EXPECT_EQ(loop_flow->continuing_target->branch_target, break_if_flow);

    EXPECT_EQ(func->start_target->branch_target, ir_loop);
}

TEST_F(IRBuilderImplTest, Loop_WithIf_BothBranchesBreak) {
    // func -> start -> loop -> loop start -> if -> true branch
    //                                           -> false branch
    //
    //   [if true] -> loop merge
    //   [if false] -> loop merge
    //   [if merge] -> nullptr
    //   [loop continuing] -> loop start
    //   [loop merge] -> func end
    //
    auto* ast_if = If(true, Block(Break()), Else(Block(Break())));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_loop = b.FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start_target, nullptr);
    ASSERT_NE(loop_flow->continuing_target, nullptr);
    ASSERT_NE(loop_flow->merge_target, nullptr);

    auto* ir_if = b.FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    ASSERT_TRUE(ir_if->Is<ir::If>());

    auto* if_flow = ir_if->As<ir::If>();
    ASSERT_NE(if_flow->true_target, nullptr);
    ASSERT_NE(if_flow->false_target, nullptr);
    ASSERT_NE(if_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start_target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_target->inbound_branches.Length());
    EXPECT_EQ(0u, if_flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    // Note, the `continue` is dead code because both if branches go out of loop, so it just gets
    // dropped.

    EXPECT_EQ(func->start_target->branch_target, loop_flow);
    EXPECT_EQ(loop_flow->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, loop_flow->merge_target);
    EXPECT_EQ(if_flow->false_target->branch_target, loop_flow->merge_target);
    EXPECT_EQ(loop_flow->continuing_target->branch_target, loop_flow->start_target);
    EXPECT_EQ(loop_flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Loop_Nested) {
    // loop {   // loop_a
    //   loop {  // loop_b
    //      if (true) { break; }  // if_a
    //      if (true) { continue; }  // if_b
    //      continuing {
    //        loop {  // loop_c
    //          break;
    //        }
    //
    //        loop {  // loop_d
    //          continuing {
    //            break if (true);  // if_c
    //          }
    //        }
    //      }
    //    }
    //    if (true) { break; }  // if_d
    //  }
    //
    // func -> start -> loop_a -> loop_a start
    //
    //   [loop_a start] -> loop_b
    //   [loop_b start] -> if_a
    //   [if_a true]  -> loop_b merge
    //   [if_a false] -> if_a merge
    //   [if_a merge] -> if_b
    //   [if_b true] -> loop_b continuing
    //   [if_b false] -> if_b merge
    //   [if_b merge] -> loop_b continug
    //   [loop_b continuing] -> loop_c
    //   [loop_c start] -> loop_c merge
    //   [loop_c continuing] -> loop_c start
    //   [loop_c merge] -> loop_d
    //   [loop_d start] -> loop_d continuing
    //   [loop_d continuing] -> if_c
    //   [if_c true]  -> loop_d merge
    //   [if_c false] -> if_c merge
    //   [if c merge] -> loop_d start
    //   [loop_d merge] -> loop_b start
    //   [loop_b merge] -> if_d
    //   [if_d true]  -> loop_a merge
    //   [if_d false] -> if_d merge
    //   [if_d merge] -> loop_a continuing
    //   [loop_a continuing] -> loop_a start
    //   [loop_a merge] -> func end
    //

    auto* ast_if_a = If(true, Block(Break()));
    auto* ast_if_b = If(true, Block(Continue()));
    auto* ast_if_c = BreakIf(true);
    auto* ast_if_d = If(true, Block(Break()));

    auto* ast_loop_d = Loop(Block(), Block(ast_if_c));
    auto* ast_loop_c = Loop(Block(Break()));

    auto* ast_loop_b = Loop(Block(ast_if_a, ast_if_b), Block(ast_loop_c, ast_loop_d));
    auto* ast_loop_a = Loop(Block(ast_loop_b, ast_if_d));

    WrapInFunction(ast_loop_a);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_loop_a = b.FlowNodeForAstNode(ast_loop_a);
    ASSERT_NE(ir_loop_a, nullptr);
    EXPECT_TRUE(ir_loop_a->Is<ir::Loop>());
    auto* loop_flow_a = ir_loop_a->As<ir::Loop>();
    ASSERT_NE(loop_flow_a->start_target, nullptr);
    ASSERT_NE(loop_flow_a->continuing_target, nullptr);
    ASSERT_NE(loop_flow_a->merge_target, nullptr);

    auto* ir_loop_b = b.FlowNodeForAstNode(ast_loop_b);
    ASSERT_NE(ir_loop_b, nullptr);
    EXPECT_TRUE(ir_loop_b->Is<ir::Loop>());
    auto* loop_flow_b = ir_loop_b->As<ir::Loop>();
    ASSERT_NE(loop_flow_b->start_target, nullptr);
    ASSERT_NE(loop_flow_b->continuing_target, nullptr);
    ASSERT_NE(loop_flow_b->merge_target, nullptr);

    auto* ir_loop_c = b.FlowNodeForAstNode(ast_loop_c);
    ASSERT_NE(ir_loop_c, nullptr);
    EXPECT_TRUE(ir_loop_c->Is<ir::Loop>());
    auto* loop_flow_c = ir_loop_c->As<ir::Loop>();
    ASSERT_NE(loop_flow_c->start_target, nullptr);
    ASSERT_NE(loop_flow_c->continuing_target, nullptr);
    ASSERT_NE(loop_flow_c->merge_target, nullptr);

    auto* ir_loop_d = b.FlowNodeForAstNode(ast_loop_d);
    ASSERT_NE(ir_loop_d, nullptr);
    EXPECT_TRUE(ir_loop_d->Is<ir::Loop>());
    auto* loop_flow_d = ir_loop_d->As<ir::Loop>();
    ASSERT_NE(loop_flow_d->start_target, nullptr);
    ASSERT_NE(loop_flow_d->continuing_target, nullptr);
    ASSERT_NE(loop_flow_d->merge_target, nullptr);

    auto* ir_if_a = b.FlowNodeForAstNode(ast_if_a);
    ASSERT_NE(ir_if_a, nullptr);
    EXPECT_TRUE(ir_if_a->Is<ir::If>());
    auto* if_flow_a = ir_if_a->As<ir::If>();
    ASSERT_NE(if_flow_a->true_target, nullptr);
    ASSERT_NE(if_flow_a->false_target, nullptr);
    ASSERT_NE(if_flow_a->merge_target, nullptr);

    auto* ir_if_b = b.FlowNodeForAstNode(ast_if_b);
    ASSERT_NE(ir_if_b, nullptr);
    EXPECT_TRUE(ir_if_b->Is<ir::If>());
    auto* if_flow_b = ir_if_b->As<ir::If>();
    ASSERT_NE(if_flow_b->true_target, nullptr);
    ASSERT_NE(if_flow_b->false_target, nullptr);
    ASSERT_NE(if_flow_b->merge_target, nullptr);

    auto* ir_if_c = b.FlowNodeForAstNode(ast_if_c);
    ASSERT_NE(ir_if_c, nullptr);
    EXPECT_TRUE(ir_if_c->Is<ir::If>());
    auto* if_flow_c = ir_if_c->As<ir::If>();
    ASSERT_NE(if_flow_c->true_target, nullptr);
    ASSERT_NE(if_flow_c->false_target, nullptr);
    ASSERT_NE(if_flow_c->merge_target, nullptr);

    auto* ir_if_d = b.FlowNodeForAstNode(ast_if_d);
    ASSERT_NE(ir_if_d, nullptr);
    EXPECT_TRUE(ir_if_d->Is<ir::If>());
    auto* if_flow_d = ir_if_d->As<ir::If>();
    ASSERT_NE(if_flow_d->true_target, nullptr);
    ASSERT_NE(if_flow_d->false_target, nullptr);
    ASSERT_NE(if_flow_d->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow_a->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_a->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_a->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_a->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_b->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_b->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_b->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_b->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_c->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_c->start_target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow_c->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_c->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_d->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_d->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_d->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_d->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_a->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_a->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_a->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_a->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_b->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_b->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_b->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_b->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_c->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_c->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_c->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_c->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_d->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_d->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_d->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_d->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, loop_flow_a);
    EXPECT_EQ(loop_flow_a->start_target->branch_target, loop_flow_b);
    EXPECT_EQ(loop_flow_b->start_target->branch_target, if_flow_a);
    EXPECT_EQ(if_flow_a->true_target->branch_target, loop_flow_b->merge_target);
    EXPECT_EQ(if_flow_a->false_target->branch_target, if_flow_a->merge_target);
    EXPECT_EQ(if_flow_a->merge_target->branch_target, if_flow_b);
    EXPECT_EQ(if_flow_b->true_target->branch_target, loop_flow_b->continuing_target);
    EXPECT_EQ(if_flow_b->false_target->branch_target, if_flow_b->merge_target);
    EXPECT_EQ(if_flow_b->merge_target->branch_target, loop_flow_b->continuing_target);
    EXPECT_EQ(loop_flow_b->continuing_target->branch_target, loop_flow_c);
    EXPECT_EQ(loop_flow_c->start_target->branch_target, loop_flow_c->merge_target);
    EXPECT_EQ(loop_flow_c->continuing_target->branch_target, loop_flow_c->start_target);
    EXPECT_EQ(loop_flow_c->merge_target->branch_target, loop_flow_d);
    EXPECT_EQ(loop_flow_d->start_target->branch_target, loop_flow_d->continuing_target);
    EXPECT_EQ(loop_flow_d->continuing_target->branch_target, if_flow_c);
    EXPECT_EQ(if_flow_c->true_target->branch_target, loop_flow_d->merge_target);
    EXPECT_EQ(if_flow_c->false_target->branch_target, if_flow_c->merge_target);
    EXPECT_EQ(if_flow_c->merge_target->branch_target, loop_flow_d->start_target);
    EXPECT_EQ(loop_flow_d->merge_target->branch_target, loop_flow_b->start_target);
    EXPECT_EQ(loop_flow_b->merge_target->branch_target, if_flow_d);
    EXPECT_EQ(if_flow_d->true_target->branch_target, loop_flow_a->merge_target);
    EXPECT_EQ(if_flow_d->false_target->branch_target, if_flow_d->merge_target);
    EXPECT_EQ(if_flow_d->merge_target->branch_target, loop_flow_a->continuing_target);
    EXPECT_EQ(loop_flow_a->continuing_target->branch_target, loop_flow_a->start_target);
    EXPECT_EQ(loop_flow_a->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, While) {
    // {
    //   while false {
    //   }
    // }
    //
    // func -> while -> loop_start -> if true
    //                             -> if false
    //
    //   [if true] -> if merge
    //   [if false] -> while merge
    //   [if merge] -> loop continuing
    //   [loop continuing] -> loop start
    //   [while merge] -> func end
    //
    auto* ast_while = While(false, Block());
    WrapInFunction(ast_while);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_while = b.FlowNodeForAstNode(ast_while);
    ASSERT_NE(ir_while, nullptr);
    ASSERT_TRUE(ir_while->Is<ir::Loop>());

    auto* flow = ir_while->As<ir::Loop>();
    ASSERT_NE(flow->start_target, nullptr);
    ASSERT_NE(flow->continuing_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_NE(flow->start_target->branch_target, nullptr);
    ASSERT_TRUE(flow->start_target->branch_target->Is<ir::If>());
    auto* if_flow = flow->start_target->branch_target->As<ir::If>();
    ASSERT_NE(if_flow->true_target, nullptr);
    ASSERT_NE(if_flow->false_target, nullptr);
    ASSERT_NE(if_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, if_flow->merge_target);
    EXPECT_EQ(if_flow->false_target->branch_target, flow->merge_target);
    EXPECT_EQ(if_flow->merge_target->branch_target, flow->continuing_target);
    EXPECT_EQ(flow->continuing_target->branch_target, flow->start_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, While_Return) {
    // {
    //   while true {
    //     return;
    //   }
    // }
    //
    // func -> while -> if true
    //                  if false
    //
    //   [if true] -> if merge
    //   [if false] -> while merge
    //   [if merge] -> func end
    //   [while merge] -> func end
    //
    auto* ast_while = While(true, Block(Return()));
    WrapInFunction(ast_while);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_while = b.FlowNodeForAstNode(ast_while);
    ASSERT_NE(ir_while, nullptr);
    ASSERT_TRUE(ir_while->Is<ir::Loop>());

    auto* flow = ir_while->As<ir::Loop>();
    ASSERT_NE(flow->start_target, nullptr);
    ASSERT_NE(flow->continuing_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_NE(flow->start_target->branch_target, nullptr);
    ASSERT_TRUE(flow->start_target->branch_target->Is<ir::If>());
    auto* if_flow = flow->start_target->branch_target->As<ir::If>();
    ASSERT_NE(if_flow->true_target, nullptr);
    ASSERT_NE(if_flow->false_target, nullptr);
    ASSERT_NE(if_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start_target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, if_flow->merge_target);
    EXPECT_EQ(if_flow->false_target->branch_target, flow->merge_target);
    EXPECT_EQ(if_flow->merge_target->branch_target, func->end_target);
    EXPECT_EQ(flow->continuing_target->branch_target, flow->start_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

// TODO(dsinclair): Enable when variable declarations and increment are supported
TEST_F(IRBuilderImplTest, DISABLED_For) {
    // for(var i: 0; i < 10; i++) {
    // }
    //
    // func -> loop -> loop start -> if true
    //                            -> if false
    //
    //   [if true] -> if merge
    //   [if false] -> loop merge
    //   [if merge] -> loop continuing
    //   [loop continuing] -> loop start
    //   [loop merge] -> func end
    //
    auto* ast_for = For(Decl(Var("i", ty.i32())), LessThan("i", 10_a), Increment("i"), Block());
    WrapInFunction(ast_for);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_for = b.FlowNodeForAstNode(ast_for);
    ASSERT_NE(ir_for, nullptr);
    ASSERT_TRUE(ir_for->Is<ir::Loop>());

    auto* flow = ir_for->As<ir::Loop>();
    ASSERT_NE(flow->start_target, nullptr);
    ASSERT_NE(flow->continuing_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_NE(flow->start_target->branch_target, nullptr);
    ASSERT_TRUE(flow->start_target->branch_target->Is<ir::If>());
    auto* if_flow = flow->start_target->branch_target->As<ir::If>();
    ASSERT_NE(if_flow->true_target, nullptr);
    ASSERT_NE(if_flow->false_target, nullptr);
    ASSERT_NE(if_flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, if_flow->merge_target);
    EXPECT_EQ(if_flow->false_target->branch_target, flow->merge_target);
    EXPECT_EQ(if_flow->merge_target->branch_target, flow->continuing_target);
    EXPECT_EQ(flow->continuing_target->branch_target, flow->start_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, For_NoInitCondOrContinuing) {
    // for (;;) {
    //   break;
    // }
    //
    // func -> loop -> loop start -> loop merge -> func end
    //
    auto* ast_for = For(nullptr, nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_for = b.FlowNodeForAstNode(ast_for);
    ASSERT_NE(ir_for, nullptr);
    ASSERT_TRUE(ir_for->Is<ir::Loop>());

    auto* flow = ir_for->As<ir::Loop>();
    ASSERT_NE(flow->start_target, nullptr);
    ASSERT_NE(flow->continuing_target, nullptr);
    ASSERT_NE(flow->merge_target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start_target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(flow->start_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->continuing_target->branch_target, flow->start_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Switch) {
    // func -> switch -> case 1
    //                -> case 2
    //                -> default
    //
    //   [case 1] -> switch merge
    //   [case 2] -> switch merge
    //   [default] -> switch merge
    //   [switch merge] -> func end
    //
    auto* ast_switch = Switch(
        1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)}, Block()),
                           Case(utils::Vector{CaseSelector(1_i)}, Block()), DefaultCase(Block())});

    WrapInFunction(ast_switch);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_switch = b.FlowNodeForAstNode(ast_switch);
    ASSERT_NE(ir_switch, nullptr);
    ASSERT_TRUE(ir_switch->Is<ir::Switch>());

    auto* flow = ir_switch->As<ir::Switch>();
    ASSERT_NE(flow->merge_target, nullptr);
    ASSERT_EQ(3u, flow->cases.Length());

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    ASSERT_TRUE(flow->cases[0].selectors[0]->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(0_i, flow->cases[0].selectors[0]->expr->As<ast::IntLiteralExpression>()->value);

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    ASSERT_TRUE(flow->cases[1].selectors[0]->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(1_i, flow->cases[1].selectors[0]->expr->As<ast::IntLiteralExpression>()->value);

    ASSERT_EQ(1u, flow->cases[2].selectors.Length());
    EXPECT_TRUE(flow->cases[2].selectors[0]->IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[2].start_target->inbound_branches.Length());
    EXPECT_EQ(3u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, ir_switch);
    EXPECT_EQ(flow->cases[0].start_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->cases[1].start_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->cases[2].start_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Switch_OnlyDefault) {
    // func -> switch -> default -> switch merge -> func end
    //
    auto* ast_switch = Switch(1_i, utils::Vector{DefaultCase(Block())});

    WrapInFunction(ast_switch);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_switch = b.FlowNodeForAstNode(ast_switch);
    ASSERT_NE(ir_switch, nullptr);
    ASSERT_TRUE(ir_switch->Is<ir::Switch>());

    auto* flow = ir_switch->As<ir::Switch>();
    ASSERT_NE(flow->merge_target, nullptr);
    ASSERT_EQ(1u, flow->cases.Length());

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    EXPECT_TRUE(flow->cases[0].selectors[0]->IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, ir_switch);
    EXPECT_EQ(flow->cases[0].start_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Switch_WithBreak) {
    // {
    //   switch(1) {
    //     case 0: {
    //       break;
    //       if true { return;}   // Dead code
    //     }
    //     default: {}
    //   }
    // }
    //
    // func -> switch -> case 1
    //                -> default
    //
    //   [case 1] -> switch merge
    //   [default] -> switch merge
    //   [switch merge] -> func end
    auto* ast_switch = Switch(1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)},
                                                      Block(Break(), If(true, Block(Return())))),
                                                 DefaultCase(Block())});

    WrapInFunction(ast_switch);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    auto* ir_switch = b.FlowNodeForAstNode(ast_switch);
    ASSERT_NE(ir_switch, nullptr);
    ASSERT_TRUE(ir_switch->Is<ir::Switch>());

    auto* flow = ir_switch->As<ir::Switch>();
    ASSERT_NE(flow->merge_target, nullptr);
    ASSERT_EQ(2u, flow->cases.Length());

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    ASSERT_TRUE(flow->cases[0].selectors[0]->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(0_i, flow->cases[0].selectors[0]->expr->As<ast::IntLiteralExpression>()->value);

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    EXPECT_TRUE(flow->cases[1].selectors[0]->IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start_target->inbound_branches.Length());
    EXPECT_EQ(2u, flow->merge_target->inbound_branches.Length());
    // This is 1 because the if is dead-code eliminated and the return doesn't happen.
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, ir_switch);
    EXPECT_EQ(flow->cases[0].start_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->cases[1].start_target->branch_target, flow->merge_target);
    EXPECT_EQ(flow->merge_target->branch_target, func->end_target);
}

TEST_F(IRBuilderImplTest, Switch_AllReturn) {
    // {
    //   switch(1) {
    //     case 0: {
    //       return;
    //     }
    //     default: {
    //       return;
    //     }
    //   }
    //   if true { return; }  // Dead code
    // }
    //
    // func -> switch -> case 1
    //                -> default
    //
    //   [case 1] -> func end
    //   [default] -> func end
    //   [switch merge] -> nullptr
    //
    auto* ast_switch =
        Switch(1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)}, Block(Return())),
                                  DefaultCase(Block(Return()))});

    auto* ast_if = If(true, Block(Return()));

    WrapInFunction(ast_switch, ast_if);
    auto& b = Build();

    auto r = b.Build();
    ASSERT_TRUE(r) << b.error();
    auto m = r.Move();

    ASSERT_EQ(b.FlowNodeForAstNode(ast_if), nullptr);

    auto* ir_switch = b.FlowNodeForAstNode(ast_switch);
    ASSERT_NE(ir_switch, nullptr);
    ASSERT_TRUE(ir_switch->Is<ir::Switch>());

    auto* flow = ir_switch->As<ir::Switch>();
    ASSERT_NE(flow->merge_target, nullptr);
    ASSERT_EQ(2u, flow->cases.Length());

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    ASSERT_TRUE(flow->cases[0].selectors[0]->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(0_i, flow->cases[0].selectors[0]->expr->As<ast::IntLiteralExpression>()->value);

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    EXPECT_TRUE(flow->cases[1].selectors[0]->IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start_target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->merge_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch_target, ir_switch);
    EXPECT_EQ(flow->cases[0].start_target->branch_target, func->end_target);
    EXPECT_EQ(flow->cases[1].start_target->branch_target, func->end_target);
    EXPECT_EQ(flow->merge_target->branch_target, nullptr);
}

}  // namespace
}  // namespace tint::ir
