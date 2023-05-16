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

#include "src/tint/ir/test_helper.h"

#include "gmock/gmock.h"
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/function_terminator.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/switch.h"

namespace tint::ir {
namespace {

/// Looks for the flow node with the given type T.
/// If no flow node is found, then nullptr is returned.
/// If multiple flow nodes are found with the type T, then an error is raised and the first is
/// returned.
template <typename T>
const T* FindSingleFlowNode(const Module& mod) {
    const T* found = nullptr;
    size_t count = 0;
    for (auto* node : mod.flow_nodes.Objects()) {
        if (auto* as = node->As<T>()) {
            count++;
            if (!found) {
                found = as;
            }
        }
    }
    if (count > 1) {
        ADD_FAILURE() << "FindSingleFlowNode() found " << count << " nodes of type "
                      << utils::TypeInfo::Of<T>().name;
    }
    return found;
}

using namespace tint::number_suffixes;  // NOLINT

using IR_BuilderImplTest = TestHelper;

TEST_F(IR_BuilderImplTest, Func) {
    Func("f", utils::Empty, ty.void_(), utils::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->start_target, nullptr);
    ASSERT_NE(f->end_target, nullptr);

    EXPECT_EQ(1u, f->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, f->end_target->inbound_branches.Length());

    EXPECT_EQ(m->functions[0]->pipeline_stage, Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func f():void {
  %fn2 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EntryPoint) {
    Func("f", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(m->functions[0]->pipeline_stage, Function::PipelineStage::kFragment);
}

TEST_F(IR_BuilderImplTest, IfStatement) {
    auto* ast_if = If(true, Block(), Else(Block()));
    WrapInFunction(ast_if);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(flow->true_.target, nullptr);
    ASSERT_NE(flow->false_.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(2u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = if true [t: %fn4, f: %fn5, m: %fn6]
    # true branch
    %fn4 = block {
    } -> %fn6 # branch

    # false branch
    %fn5 = block {
    } -> %fn6 # branch

  # if merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, IfStatement_TrueReturns) {
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_if);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(flow->true_.target, nullptr);
    ASSERT_NE(flow->false_.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = if true [t: %fn4, f: %fn5, m: %fn6]
    # true branch
    %fn4 = block {
    } -> %func_end # return
    # false branch
    %fn5 = block {
    } -> %fn6 # branch

  # if merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, IfStatement_FalseReturns) {
    auto* ast_if = If(true, Block(), Else(Block(Return())));
    WrapInFunction(ast_if);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(flow->true_.target, nullptr);
    ASSERT_NE(flow->false_.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = if true [t: %fn4, f: %fn5, m: %fn6]
    # true branch
    %fn4 = block {
    } -> %fn6 # branch

    # false branch
    %fn5 = block {
    } -> %func_end # return
  # if merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, IfStatement_BothReturn) {
    auto* ast_if = If(true, Block(Return()), Else(Block(Return())));
    WrapInFunction(ast_if);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(flow->true_.target, nullptr);
    ASSERT_NE(flow->false_.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = if true [t: %fn4, f: %fn5]
    # true branch
    %fn4 = block {
    } -> %func_end # return
    # false branch
    %fn5 = block {
    } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, IfStatement_JumpChainToMerge) {
    auto* ast_loop = Loop(Block(Break()));
    auto* ast_if = If(true, Block(ast_loop));
    WrapInFunction(ast_if);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    auto* loop_flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(loop_flow, nullptr);
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = if true [t: %fn4, f: %fn5, m: %fn6]
    # true branch
    %fn4 = block {
    } -> %fn7 # branch

    %fn7 = loop [s: %fn8, m: %fn9]
      # loop start
      %fn8 = block {
      } -> %fn9 # branch

    # loop merge
    %fn9 = block {
    } -> %fn6 # branch

    # false branch
    %fn5 = block {
    } -> %fn6 # branch

  # if merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithBreak) {
    auto* ast_loop = Loop(Block(Break()));
    WrapInFunction(ast_loop);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, m: %fn5]
    # loop start
    %fn4 = block {
    } -> %fn5 # branch

  # loop merge
  %fn5 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithContinue) {
    auto* ast_if = If(true, Block(Break()));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* loop_flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, c: %fn5, m: %fn6]
    # loop start
    %fn4 = block {
    } -> %fn7 # branch

    %fn7 = if true [t: %fn8, f: %fn9, m: %fn10]
      # true branch
      %fn8 = block {
      } -> %fn6 # branch

      # false branch
      %fn9 = block {
      } -> %fn10 # branch

    # if merge
    %fn10 = block {
    } -> %fn5 # branch

    # loop continuing
    %fn5 = block {
    } -> %fn4 # branch

  # loop merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithContinuing_BreakIf) {
    auto* ast_break_if = BreakIf(true);
    auto* ast_loop = Loop(Block(), Block(ast_break_if));
    WrapInFunction(ast_loop);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* loop_flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* break_if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(break_if_flow->true_.target, nullptr);
    ASSERT_NE(break_if_flow->false_.target, nullptr);
    ASSERT_NE(break_if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, c: %fn5, m: %fn6]
    # loop start
    %fn4 = block {
    } -> %fn5 # branch

    # loop continuing
    %fn5 = block {
    } -> %fn7 # branch

    %fn7 = if true [t: %fn8, f: %fn9, m: %fn10]
      # true branch
      %fn8 = block {
      } -> %fn6 # branch

      # false branch
      %fn9 = block {
      } -> %fn10 # branch

    # if merge
    %fn10 = block {
    } -> %fn4 # branch

  # loop merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_Continuing_Body_Scope) {
    auto* a = Decl(Let("a", Expr(true)));
    auto* ast_break_if = BreakIf("a");
    auto* ast_loop = Loop(Block(a), Block(ast_break_if));
    WrapInFunction(ast_loop);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, c: %fn5, m: %fn6]
    # loop start
    %fn4 = block {
    } -> %fn5 # branch

    # loop continuing
    %fn5 = block {
    } -> %fn7 # branch

    %fn7 = if true [t: %fn8, f: %fn9, m: %fn10]
      # true branch
      %fn8 = block {
      } -> %fn6 # branch

      # false branch
      %fn9 = block {
      } -> %fn10 # branch

    # if merge
    %fn10 = block {
    } -> %fn4 # branch

  # loop merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithReturn) {
    auto* ast_if = If(true, Block(Return()));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* loop_flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, c: %fn5]
    # loop start
    %fn4 = block {
    } -> %fn6 # branch

    %fn6 = if true [t: %fn7, f: %fn8, m: %fn9]
      # true branch
      %fn7 = block {
      } -> %func_end # return
      # false branch
      %fn8 = block {
      } -> %fn9 # branch

    # if merge
    %fn9 = block {
    } -> %fn5 # branch

    # loop continuing
    %fn5 = block {
    } -> %fn4 # branch

} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithOnlyReturn) {
    auto* ast_loop = Loop(Block(Return(), Continue()));
    WrapInFunction(ast_loop, If(true, Block(Return())));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* loop_flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4]
    # loop start
    %fn4 = block {
    } -> %func_end # return
} %func_end

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

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* loop_flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* break_if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(break_if_flow->true_.target, nullptr);
    ASSERT_NE(break_if_flow->false_.target, nullptr);
    ASSERT_NE(break_if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow->inbound_branches.Length());
    EXPECT_EQ(2u, loop_flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, loop_flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, loop_flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->start_target->inbound_branches.Length());
    // This is 1 because only the loop branch happens. The subsequent if return is dead code.
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4]
    # loop start
    %fn4 = block {
    } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Loop_WithIf_BothBranchesBreak) {
    auto* ast_if = If(true, Block(Break()), Else(Block(Break())));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* loop_flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(loop_flow->start.target, nullptr);
    ASSERT_NE(loop_flow->continuing.target, nullptr);
    ASSERT_NE(loop_flow->merge.target, nullptr);

    auto* if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, m: %fn5]
    # loop start
    %fn4 = block {
    } -> %fn6 # branch

    %fn6 = if true [t: %fn7, f: %fn8]
      # true branch
      %fn7 = block {
      } -> %fn5 # branch

      # false branch
      %fn8 = block {
      } -> %fn5 # branch

  # loop merge
  %fn5 = block {
  } -> %func_end # return
} %func_end

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

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto block_exit = [&](const ir::FlowNode* node) -> const ir::FlowNode* {
        if (auto* block = As<ir::Block>(node)) {
            return block->branch.target;
        }
        return nullptr;
    };

    auto* loop_flow_a = As<ir::Loop>(m->functions[0]->start_target->branch.target);
    ASSERT_NE(loop_flow_a, nullptr);
    ASSERT_NE(loop_flow_a->start.target, nullptr);
    ASSERT_NE(loop_flow_a->continuing.target, nullptr);
    ASSERT_NE(loop_flow_a->merge.target, nullptr);

    auto* loop_flow_b = As<ir::Loop>(block_exit(loop_flow_a->start.target));
    ASSERT_NE(loop_flow_b, nullptr);
    ASSERT_NE(loop_flow_b->start.target, nullptr);
    ASSERT_NE(loop_flow_b->continuing.target, nullptr);
    ASSERT_NE(loop_flow_b->merge.target, nullptr);

    auto* if_flow_a = As<ir::If>(block_exit(loop_flow_b->start.target));
    ASSERT_NE(if_flow_a, nullptr);
    ASSERT_NE(if_flow_a->true_.target, nullptr);
    ASSERT_NE(if_flow_a->false_.target, nullptr);
    ASSERT_NE(if_flow_a->merge.target, nullptr);

    auto* if_flow_b = As<ir::If>(block_exit(if_flow_a->merge.target));
    ASSERT_NE(if_flow_b, nullptr);
    ASSERT_NE(if_flow_b->true_.target, nullptr);
    ASSERT_NE(if_flow_b->false_.target, nullptr);
    ASSERT_NE(if_flow_b->merge.target, nullptr);

    auto* loop_flow_c = As<ir::Loop>(block_exit(loop_flow_b->continuing.target));
    ASSERT_NE(loop_flow_c, nullptr);
    ASSERT_NE(loop_flow_c->start.target, nullptr);
    ASSERT_NE(loop_flow_c->continuing.target, nullptr);
    ASSERT_NE(loop_flow_c->merge.target, nullptr);

    auto* loop_flow_d = As<ir::Loop>(block_exit(loop_flow_c->merge.target));
    ASSERT_NE(loop_flow_d, nullptr);
    ASSERT_NE(loop_flow_d->start.target, nullptr);
    ASSERT_NE(loop_flow_d->continuing.target, nullptr);
    ASSERT_NE(loop_flow_d->merge.target, nullptr);

    auto* if_flow_c = As<ir::If>(block_exit(loop_flow_d->continuing.target));
    ASSERT_NE(if_flow_c, nullptr);
    ASSERT_NE(if_flow_c->true_.target, nullptr);
    ASSERT_NE(if_flow_c->false_.target, nullptr);
    ASSERT_NE(if_flow_c->merge.target, nullptr);

    auto* if_flow_d = As<ir::If>(block_exit(loop_flow_b->merge.target));
    ASSERT_NE(if_flow_d, nullptr);
    ASSERT_NE(if_flow_d->true_.target, nullptr);
    ASSERT_NE(if_flow_d->false_.target, nullptr);
    ASSERT_NE(if_flow_d->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, c: %fn5, m: %fn6]
    # loop start
    %fn4 = block {
    } -> %fn7 # branch

    %fn7 = loop [s: %fn8, c: %fn9, m: %fn10]
      # loop start
      %fn8 = block {
      } -> %fn11 # branch

      %fn11 = if true [t: %fn12, f: %fn13, m: %fn14]
        # true branch
        %fn12 = block {
        } -> %fn10 # branch

        # false branch
        %fn13 = block {
        } -> %fn14 # branch

      # if merge
      %fn14 = block {
      } -> %fn15 # branch

      %fn15 = if true [t: %fn16, f: %fn17, m: %fn18]
        # true branch
        %fn16 = block {
        } -> %fn9 # branch

        # false branch
        %fn17 = block {
        } -> %fn18 # branch

      # if merge
      %fn18 = block {
      } -> %fn9 # branch

      # loop continuing
      %fn9 = block {
      } -> %fn19 # branch

      %fn19 = loop [s: %fn20, m: %fn21]
        # loop start
        %fn20 = block {
        } -> %fn21 # branch

      # loop merge
      %fn21 = block {
      } -> %fn22 # branch

      %fn22 = loop [s: %fn23, c: %fn24, m: %fn25]
        # loop start
        %fn23 = block {
        } -> %fn24 # branch

        # loop continuing
        %fn24 = block {
        } -> %fn26 # branch

        %fn26 = if true [t: %fn27, f: %fn28, m: %fn29]
          # true branch
          %fn27 = block {
          } -> %fn25 # branch

          # false branch
          %fn28 = block {
          } -> %fn29 # branch

        # if merge
        %fn29 = block {
        } -> %fn23 # branch

      # loop merge
      %fn25 = block {
      } -> %fn8 # branch

    # loop merge
    %fn10 = block {
    } -> %fn30 # branch

    %fn30 = if true [t: %fn31, f: %fn32, m: %fn33]
      # true branch
      %fn31 = block {
      } -> %fn6 # branch

      # false branch
      %fn32 = block {
      } -> %fn33 # branch

    # if merge
    %fn33 = block {
    } -> %fn5 # branch

    # loop continuing
    %fn5 = block {
    } -> %fn4 # branch

  # loop merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, While) {
    auto* ast_while = While(false, Block());
    WrapInFunction(ast_while);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_NE(flow->start.target->As<ir::Block>()->branch.target, nullptr);
    ASSERT_TRUE(flow->start.target->As<ir::Block>()->branch.target->Is<ir::If>());
    auto* if_flow = flow->start.target->As<ir::Block>()->branch.target->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge.target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, c: %fn5, m: %fn6]
    # loop start
    %fn4 = block {
    } -> %fn7 # branch

    %fn7 = if false [t: %fn8, f: %fn9, m: %fn10]
      # true branch
      %fn8 = block {
      } -> %fn10 # branch

      # false branch
      %fn9 = block {
      } -> %fn6 # branch

    # if merge
    %fn10 = block {
    } -> %fn5 # branch

    # loop continuing
    %fn5 = block {
    } -> %fn4 # branch

  # loop merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, While_Return) {
    auto* ast_while = While(true, Block(Return()));
    WrapInFunction(ast_while);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_NE(flow->start.target->As<ir::Block>()->branch.target, nullptr);
    ASSERT_TRUE(flow->start.target->As<ir::Block>()->branch.target->Is<ir::If>());
    auto* if_flow = flow->start.target->As<ir::Block>()->branch.target->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(2u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge.target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, m: %fn5]
    # loop start
    %fn4 = block {
    } -> %fn6 # branch

    %fn6 = if true [t: %fn7, f: %fn8, m: %fn9]
      # true branch
      %fn7 = block {
      } -> %fn9 # branch

      # false branch
      %fn8 = block {
      } -> %fn5 # branch

    # if merge
    %fn9 = block {
    } -> %func_end # return
  # loop merge
  %fn5 = block {
  } -> %func_end # return
} %func_end

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

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_NE(flow->start.target->As<ir::Block>()->branch.target, nullptr);
    ASSERT_TRUE(flow->start.target->As<ir::Block>()->branch.target->Is<ir::If>());
    auto* if_flow = flow->start.target->As<ir::Block>()->branch.target->As<ir::If>();
    ASSERT_NE(if_flow->true_.target, nullptr);
    ASSERT_NE(if_flow->false_.target, nullptr);
    ASSERT_NE(if_flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->true_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->false_.target->inbound_branches.Length());
    EXPECT_EQ(1u, if_flow->merge.target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()), R"()");
}

TEST_F(IR_BuilderImplTest, For_NoInitCondOrContinuing) {
    auto* ast_for = For(nullptr, nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(flow->start.target, nullptr);
    ASSERT_NE(flow->continuing.target, nullptr);
    ASSERT_NE(flow->merge.target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(2u, flow->start.target->inbound_branches.Length());
    EXPECT_EQ(0u, flow->continuing.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = loop [s: %fn4, m: %fn5]
    # loop start
    %fn4 = block {
    } -> %fn5 # branch

  # loop merge
  %fn5 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch) {
    auto* ast_switch = Switch(
        1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)}, Block()),
                           Case(utils::Vector{CaseSelector(1_i)}, Block()), DefaultCase(Block())});

    WrapInFunction(ast_switch);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Switch>(m.Get());
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(3u, flow->cases.Length());

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = switch 1i [c: (0i, %fn4), c: (1i, %fn5), c: (default, %fn6), m: %fn7]
    # case 0i
    %fn4 = block {
    } -> %fn7 # branch

    # case 1i
    %fn5 = block {
    } -> %fn7 # branch

    # case default
    %fn6 = block {
    } -> %fn7 # branch

  # switch merge
  %fn7 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch_MultiSelector) {
    auto* ast_switch = Switch(
        1_i,
        utils::Vector{Case(
            utils::Vector{CaseSelector(0_i), CaseSelector(1_i), DefaultCaseSelector()}, Block())});

    WrapInFunction(ast_switch);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Switch>(m.Get());
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(1u, flow->cases.Length());

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = switch 1i [c: (0i 1i default, %fn4), m: %fn5]
    # case 0i 1i default
    %fn4 = block {
    } -> %fn5 # branch

  # switch merge
  %fn5 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch_OnlyDefault) {
    auto* ast_switch = Switch(1_i, utils::Vector{DefaultCase(Block())});
    WrapInFunction(ast_switch);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Switch>(m.Get());
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(1u, flow->cases.Length());

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    ASSERT_EQ(1u, flow->cases[0].selectors.Length());
    EXPECT_TRUE(flow->cases[0].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->inbound_branches.Length());
    EXPECT_EQ(1u, flow->cases[0].start.target->inbound_branches.Length());
    EXPECT_EQ(1u, flow->merge.target->inbound_branches.Length());
    EXPECT_EQ(1u, func->end_target->inbound_branches.Length());

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = switch 1i [c: (default, %fn4), m: %fn5]
    # case default
    %fn4 = block {
    } -> %fn5 # branch

  # switch merge
  %fn5 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch_WithBreak) {
    auto* ast_switch = Switch(1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)},
                                                      Block(Break(), If(true, Block(Return())))),
                                                 DefaultCase(Block())});
    WrapInFunction(ast_switch);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Switch>(m.Get());
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(2u, flow->cases.Length());

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = switch 1i [c: (0i, %fn4), c: (default, %fn5), m: %fn6]
    # case 0i
    %fn4 = block {
    } -> %fn6 # branch

    # case default
    %fn5 = block {
    } -> %fn6 # branch

  # switch merge
  %fn6 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Switch_AllReturn) {
    auto* ast_switch =
        Switch(1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)}, Block(Return())),
                                  DefaultCase(Block(Return()))});
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_switch, ast_if);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(FindSingleFlowNode<ir::If>(m.Get()), nullptr);

    auto* flow = FindSingleFlowNode<ir::Switch>(m.Get());
    ASSERT_NE(flow->merge.target, nullptr);
    ASSERT_EQ(2u, flow->cases.Length());

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn2 = block {
  } -> %fn3 # branch

  %fn3 = switch 1i [c: (0i, %fn4), c: (default, %fn5)]
    # case 0i
    %fn4 = block {
    } -> %func_end # return
    # case default
    %fn5 = block {
    } -> %func_end # return
} %func_end

)");
}

}  // namespace
}  // namespace tint::ir
