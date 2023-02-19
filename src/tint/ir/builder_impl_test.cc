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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function f
  %bb1 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = if (true)
    # true branch
    %bb3 = Block
    BranchTo %bb4 ()

    # false branch
    %bb5 = Block
    BranchTo %bb4 ()

  # if merge
  %bb4 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = if (true)
    # true branch
    %bb3 = Block
    Return ()
    # false branch
    %bb4 = Block
    BranchTo %bb5 ()

  # if merge
  %bb5 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = if (true)
    # true branch
    %bb3 = Block
    BranchTo %bb4 ()

    # false branch
    %bb5 = Block
    Return ()
  # if merge
  %bb4 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = if (true)
    # true branch
    %bb3 = Block
    Return ()
    # false branch
    %bb4 = Block
    Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = if (true)
    # true branch
    %bb3 = Block
    BranchTo %bb4 ()

    %bb4 = loop
      # loop start
      %bb5 = Block
      BranchTo %bb6 ()

      # loop continuing
      %bb7 = Block
      BranchTo %bb5 ()

    # loop merge
    %bb6 = Block
    BranchTo %bb8 ()

    # false branch
    %bb9 = Block
    BranchTo %bb8 ()

  # if merge
  %bb8 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    # loop continuing
    %bb5 = Block
    BranchTo %bb3 ()

  # loop merge
  %bb4 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    %bb4 = if (true)
      # true branch
      %bb5 = Block
      BranchTo %bb6 ()

      # false branch
      %bb7 = Block
      BranchTo %bb8 ()

    # if merge
    %bb8 = Block
    BranchTo %bb9 ()

    # loop continuing
    %bb9 = Block
    BranchTo %bb3 ()

  # loop merge
  %bb6 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    # loop continuing
    %bb4 = Block
    BranchTo %bb5 ()

    %bb5 = if (true)
      # true branch
      %bb6 = Block
      BranchTo %bb7 ()

      # false branch
      %bb8 = Block
      BranchTo %bb9 ()

    # if merge
    %bb9 = Block
    BranchTo %bb3 ()

  # loop merge
  %bb7 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    %bb4 = if (true)
      # true branch
      %bb5 = Block
      Return ()
      # false branch
      %bb6 = Block
      BranchTo %bb7 ()

    # if merge
    %bb7 = Block
    BranchTo %bb8 ()

    # loop continuing
    %bb8 = Block
    BranchTo %bb3 ()

  # loop merge
  # Dead
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    Return ()
    # loop continuing
    %bb4 = Block
    BranchTo %bb3 ()

  # loop merge
  # Dead
FunctionEnd

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithOnlyReturn_ContinuingBreakIf) {
    // Note, even though there is code in the loop merge (specifically, the
    // `ast_if` below), it doesn't get emitted as there is no way to reach the
    // loop merge due to the loop itself doing a `return`. This is why the
    // loop merge gets marked as Dead and the `ast_if` doesn't appear.
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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    Return ()
    # loop continuing
    %bb4 = Block
    BranchTo %bb5 ()

    %bb5 = if (true)
      # true branch
      %bb6 = Block
      BranchTo %bb7 ()

      # false branch
      %bb8 = Block
      BranchTo %bb9 ()

    # if merge
    %bb9 = Block
    BranchTo %bb3 ()

  # loop merge
  # Dead
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    %bb4 = if (true)
      # true branch
      %bb5 = Block
      BranchTo %bb6 ()

      # false branch
      %bb7 = Block
      BranchTo %bb6 ()

    # loop continuing
    %bb8 = Block
    BranchTo %bb3 ()

  # loop merge
  %bb6 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    %bb4 = loop
      # loop start
      %bb5 = Block
      BranchTo %bb6 ()

      %bb6 = if (true)
        # true branch
        %bb7 = Block
        BranchTo %bb8 ()

        # false branch
        %bb9 = Block
        BranchTo %bb10 ()

      # if merge
      %bb10 = Block
      BranchTo %bb11 ()

      %bb11 = if (true)
        # true branch
        %bb12 = Block
        BranchTo %bb13 ()

        # false branch
        %bb14 = Block
        BranchTo %bb15 ()

      # if merge
      %bb15 = Block
      BranchTo %bb13 ()

      # loop continuing
      %bb13 = Block
      BranchTo %bb16 ()

      %bb16 = loop
        # loop start
        %bb17 = Block
        BranchTo %bb18 ()

        # loop continuing
        %bb19 = Block
        BranchTo %bb17 ()

      # loop merge
      %bb18 = Block
      BranchTo %bb20 ()

      %bb20 = loop
        # loop start
        %bb21 = Block
        BranchTo %bb22 ()

        # loop continuing
        %bb22 = Block
        BranchTo %bb23 ()

        %bb23 = if (true)
          # true branch
          %bb24 = Block
          BranchTo %bb25 ()

          # false branch
          %bb26 = Block
          BranchTo %bb27 ()

        # if merge
        %bb27 = Block
        BranchTo %bb21 ()

      # loop merge
      %bb25 = Block
      BranchTo %bb5 ()

    # loop merge
    %bb8 = Block
    BranchTo %bb28 ()

    %bb28 = if (true)
      # true branch
      %bb29 = Block
      BranchTo %bb30 ()

      # false branch
      %bb31 = Block
      BranchTo %bb32 ()

    # if merge
    %bb32 = Block
    BranchTo %bb33 ()

    # loop continuing
    %bb33 = Block
    BranchTo %bb3 ()

  # loop merge
  %bb30 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    %bb4 = if (false)
      # true branch
      %bb5 = Block
      BranchTo %bb6 ()

      # false branch
      %bb7 = Block
      BranchTo %bb8 ()

    # if merge
    %bb6 = Block
    BranchTo %bb9 ()

    # loop continuing
    %bb9 = Block
    BranchTo %bb3 ()

  # loop merge
  %bb8 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    %bb4 = if (true)
      # true branch
      %bb5 = Block
      BranchTo %bb6 ()

      # false branch
      %bb7 = Block
      BranchTo %bb8 ()

    # if merge
    %bb6 = Block
    Return ()
    # loop continuing
    %bb9 = Block
    BranchTo %bb3 ()

  # loop merge
  %bb8 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = loop
    # loop start
    %bb3 = Block
    BranchTo %bb4 ()

    # loop continuing
    %bb5 = Block
    BranchTo %bb3 ()

  # loop merge
  %bb4 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = Switch (1)
    # Case 0
    %bb3 = Block
    BranchTo %bb4 ()

    # Case 1
    %bb5 = Block
    BranchTo %bb4 ()

    # Case default
    %bb6 = Block
    BranchTo %bb4 ()

  # Switch Merge
  %bb4 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = Switch (1)
    # Case default
    %bb3 = Block
    BranchTo %bb4 ()

  # Switch Merge
  %bb4 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = Switch (1)
    # Case 0
    %bb3 = Block
    BranchTo %bb4 ()

    # Case default
    %bb5 = Block
    BranchTo %bb4 ()

  # Switch Merge
  %bb4 = Block
  Return ()
FunctionEnd

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

    EXPECT_EQ(Disassemble(m), R"(%bb0 = Function test_function
  %bb1 = Block
  BranchTo %bb2 ()

  %bb2 = Switch (1)
    # Case 0
    %bb3 = Block
    Return ()
    # Case default
    %bb4 = Block
    Return ()
  # Switch Merge
  # Dead
FunctionEnd

)");
}

TEST_F(IR_BuilderImplTest, EmitLiteral_Bool_True) {
    auto* expr = Expr(true);
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate, expr);

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
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate, expr);

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
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate, expr);

    auto& b = CreateBuilder();
    auto r = b.EmitLiteral(expr);
    ASSERT_TRUE(r) << b.error();

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
    ASSERT_TRUE(r) << b.error();

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
    ASSERT_TRUE(r) << b.error();

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
    auto* expr = Bitcast<f32>(3_u);
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
