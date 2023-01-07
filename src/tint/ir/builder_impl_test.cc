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
#include "src/tint/constant/scalar.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_BuilderImplTest = TestHelper;

TEST_F(IR_BuilderImplTest, Func) {
    // func -> start -> end

    Func("f", utils::Empty, ty.void_(), utils::Empty);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    ASSERT_EQ(0u, m.entry_points.Length());
    ASSERT_EQ(1u, m.functions.Length());

    auto* f = m.functions[0];
    EXPECT_NE(f->start_target, nullptr);
    EXPECT_NE(f->end_target, nullptr);

    EXPECT_EQ(1u, f->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, f->end_target->inbound_branches.Length());

    EXPECT_EQ(f->start_target->branch.target, f->end_target);
}

TEST_F(IR_BuilderImplTest, EntryPoint) {
    Func("f", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    ASSERT_EQ(1u, m.entry_points.Length());
    EXPECT_EQ(m.functions[0], m.entry_points[0]);
}

TEST_F(IR_BuilderImplTest, IfStatement) {
    // func -> start -> if -> true block
    //                     -> false block
    //
    //   [true block]  -> if merge
    //   [false block] -> if merge
    //   [if merge]    -> func end
    //
    auto* ast_if = If(true, Block(), Else(Block()));
    WrapInFunction(ast_if);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_if = FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* flow = ir_if->As<ir::If>();
    ASSERT_NE(flow->true_.target, nullptr);
    ASSERT_NE(flow->false_.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(2u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, flow);
    EXPECT_EQ(flow->true_.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->false_.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);

    // Check condition
    ASSERT_TRUE(flow->condition->Is<Constant>());
    auto* instr = flow->condition->As<Constant>()->value;
    ASSERT_TRUE(instr->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(instr->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, IfStatement_TrueReturns) {
    // func -> start -> if -> true block
    //                     -> false block
    //
    //   [true block]  -> func end
    //   [false block] -> if merge
    //   [if merge]    -> func end
    //
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_if);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_if = FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* flow = ir_if->As<ir::If>();
    ASSERT_NE(flow->true_.target, nullptr);
    ASSERT_NE(flow->false_.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, flow);
    EXPECT_EQ(flow->true_.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(flow->false_.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, IfStatement_FalseReturns) {
    // func -> start -> if -> true block
    //                     -> false block
    //
    //   [true block]  -> if merge
    //   [false block] -> func end
    //   [if merge]    -> func end
    //
    auto* ast_if = If(true, Block(), Else(Block(Return())));
    WrapInFunction(ast_if);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_if = FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* flow = ir_if->As<ir::If>();
    ASSERT_NE(flow->true_.target, nullptr);
    ASSERT_NE(flow->false_.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, flow);
    EXPECT_EQ(flow->true_.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->false_.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, IfStatement_BothReturn) {
    // func -> start -> if -> true block
    //                     -> false block
    //
    //   [true block]  -> func end
    //   [false block] -> func end
    //   [if merge]    -> nullptr
    //
    auto* ast_if = If(true, Block(Return()), Else(Block(Return())));
    WrapInFunction(ast_if);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_if = FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* flow = ir_if->As<ir::If>();
    ASSERT_NE(flow->true_.target, nullptr);
    ASSERT_NE(flow->false_.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, flow);
    EXPECT_EQ(flow->true_.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(flow->false_.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, IfStatement_JumpChainToMerge) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_if = FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    EXPECT_TRUE(ir_if->Is<ir::If>());

    auto* if_flow = ir_if->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    auto* ir_loop = FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(func->start_target->branch.target, if_flow);
    EXPECT_EQ(if_flow->true_.target->As<ir::Block>()->branch.target, loop_flow);
    EXPECT_EQ(loop_flow->start.target->As<ir::Block>()->branch.target, loop_flow->merge.target);
    EXPECT_EQ(loop_flow->merge.target->As<ir::Block>()->branch.target, if_flow->merge.target);
    EXPECT_EQ(loop_flow->continuing.target->As<ir::Block>()->branch.target,
              loop_flow->start.target);
    EXPECT_EQ(if_flow->false_.target->As<ir::Block>()->branch.target, if_flow->merge.target);
    EXPECT_EQ(if_flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, Loop_WithBreak) {
    // func -> start -> loop -> loop start -> loop merge -> func end
    //
    //   [continuing] -> loop start
    //
    auto* ast_loop = Loop(Block(Break()));
    WrapInFunction(ast_loop);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_loop = FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->As<ir::Block>()->branch.target, flow);
    EXPECT_EQ(flow->start.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->continuing.target->As<ir::Block>()->branch.target, flow->start.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, Loop_WithContinue) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_loop = FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* ir_if = FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    ASSERT_TRUE(ir_if->Is<ir::If>());

    auto* if_flow = ir_if->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, loop_flow);
    EXPECT_EQ(loop_flow->start.target->As<ir::Block>()->branch.target, if_flow);
    EXPECT_EQ(if_flow->true_.target->As<ir::Block>()->branch.target, loop_flow->merge.target);
    EXPECT_EQ(if_flow->false_.target->As<ir::Block>()->branch.target, if_flow->merge.target);
    EXPECT_EQ(loop_flow->continuing.target->As<ir::Block>()->branch.target,
              loop_flow->start.target);
    EXPECT_EQ(loop_flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, Loop_WithContinuing_BreakIf) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_loop = FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* ir_break_if = FlowNodeForAstNode(ast_break_if);
    ASSERT_NE(ir_break_if, nullptr);
    ASSERT_TRUE(ir_break_if->Is<ir::If>());

    auto* break_if_flow = ir_break_if->As<ir::If>();
    ASSERT_NE(break_if_flow->true_.target, nullptr);
    ASSERT_NE(break_if_flow->false_.target, nullptr);
    ASSERT_NE(break_if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, break_if_flow->inbound_branches.Length());
    EXPECT_EQ(1u, break_if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, break_if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, break_if_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, loop_flow);
    EXPECT_EQ(loop_flow->start.target->As<ir::Block>()->branch.target,
              loop_flow->continuing.target);
    EXPECT_EQ(loop_flow->continuing.target->As<ir::Block>()->branch.target, break_if_flow);
    EXPECT_EQ(break_if_flow->true_.target->As<ir::Block>()->branch.target, loop_flow->merge.target);
    EXPECT_EQ(break_if_flow->false_.target->As<ir::Block>()->branch.target,
              break_if_flow->merge.target);
    EXPECT_EQ(break_if_flow->merge.target->As<ir::Block>()->branch.target, loop_flow->start.target);
    EXPECT_EQ(loop_flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, Loop_WithReturn) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_loop = FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* ir_if = FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    ASSERT_TRUE(ir_if->Is<ir::If>());

    auto* if_flow = ir_if->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(loop_flow->start.target->As<ir::Block>()->branch.target, if_flow);
    EXPECT_EQ(if_flow->true_.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(if_flow->false_.target->As<ir::Block>()->branch.target, if_flow->merge.target);
    EXPECT_EQ(loop_flow->continuing.target->As<ir::Block>()->branch.target,
              loop_flow->start.target);

    EXPECT_EQ(func->start_target->branch.target, ir_loop);
    EXPECT_EQ(loop_flow->merge.target->As<ir::Block>()->branch.target, nullptr);

    // Check condition
    ASSERT_TRUE(if_flow->condition->Is<Constant>());
    auto* instr = if_flow->condition->As<Constant>()->value;
    ASSERT_TRUE(instr->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(instr->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, Loop_WithOnlyReturn) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_loop = FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(loop_flow->start.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(loop_flow->continuing.target->As<ir::Block>()->branch.target,
              loop_flow->start.target);

    EXPECT_EQ(func->start_target->branch.target, ir_loop);
}

TEST_F(IR_BuilderImplTest, Loop_WithOnlyReturn_ContinuingBreakIf) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_loop = FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* ir_if = FlowNodeForAstNode(ast_if);
    EXPECT_EQ(ir_if, nullptr);

    auto* ir_break_if = FlowNodeForAstNode(ast_break_if);
    ASSERT_NE(ir_break_if, nullptr);
    EXPECT_TRUE(ir_break_if->Is<ir::If>());

    auto* break_if_flow = ir_break_if->As<ir::If>();
    ASSERT_NE(break_if_flow->true_.target, nullptr);
    ASSERT_NE(break_if_flow->false_.target, nullptr);
    ASSERT_NE(break_if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    // This is 1 because only the loop branch happens. The subsequent if return is dead code.
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(loop_flow->start.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(loop_flow->continuing.target->As<ir::Block>()->branch.target, break_if_flow);

    EXPECT_EQ(func->start_target->branch.target, ir_loop);
}

TEST_F(IR_BuilderImplTest, Loop_WithIf_BothBranchesBreak) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_loop = FlowNodeForAstNode(ast_loop);
    ASSERT_NE(ir_loop, nullptr);
    EXPECT_TRUE(ir_loop->Is<ir::Loop>());

    auto* loop_flow = ir_loop->As<ir::Loop>();
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* ir_if = FlowNodeForAstNode(ast_if);
    ASSERT_NE(ir_if, nullptr);
    ASSERT_TRUE(ir_if->Is<ir::If>());

    auto* if_flow = ir_if->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(0u, if_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    // Note, the `continue` is dead code because both if branches go out of loop, so it just gets
    // dropped.

    EXPECT_EQ(func->start_target->branch.target, loop_flow);
    EXPECT_EQ(loop_flow->start.target->As<ir::Block>()->branch.target, if_flow);
    EXPECT_EQ(if_flow->true_.target->As<ir::Block>()->branch.target, loop_flow->merge.target);
    EXPECT_EQ(if_flow->false_.target->As<ir::Block>()->branch.target, loop_flow->merge.target);
    EXPECT_EQ(loop_flow->continuing.target->As<ir::Block>()->branch.target,
              loop_flow->start.target);
    EXPECT_EQ(loop_flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, Loop_Nested) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_loop_a = FlowNodeForAstNode(ast_loop_a);
    ASSERT_NE(ir_loop_a, nullptr);
    EXPECT_TRUE(ir_loop_a->Is<ir::Loop>());
    auto* loop_flow_a = ir_loop_a->As<ir::Loop>();
    ASSERT_NE(loop_flow_a->start.target, nullptr);
    ASSERT_NE(loop_flow_a->continuing.target, nullptr);
    ASSERT_NE(loop_flow_a->merge.target, nullptr);

    auto* ir_loop_b = FlowNodeForAstNode(ast_loop_b);
    ASSERT_NE(ir_loop_b, nullptr);
    EXPECT_TRUE(ir_loop_b->Is<ir::Loop>());
    auto* loop_flow_b = ir_loop_b->As<ir::Loop>();
    ASSERT_NE(loop_flow_b->start.target, nullptr);
    ASSERT_NE(loop_flow_b->continuing.target, nullptr);
    ASSERT_NE(loop_flow_b->merge.target, nullptr);

    auto* ir_loop_c = FlowNodeForAstNode(ast_loop_c);
    ASSERT_NE(ir_loop_c, nullptr);
    EXPECT_TRUE(ir_loop_c->Is<ir::Loop>());
    auto* loop_flow_c = ir_loop_c->As<ir::Loop>();
    ASSERT_NE(loop_flow_c->start.target, nullptr);
    ASSERT_NE(loop_flow_c->continuing.target, nullptr);
    ASSERT_NE(loop_flow_c->merge.target, nullptr);

    auto* ir_loop_d = FlowNodeForAstNode(ast_loop_d);
    ASSERT_NE(ir_loop_d, nullptr);
    EXPECT_TRUE(ir_loop_d->Is<ir::Loop>());
    auto* loop_flow_d = ir_loop_d->As<ir::Loop>();
    ASSERT_NE(loop_flow_d->start.target, nullptr);
    ASSERT_NE(loop_flow_d->continuing.target, nullptr);
    ASSERT_NE(loop_flow_d->merge.target, nullptr);

    auto* ir_if_a = FlowNodeForAstNode(ast_if_a);
    ASSERT_NE(ir_if_a, nullptr);
    EXPECT_TRUE(ir_if_a->Is<ir::If>());
    auto* if_flow_a = ir_if_a->As<ir::If>();
    ASSERT_NE(if_flow_a->true_.target, nullptr);
    ASSERT_NE(if_flow_a->false_.target, nullptr);
    ASSERT_NE(if_flow_a->merge.target, nullptr);

    auto* ir_if_b = FlowNodeForAstNode(ast_if_b);
    ASSERT_NE(ir_if_b, nullptr);
    EXPECT_TRUE(ir_if_b->Is<ir::If>());
    auto* if_flow_b = ir_if_b->As<ir::If>();
    ASSERT_NE(if_flow_b->true_.target, nullptr);
    ASSERT_NE(if_flow_b->false_.target, nullptr);
    ASSERT_NE(if_flow_b->merge.target, nullptr);

    auto* ir_if_c = FlowNodeForAstNode(ast_if_c);
    ASSERT_NE(ir_if_c, nullptr);
    EXPECT_TRUE(ir_if_c->Is<ir::If>());
    auto* if_flow_c = ir_if_c->As<ir::If>();
    ASSERT_NE(if_flow_c->true_.target, nullptr);
    ASSERT_NE(if_flow_c->false_.target, nullptr);
    ASSERT_NE(if_flow_c->merge.target, nullptr);

    auto* ir_if_d = FlowNodeForAstNode(ast_if_d);
    ASSERT_NE(ir_if_d, nullptr);
    EXPECT_TRUE(ir_if_d->Is<ir::If>());
    auto* if_flow_d = ir_if_d->As<ir::If>();
    ASSERT_NE(if_flow_d->true_.target, nullptr);
    ASSERT_NE(if_flow_d->false_.target, nullptr);
    ASSERT_NE(if_flow_d->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, loop_flow_a->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_a->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_a->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_a->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_b->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_b->start.target->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_b->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_b->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_c->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_c->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow_c->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_c->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_d->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow_d->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_d->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow_d->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_a->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_a->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_a->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_a->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_b->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_b->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_b->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_b->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_c->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_c->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_c->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_c->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_d->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_d->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_d->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow_d->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, loop_flow_a);
    EXPECT_EQ(loop_flow_a->start.target->As<ir::Block>()->branch.target, loop_flow_b);
    EXPECT_EQ(loop_flow_b->start.target->As<ir::Block>()->branch.target, if_flow_a);
    EXPECT_EQ(if_flow_a->true_.target->As<ir::Block>()->branch.target, loop_flow_b->merge.target);
    EXPECT_EQ(if_flow_a->false_.target->As<ir::Block>()->branch.target, if_flow_a->merge.target);
    EXPECT_EQ(if_flow_a->merge.target->As<ir::Block>()->branch.target, if_flow_b);
    EXPECT_EQ(if_flow_b->true_.target->As<ir::Block>()->branch.target,
              loop_flow_b->continuing.target);
    EXPECT_EQ(if_flow_b->false_.target->As<ir::Block>()->branch.target, if_flow_b->merge.target);
    EXPECT_EQ(if_flow_b->merge.target->As<ir::Block>()->branch.target,
              loop_flow_b->continuing.target);
    EXPECT_EQ(loop_flow_b->continuing.target->As<ir::Block>()->branch.target, loop_flow_c);
    EXPECT_EQ(loop_flow_c->start.target->As<ir::Block>()->branch.target, loop_flow_c->merge.target);
    EXPECT_EQ(loop_flow_c->continuing.target->As<ir::Block>()->branch.target,
              loop_flow_c->start.target);
    EXPECT_EQ(loop_flow_c->merge.target->As<ir::Block>()->branch.target, loop_flow_d);
    EXPECT_EQ(loop_flow_d->start.target->As<ir::Block>()->branch.target,
              loop_flow_d->continuing.target);
    EXPECT_EQ(loop_flow_d->continuing.target->As<ir::Block>()->branch.target, if_flow_c);
    EXPECT_EQ(if_flow_c->true_.target->As<ir::Block>()->branch.target, loop_flow_d->merge.target);
    EXPECT_EQ(if_flow_c->false_.target->As<ir::Block>()->branch.target, if_flow_c->merge.target);
    EXPECT_EQ(if_flow_c->merge.target->As<ir::Block>()->branch.target, loop_flow_d->start.target);
    EXPECT_EQ(loop_flow_d->merge.target->As<ir::Block>()->branch.target, loop_flow_b->start.target);
    EXPECT_EQ(loop_flow_b->merge.target->As<ir::Block>()->branch.target, if_flow_d);
    EXPECT_EQ(if_flow_d->true_.target->As<ir::Block>()->branch.target, loop_flow_a->merge.target);
    EXPECT_EQ(if_flow_d->false_.target->As<ir::Block>()->branch.target, if_flow_d->merge.target);
    EXPECT_EQ(if_flow_d->merge.target->As<ir::Block>()->branch.target,
              loop_flow_a->continuing.target);
    EXPECT_EQ(loop_flow_a->continuing.target->As<ir::Block>()->branch.target,
              loop_flow_a->start.target);
    EXPECT_EQ(loop_flow_a->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, While) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_while = FlowNodeForAstNode(ast_while);
    ASSERT_NE(ir_while, nullptr);
    ASSERT_TRUE(ir_while->Is<ir::Loop>());

    auto* flow = ir_while->As<ir::Loop>();
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_NE(flow->start.target->As<ir::Block>()->branch.target, nullptr);
    ASSERT_TRUE(flow->start.target->As<ir::Block>()->branch.target->Is<ir::If>());
    auto* if_flow = flow->start.target->As<ir::Block>()->branch.target->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge.target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, flow);
    EXPECT_EQ(flow->start.target->As<ir::Block>()->branch.target, if_flow);
    EXPECT_EQ(if_flow->true_.target->As<ir::Block>()->branch.target, if_flow->merge.target);
    EXPECT_EQ(if_flow->false_.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(if_flow->merge.target->As<ir::Block>()->branch.target, flow->continuing.target);
    EXPECT_EQ(flow->continuing.target->As<ir::Block>()->branch.target, flow->start.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);

    // Check condition
    ASSERT_TRUE(if_flow->condition->Is<Constant>());
    auto* instr = if_flow->condition->As<Constant>()->value;
    ASSERT_TRUE(instr->Is<constant::Scalar<bool>>());
    EXPECT_FALSE(instr->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, While_Return) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_while = FlowNodeForAstNode(ast_while);
    ASSERT_NE(ir_while, nullptr);
    ASSERT_TRUE(ir_while->Is<ir::Loop>());

    auto* flow = ir_while->As<ir::Loop>();
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_NE(flow->start.target->As<ir::Block>()->branch.target, nullptr);
    ASSERT_TRUE(flow->start.target->As<ir::Block>()->branch.target->Is<ir::If>());
    auto* if_flow = flow->start.target->As<ir::Block>()->branch.target->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge.target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, flow);
    EXPECT_EQ(flow->start.target->As<ir::Block>()->branch.target, if_flow);
    EXPECT_EQ(if_flow->true_.target->As<ir::Block>()->branch.target, if_flow->merge.target);
    EXPECT_EQ(if_flow->false_.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(if_flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(flow->continuing.target->As<ir::Block>()->branch.target, flow->start.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

// TODO(dsinclair): Enable when variable declarations and increment are supported
TEST_F(IR_BuilderImplTest, DISABLED_For) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_for = FlowNodeForAstNode(ast_for);
    ASSERT_NE(ir_for, nullptr);
    ASSERT_TRUE(ir_for->Is<ir::Loop>());

    auto* flow = ir_for->As<ir::Loop>();
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_NE(flow->start.target->As<ir::Block>()->branch.target, nullptr);
    ASSERT_TRUE(flow->start.target->As<ir::Block>()->branch.target->Is<ir::If>());
    auto* if_flow = flow->start.target->As<ir::Block>()->branch.target->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge.target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, flow);
    EXPECT_EQ(flow->start.target->As<ir::Block>()->branch.target, if_flow);
    EXPECT_EQ(if_flow->true_.target->As<ir::Block>()->branch.target, if_flow->merge.target);
    EXPECT_EQ(if_flow->false_.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(if_flow->merge.target->As<ir::Block>()->branch.target, flow->continuing.target);
    EXPECT_EQ(flow->continuing.target->As<ir::Block>()->branch.target, flow->start.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);

    // Check condition
    ASSERT_TRUE(if_flow->condition->Is<Constant>());
    auto* instr = if_flow->condition->As<Constant>()->value;
    ASSERT_TRUE(instr->Is<constant::Scalar<bool>>());
    EXPECT_FALSE(instr->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, For_NoInitCondOrContinuing) {
    // for (;;) {
    //   break;
    // }
    //
    // func -> loop -> loop start -> loop merge -> func end
    //
    auto* ast_for = For(nullptr, nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_for = FlowNodeForAstNode(ast_for);
    ASSERT_NE(ir_for, nullptr);
    ASSERT_TRUE(ir_for->Is<ir::Loop>());

    auto* flow = ir_for->As<ir::Loop>();
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(flow->start.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->continuing.target->As<ir::Block>()->branch.target, flow->start.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, Switch) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_switch = FlowNodeForAstNode(ast_switch);
    ASSERT_NE(ir_switch, nullptr);
    ASSERT_TRUE(ir_switch->Is<ir::Switch>());

    auto* flow = ir_switch->As<ir::Switch>();
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(3u, flow->cases.Length());

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    ASSERT_TRUE(flow->cases[0].selectors[0].val->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i, flow->cases[0].selectors[0].val->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    ASSERT_TRUE(flow->cases[1].selectors[0].val->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(1_i, flow->cases[1].selectors[0].val->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, flow->cases[2].selectors.Length());
    EXPECT_TRUE(flow->cases[2].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[2].start.target->inbound_branches.Length());
    EXPECT_EQ(3u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, ir_switch);
    EXPECT_EQ(flow->cases[0].start.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->cases[1].start.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->cases[2].start.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);

    // Check condition
    ASSERT_TRUE(flow->condition->Is<Constant>());
    auto* instr = flow->condition->As<Constant>()->value;
    ASSERT_TRUE(instr->Is<constant::Scalar<i32>>());
    EXPECT_EQ(1_i, instr->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BuilderImplTest, Switch_OnlyDefault) {
    // func -> switch -> default -> switch merge -> func end
    //
    auto* ast_switch = Switch(1_i, utils::Vector{DefaultCase(Block())});

    WrapInFunction(ast_switch);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_switch = FlowNodeForAstNode(ast_switch);
    ASSERT_NE(ir_switch, nullptr);
    ASSERT_TRUE(ir_switch->Is<ir::Switch>());

    auto* flow = ir_switch->As<ir::Switch>();
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(1u, flow->cases.Length());

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    EXPECT_TRUE(flow->cases[0].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, ir_switch);
    EXPECT_EQ(flow->cases[0].start.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, Switch_WithBreak) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    auto* ir_switch = FlowNodeForAstNode(ast_switch);
    ASSERT_NE(ir_switch, nullptr);
    ASSERT_TRUE(ir_switch->Is<ir::Switch>());

    auto* flow = ir_switch->As<ir::Switch>();
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(2u, flow->cases.Length());

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    ASSERT_TRUE(flow->cases[0].selectors[0].val->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i, flow->cases[0].selectors[0].val->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    EXPECT_TRUE(flow->cases[1].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start.target->inbound_branches.Length());
    EXPECT_EQ(2u, flow->merge.target->inbound_branches.Length());
    // This is 1 because the if is dead-code eliminated and the return doesn't happen.
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, ir_switch);
    EXPECT_EQ(flow->cases[0].start.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->cases[1].start.target->As<ir::Block>()->branch.target, flow->merge.target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, func->end_target);
}

TEST_F(IR_BuilderImplTest, Switch_AllReturn) {
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
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    ASSERT_EQ(FlowNodeForAstNode(ast_if), nullptr);

    auto* ir_switch = FlowNodeForAstNode(ast_switch);
    ASSERT_NE(ir_switch, nullptr);
    ASSERT_TRUE(ir_switch->Is<ir::Switch>());

    auto* flow = ir_switch->As<ir::Switch>();
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(2u, flow->cases.Length());

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    ASSERT_TRUE(flow->cases[0].selectors[0].val->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i, flow->cases[0].selectors[0].val->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    EXPECT_TRUE(flow->cases[1].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(func->start_target->branch.target, ir_switch);
    EXPECT_EQ(flow->cases[0].start.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(flow->cases[1].start.target->As<ir::Block>()->branch.target, func->end_target);
    EXPECT_EQ(flow->merge.target->As<ir::Block>()->branch.target, nullptr);
}

TEST_F(IR_BuilderImplTest, EmitLiteral_Bool_True) {
    auto* expr = Expr(true);
    GlobalVar("a", ty.bool_(), ast::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_TRUE(r) << b.error();

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(val->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_Bool_False) {
    auto* expr = Expr(false);
    GlobalVar("a", ty.bool_(), ast::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_TRUE(r) << b.error();

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<bool>>());
    EXPECT_FALSE(val->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_F32) {
    auto* expr = Expr(1.2_f);
    GlobalVar("a", ty.f32(), ast::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_TRUE(r) << b.error();

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<f32>>());
    EXPECT_EQ(1.2_f, val->As<constant::Scalar<f32>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_F16) {
    Enable(ast::Extension::kF16);
    auto* expr = Expr(1.2_h);
    GlobalVar("a", ty.f16(), ast::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_TRUE(r) << b.error();

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<f16>>());
    EXPECT_EQ(1.2_h, val->As<constant::Scalar<f16>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_I32) {
    auto* expr = Expr(-2_i);
    GlobalVar("a", ty.i32(), ast::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_TRUE(r) << b.error();

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<i32>>());
    EXPECT_EQ(-2_i, val->As<constant::Scalar<i32>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_U32) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), ast::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_TRUE(r) << b.error();

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<u32>>());
    EXPECT_EQ(2_u, val->As<constant::Scalar<u32>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Add) {
    auto* expr = Add(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 + 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Subtract) {
    auto* expr = Sub(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 - 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Multiply) {
    auto* expr = Mul(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 * 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Div) {
    auto* expr = Div(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 / 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Modulo) {
    auto* expr = Mod(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 % 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_And) {
    auto* expr = And(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 & 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Or) {
    auto* expr = Or(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 | 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Xor) {
    auto* expr = Xor(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 ^ 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LogicalAnd) {
    auto* expr = LogicalAnd(true, false);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (bool) = true && false
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LogicalOr) {
    auto* expr = LogicalOr(false, true);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (bool) = false || true
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Equal) {
    auto* expr = Equal(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (bool) = 3 == 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_NotEqual) {
    auto* expr = NotEqual(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (bool) = 3 != 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LessThan) {
    auto* expr = LessThan(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (bool) = 3 < 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_GreaterThan) {
    auto* expr = GreaterThan(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (bool) = 3 > 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LessThanEqual) {
    auto* expr = LessThanEqual(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (bool) = 3 <= 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_GreaterThanEqual) {
    auto* expr = GreaterThanEqual(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (bool) = 3 >= 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_ShiftLeft) {
    auto* expr = Shl(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 << 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_ShiftRight) {
    auto* expr = Shr(3_u, 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 >> 4
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Compound) {
    auto* expr = LogicalAnd(LessThan(1_u, Add(Shr(3_u, 4_u), 9_u)),
                            GreaterThan(2.5_f, Div(6.7_f, Mul(2.3_f, 5.5_f))));
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (u32) = 3 >> 4
%2 (u32) = %1 (u32) + 9
%3 (bool) = 1 < %2 (u32)
%4 (f32) = 2.3 * 5.5
%5 (f32) = 6.7 / %4 (f32)
%6 (bool) = 2.5 > %5 (f32)
%7 (bool) = %3 (bool) && %6 (bool)
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Bitcast) {
    auto* expr = Bitcast(ty.f32(), 3_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_TRUE(r) << b.error();

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1 (f32) = bitcast(3)
)");
}

}  // namespace
}  // namespace tint::ir
