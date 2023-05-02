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

#include "gmock/gmock.h"
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_BuilderImplTest = TestHelper;

TEST_F(IR_BuilderImplTest, Func) {
    Func("f", utils::Empty, ty.void_(), utils::Empty);
    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    ASSERT_EQ(0u, m.entry_points.Length());
    ASSERT_EQ(1u, m.functions.Length());

    auto* f = m.functions[0];
    ASSERT_NE(f->start_target, nullptr);
    ASSERT_NE(f->end_target, nullptr);

    EXPECT_EQ(1u, f->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, f->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func f
  %fn1 = block
  ret
func_end

)");
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = if true [t: %fn3, f: %fn4, m: %fn5]
    # true branch
    %fn3 = block
    branch %fn5

    # false branch
    %fn4 = block
    branch %fn5

  # if merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, IfStatement_TrueReturns) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = if true [t: %fn3, f: %fn4, m: %fn5]
    # true branch
    %fn3 = block
    ret
    # false branch
    %fn4 = block
    branch %fn5

  # if merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, IfStatement_FalseReturns) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = if true [t: %fn3, f: %fn4, m: %fn5]
    # true branch
    %fn3 = block
    branch %fn5

    # false branch
    %fn4 = block
    ret
  # if merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, IfStatement_BothReturn) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = if true [t: %fn3, f: %fn4]
    # true branch
    %fn3 = block
    ret
    # false branch
    %fn4 = block
    ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, IfStatement_JumpChainToMerge) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = if true [t: %fn3, f: %fn4, m: %fn5]
    # true branch
    %fn3 = block
    branch %fn6

    %fn6 = loop [s: %fn7, m: %fn8]
      # loop start
      %fn7 = block
      branch %fn8

    # loop merge
    %fn8 = block
    branch %fn5

    # false branch
    %fn4 = block
    branch %fn5

  # if merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithBreak) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, m: %fn4]
    # loop start
    %fn3 = block
    branch %fn4

  # loop merge
  %fn4 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithContinue) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, c: %fn4, m: %fn5]
    # loop start
    %fn3 = block
    branch %fn6

    %fn6 = if true [t: %fn7, f: %fn8, m: %fn9]
      # true branch
      %fn7 = block
      branch %fn5

      # false branch
      %fn8 = block
      branch %fn9

    # if merge
    %fn9 = block
    branch %fn4

    # loop continuing
    %fn4 = block
    branch %fn3

  # loop merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithContinuing_BreakIf) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, c: %fn4, m: %fn5]
    # loop start
    %fn3 = block
    branch %fn4

    # loop continuing
    %fn4 = block
    branch %fn6

    %fn6 = if true [t: %fn7, f: %fn8, m: %fn9]
      # true branch
      %fn7 = block
      branch %fn5

      # false branch
      %fn8 = block
      branch %fn9

    # if merge
    %fn9 = block
    branch %fn3

  # loop merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithReturn) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, c: %fn4]
    # loop start
    %fn3 = block
    branch %fn5

    %fn5 = if true [t: %fn6, f: %fn7, m: %fn8]
      # true branch
      %fn6 = block
      ret
      # false branch
      %fn7 = block
      branch %fn8

    # if merge
    %fn8 = block
    branch %fn4

    # loop continuing
    %fn4 = block
    branch %fn3

func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithOnlyReturn) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3]
    # loop start
    %fn3 = block
    ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithOnlyReturn_ContinuingBreakIf) {
    // Note, even though there is code in the loop merge (specifically, the
    // `ast_if` below), it doesn't get emitted as there is no way to reach the
    // loop merge due to the loop itself doing a `return`. This is why the
    // loop merge gets marked as Dead and the `ast_if` doesn't appear.
    //
    // Similar, the continuing block goes away as there is no way to get there, so it's treated
    // as dead code and dropped.
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3]
    # loop start
    %fn3 = block
    ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithIf_BothBranchesBreak) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, m: %fn4]
    # loop start
    %fn3 = block
    branch %fn5

    %fn5 = if true [t: %fn6, f: %fn7]
      # true branch
      %fn6 = block
      branch %fn4

      # false branch
      %fn7 = block
      branch %fn4

  # loop merge
  %fn4 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_Nested) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, c: %fn4, m: %fn5]
    # loop start
    %fn3 = block
    branch %fn6

    %fn6 = loop [s: %fn7, c: %fn8, m: %fn9]
      # loop start
      %fn7 = block
      branch %fn10

      %fn10 = if true [t: %fn11, f: %fn12, m: %fn13]
        # true branch
        %fn11 = block
        branch %fn9

        # false branch
        %fn12 = block
        branch %fn13

      # if merge
      %fn13 = block
      branch %fn14

      %fn14 = if true [t: %fn15, f: %fn16, m: %fn17]
        # true branch
        %fn15 = block
        branch %fn8

        # false branch
        %fn16 = block
        branch %fn17

      # if merge
      %fn17 = block
      branch %fn8

      # loop continuing
      %fn8 = block
      branch %fn18

      %fn18 = loop [s: %fn19, m: %fn20]
        # loop start
        %fn19 = block
        branch %fn20

      # loop merge
      %fn20 = block
      branch %fn21

      %fn21 = loop [s: %fn22, c: %fn23, m: %fn24]
        # loop start
        %fn22 = block
        branch %fn23

        # loop continuing
        %fn23 = block
        branch %fn25

        %fn25 = if true [t: %fn26, f: %fn27, m: %fn28]
          # true branch
          %fn26 = block
          branch %fn24

          # false branch
          %fn27 = block
          branch %fn28

        # if merge
        %fn28 = block
        branch %fn22

      # loop merge
      %fn24 = block
      branch %fn7

    # loop merge
    %fn9 = block
    branch %fn29

    %fn29 = if true [t: %fn30, f: %fn31, m: %fn32]
      # true branch
      %fn30 = block
      branch %fn5

      # false branch
      %fn31 = block
      branch %fn32

    # if merge
    %fn32 = block
    branch %fn4

    # loop continuing
    %fn4 = block
    branch %fn3

  # loop merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, While) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, c: %fn4, m: %fn5]
    # loop start
    %fn3 = block
    branch %fn6

    %fn6 = if false [t: %fn7, f: %fn8, m: %fn9]
      # true branch
      %fn7 = block
      branch %fn9

      # false branch
      %fn8 = block
      branch %fn5

    # if merge
    %fn9 = block
    branch %fn4

    # loop continuing
    %fn4 = block
    branch %fn3

  # loop merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, While_Return) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, m: %fn4]
    # loop start
    %fn3 = block
    branch %fn5

    %fn5 = if true [t: %fn6, f: %fn7, m: %fn8]
      # true branch
      %fn6 = block
      branch %fn8

      # false branch
      %fn7 = block
      branch %fn4

    # if merge
    %fn8 = block
    ret
  # loop merge
  %fn4 = block
  ret
func_end

)");
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

    EXPECT_EQ(Disassemble(m), R"()");
}

TEST_F(IR_BuilderImplTest, For_NoInitCondOrContinuing) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = loop [s: %fn3, m: %fn4]
    # loop start
    %fn3 = block
    branch %fn4

  # loop merge
  %fn4 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch) {
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
    ASSERT_TRUE(flow->cases[0].selectors[0].val->value->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              flow->cases[0].selectors[0].val->value->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    ASSERT_TRUE(flow->cases[1].selectors[0].val->value->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(1_i,
              flow->cases[1].selectors[0].val->value->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, flow->cases[2].selectors.Length());
    EXPECT_TRUE(flow->cases[2].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[2].start.target->inbound_branches.Length());
    EXPECT_EQ(3u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = switch 1i [c: (0i, %fn3), c: (1i, %fn4), c: (default, %fn5), m: %fn6]
    # case 0i
    %fn3 = block
    branch %fn6

    # case 1i
    %fn4 = block
    branch %fn6

    # case default
    %fn5 = block
    branch %fn6

  # switch merge
  %fn6 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch_MultiSelector) {
    auto* ast_switch = Switch(
        1_i,
        utils::Vector{Case(
            utils::Vector{CaseSelector(0_i), CaseSelector(1_i), DefaultCaseSelector()}, Block())});

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

    ASSERT_EQ(3u, flow->cases[0].selectors.Length());
    ASSERT_TRUE(flow->cases[0].selectors[0].val->value->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              flow->cases[0].selectors[0].val->value->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_TRUE(flow->cases[0].selectors[1].val->value->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(1_i,
              flow->cases[0].selectors[1].val->value->As<constant::Scalar<tint::i32>>()->ValueOf());

    EXPECT_TRUE(flow->cases[0].selectors[2].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = switch 1i [c: (0i 1i default, %fn3), m: %fn4]
    # case 0i 1i default
    %fn3 = block
    branch %fn4

  # switch merge
  %fn4 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch_OnlyDefault) {
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

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = switch 1i [c: (default, %fn3), m: %fn4]
    # case default
    %fn3 = block
    branch %fn4

  # switch merge
  %fn4 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch_WithBreak) {
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
    ASSERT_TRUE(flow->cases[0].selectors[0].val->value->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              flow->cases[0].selectors[0].val->value->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    EXPECT_TRUE(flow->cases[1].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start.target->inbound_branches.Length());
    EXPECT_EQ(2u, flow->merge.target->inbound_branches.Length());
    // This is 1 because the if is dead-code eliminated and the return doesn't happen.
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = switch 1i [c: (0i, %fn3), c: (default, %fn4), m: %fn5]
    # case 0i
    %fn3 = block
    branch %fn5

    # case default
    %fn4 = block
    branch %fn5

  # switch merge
  %fn5 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch_AllReturn) {
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
    ASSERT_TRUE(flow->cases[0].selectors[0].val->value->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              flow->cases[0].selectors[0].val->value->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, flow->cases[1].selectors.Length());
    EXPECT_TRUE(flow->cases[1].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[1].start.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  branch %fn2

  %fn2 = switch 1i [c: (0i, %fn3), c: (default, %fn4)]
    # case 0i
    %fn3 = block
    ret
    # case default
    %fn4 = block
    ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitLiteral_Bool_True) {
    auto* expr = Expr(true);
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<bool>>());
    EXPECT_TRUE(val->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_Bool_False) {
    auto* expr = Expr(false);
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<bool>>());
    EXPECT_FALSE(val->As<constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_F32) {
    auto* expr = Expr(1.2_f);
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<f32>>());
    EXPECT_EQ(1.2_f, val->As<constant::Scalar<f32>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_F16) {
    Enable(builtin::Extension::kF16);
    auto* expr = Expr(1.2_h);
    GlobalVar("a", ty.f16(), builtin::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<f16>>());
    EXPECT_EQ(1.2_h, val->As<constant::Scalar<f16>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_I32) {
    auto* expr = Expr(-2_i);
    GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<i32>>());
    EXPECT_EQ(-2_i, val->As<constant::Scalar<i32>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, EmitLiteral_U32) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    ASSERT_TRUE(r.Get()->Is<Constant>());
    auto* val = r.Get()->As<Constant>()->value;
    EXPECT_TRUE(val->Is<constant::Scalar<u32>>());
    EXPECT_EQ(2_u, val->As<constant::Scalar<u32>>()->ValueAs<f32>());
}

TEST_F(IR_BuilderImplTest, Emit_GlobalVar_NoInit) {
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPrivate);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = block
%1(ref<private, u32, read_write>) = var private read_write
ret

)");
}

TEST_F(IR_BuilderImplTest, Emit_GlobalVar_Init) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPrivate, expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = block
%1(ref<private, u32, read_write>) = var private read_write
store %1(ref<private, u32, read_write>), 2u
ret

)");
}

TEST_F(IR_BuilderImplTest, Emit_Var_NoInit) {
    auto* a = Var("a", ty.u32(), builtin::AddressSpace::kFunction);
    WrapInFunction(a);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  %1(ref<function, u32, read_write>) = var function read_write
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Emit_Var_Init) {
    auto* expr = Expr(2_u);
    auto* a = Var("a", ty.u32(), builtin::AddressSpace::kFunction, expr);
    WrapInFunction(a);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  %1(ref<function, u32, read_write>) = var function read_write
  store %1(ref<function, u32, read_write>), 2u
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Add) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Add(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = add %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Subtract) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Sub(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = sub %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Multiply) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Mul(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = mul %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Div) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Div(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = div %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Modulo) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Mod(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = mod %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_And) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = And(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = bit_and %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Or) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Or(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = bit_or %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Xor) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Xor(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = bit_xor %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LogicalAnd) {
    Func("my_func", utils::Empty, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = LogicalAnd(Call("my_func"), false);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(bool) = call my_func
%2(bool) = log_and %1(bool), false
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LogicalOr) {
    Func("my_func", utils::Empty, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = LogicalOr(Call("my_func"), true);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(bool) = call my_func
%2(bool) = log_or %1(bool), true
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Equal) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Equal(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(bool) = eq %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_NotEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = NotEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(bool) = neq %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LessThan) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = LessThan(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(bool) = lt %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_GreaterThan) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = GreaterThan(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(bool) = gt %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LessThanEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = LessThanEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(bool) = lte %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_GreaterThanEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = GreaterThanEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(bool) = gte %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_ShiftLeft) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Shl(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = shiftl %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_ShiftRight) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Shr(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(u32) = call my_func
%2(u32) = shiftr %1(u32), 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Compound) {
    Func("my_func", utils::Empty, ty.f32(), utils::Vector{Return(0_f)});

    auto* expr = LogicalAnd(LessThan(Call("my_func"), 2_f),
                            GreaterThan(2.5_f, Div(Call("my_func"), Mul(2.3_f, Call("my_func")))));
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(f32) = call my_func
%2(bool) = lt %1(f32), 2.0f
%3(f32) = call my_func
%4(f32) = call my_func
%5(f32) = mul 2.29999995231628417969f, %4(f32)
%6(f32) = div %3(f32), %5(f32)
%7(bool) = gt 2.5f, %6(f32)
%8(bool) = log_and %2(bool), %7(bool)
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Compound_WithConstEval) {
    Func("my_func", utils::Vector{Param("p", ty.bool_())}, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = Call("my_func", LogicalAnd(LessThan(2.4_f, 2_f),
                                            GreaterThan(2.5_f, Div(10_f, Mul(2.3_f, 9.4_f)))));
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(bool) = call my_func, false
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Bitcast) {
    Func("my_func", utils::Empty, ty.f32(), utils::Vector{Return(0_f)});

    auto* expr = Bitcast<f32>(Call("my_func"));
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(f32) = call my_func
%2(f32) = bitcast %1(f32)
)");
}

TEST_F(IR_BuilderImplTest, EmitStatement_Discard) {
    auto* expr = Discard();
    Func("test_function", {}, ty.void_(), expr,
         utils::Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    auto& b = CreateBuilder();
    InjectFlowBlock();
    b.EmitStatement(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(discard
)");
}

TEST_F(IR_BuilderImplTest, EmitStatement_UserFunction) {
    Func("my_func", utils::Vector{Param("p", ty.f32())}, ty.void_(), utils::Empty);

    auto* stmt = CallStmt(Call("my_func", Mul(2_a, 3_a)));
    WrapInFunction(stmt);

    auto& b = CreateBuilder();

    InjectFlowBlock();
    b.EmitStatement(stmt);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(void) = call my_func, 6.0f
)");
}

// TODO(dsinclair): This needs assignment in order to output correctly. The empty constructor ends
// up materializing, so there is no expression to emit until there is a usage. When assigment is
// implemented this can be enabled (and the output updated).
TEST_F(IR_BuilderImplTest, DISABLED_EmitExpression_ConstructEmpty) {
    auto* expr = vec3(ty.f32());
    GlobalVar("i", builtin::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1(vec3<f32>) = construct
)");
}

// Requires identifier expressions
TEST_F(IR_BuilderImplTest, DISABLED_EmitExpression_Construct) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_f));
    auto* expr = vec3(ty.f32(), 2_f, 3_f, i);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%2(vec3<f32>) = construct 2.0f, 3.0f, %1(void)
)");
}

// Requires identifier expressions
TEST_F(IR_BuilderImplTest, DISABLED_EmitExpression_Convert) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_i));
    auto* expr = Call(ty.f32(), i);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%2(f32) = convert i32, %1(void)
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_MaterializedCall) {
    auto* expr = Return(Call("trunc", 2.5_f));

    Func("test_function", {}, ty.f32(), expr, utils::Empty);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test_function
  %fn1 = block
  ret 2.0f
func_end

)");
}

// Requires identifier expressions
TEST_F(IR_BuilderImplTest, DISABLED_EmitExpression_Builtin) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_f));
    auto* expr = Call("asin", i);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%2(f32) = asin %1(void)
)");
}

}  // namespace
}  // namespace tint::ir
