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

namespace tint::ir {
namespace {

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

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    EXPECT_EQ(func->start_target->branch_target, flow);
    EXPECT_EQ(flow->true_target->branch_target, func->end_target);
    EXPECT_EQ(flow->false_target->branch_target, func->end_target);
    EXPECT_EQ(flow->merge_target, nullptr);
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
    //   [loop merge] -> func end
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

    EXPECT_EQ(loop_flow->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, func->end_target);
    EXPECT_EQ(if_flow->false_target->branch_target, if_flow->merge_target);

    EXPECT_EQ(loop_flow->continuing_target->branch_target, loop_flow->start_target);

    EXPECT_EQ(func->start_target->branch_target, ir_loop);
    EXPECT_EQ(loop_flow->merge_target->branch_target, func->end_target);
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

    ASSERT_EQ(1u, m.functions.Length());
    auto* func = m.functions[0];

    // Note, the `continue` is dead code because both if branches go out of loop, so it just gets
    // dropped.

    EXPECT_EQ(func->start_target->branch_target, loop_flow);
    EXPECT_EQ(loop_flow->start_target->branch_target, if_flow);
    EXPECT_EQ(if_flow->true_target->branch_target, loop_flow->merge_target);
    EXPECT_EQ(if_flow->false_target->branch_target, loop_flow->merge_target);
    EXPECT_EQ(if_flow->merge_target, nullptr);
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

}  // namespace
}  // namespace tint::ir
