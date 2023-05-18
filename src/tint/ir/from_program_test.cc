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
    ASSERT_NE(f->StartTarget(), nullptr);
    ASSERT_NE(f->EndTarget(), nullptr);

    EXPECT_EQ(1u, f->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, f->EndTarget()->InboundBranches().Length());

    EXPECT_EQ(m->functions[0]->Stage(), Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func f():void {
  %fn2 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Func_WithParam) {
    Func("f", utils::Vector{Param("a", ty.u32())}, ty.u32(), utils::Vector{Return("a")});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->StartTarget(), nullptr);
    ASSERT_NE(f->EndTarget(), nullptr);

    EXPECT_EQ(1u, f->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, f->EndTarget()->InboundBranches().Length());

    EXPECT_EQ(m->functions[0]->Stage(), Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func f(%a:u32):u32 {
  %fn2 = block {
  } -> %func_end %a # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, Func_WithMultipleParam) {
    Func("f", utils::Vector{Param("a", ty.u32()), Param("b", ty.i32()), Param("c", ty.bool_())},
         ty.void_(), utils::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->StartTarget(), nullptr);
    ASSERT_NE(f->EndTarget(), nullptr);

    EXPECT_EQ(1u, f->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, f->EndTarget()->InboundBranches().Length());

    EXPECT_EQ(m->functions[0]->Stage(), Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func f(%a:u32, %b:i32, %c:bool):void {
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

    EXPECT_EQ(m->functions[0]->Stage(), Function::PipelineStage::kFragment);
}

TEST_F(IR_BuilderImplTest, IfStatement) {
    auto* ast_if = If(true, Block(), Else(Block()));
    WrapInFunction(ast_if);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(flow->True().target, nullptr);
    ASSERT_NE(flow->False().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->False().target->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->True().target, nullptr);
    ASSERT_NE(flow->False().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(2u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->True().target, nullptr);
    ASSERT_NE(flow->False().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(2u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->True().target, nullptr);
    ASSERT_NE(flow->False().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->False().target->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(2u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(if_flow->True().target, nullptr);
    ASSERT_NE(if_flow->False().target, nullptr);
    ASSERT_NE(if_flow->Merge().target, nullptr);

    auto* loop_flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(loop_flow, nullptr);
    ASSERT_NE(loop_flow->Start().target, nullptr);
    ASSERT_NE(loop_flow->Continuing().target, nullptr);
    ASSERT_NE(loop_flow->Merge().target, nullptr);

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
    ASSERT_NE(flow->Start().target, nullptr);
    ASSERT_NE(flow->Continuing().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(loop_flow->Start().target, nullptr);
    ASSERT_NE(loop_flow->Continuing().target, nullptr);
    ASSERT_NE(loop_flow->Merge().target, nullptr);

    auto* if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(if_flow->True().target, nullptr);
    ASSERT_NE(if_flow->False().target, nullptr);
    ASSERT_NE(if_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(loop_flow->Start().target, nullptr);
    ASSERT_NE(loop_flow->Continuing().target, nullptr);
    ASSERT_NE(loop_flow->Merge().target, nullptr);

    auto* break_if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(break_if_flow->True().target, nullptr);
    ASSERT_NE(break_if_flow->False().target, nullptr);
    ASSERT_NE(break_if_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, break_if_flow->InboundBranches().Length());
    EXPECT_EQ(1u, break_if_flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, break_if_flow->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, break_if_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(loop_flow->Start().target, nullptr);
    ASSERT_NE(loop_flow->Continuing().target, nullptr);
    ASSERT_NE(loop_flow->Merge().target, nullptr);

    auto* if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(if_flow->True().target, nullptr);
    ASSERT_NE(if_flow->False().target, nullptr);
    ASSERT_NE(if_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(loop_flow->Start().target, nullptr);
    ASSERT_NE(loop_flow->Continuing().target, nullptr);
    ASSERT_NE(loop_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(loop_flow->Start().target, nullptr);
    ASSERT_NE(loop_flow->Continuing().target, nullptr);
    ASSERT_NE(loop_flow->Merge().target, nullptr);

    auto* break_if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(break_if_flow->True().target, nullptr);
    ASSERT_NE(break_if_flow->False().target, nullptr);
    ASSERT_NE(break_if_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    // This is 1 because only the loop branch happens. The subsequent if return is dead code.
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(loop_flow->Start().target, nullptr);
    ASSERT_NE(loop_flow->Continuing().target, nullptr);
    ASSERT_NE(loop_flow->Merge().target, nullptr);

    auto* if_flow = FindSingleFlowNode<ir::If>(m.Get());
    ASSERT_NE(if_flow->True().target, nullptr);
    ASSERT_NE(if_flow->False().target, nullptr);
    ASSERT_NE(if_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False().target->InboundBranches().Length());
    EXPECT_EQ(0u, if_flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
            return block->Branch().target;
        }
        return nullptr;
    };

    auto* loop_flow_a = As<ir::Loop>(m->functions[0]->StartTarget()->Branch().target);
    ASSERT_NE(loop_flow_a, nullptr);
    ASSERT_NE(loop_flow_a->Start().target, nullptr);
    ASSERT_NE(loop_flow_a->Continuing().target, nullptr);
    ASSERT_NE(loop_flow_a->Merge().target, nullptr);

    auto* loop_flow_b = As<ir::Loop>(block_exit(loop_flow_a->Start().target));
    ASSERT_NE(loop_flow_b, nullptr);
    ASSERT_NE(loop_flow_b->Start().target, nullptr);
    ASSERT_NE(loop_flow_b->Continuing().target, nullptr);
    ASSERT_NE(loop_flow_b->Merge().target, nullptr);

    auto* if_flow_a = As<ir::If>(block_exit(loop_flow_b->Start().target));
    ASSERT_NE(if_flow_a, nullptr);
    ASSERT_NE(if_flow_a->True().target, nullptr);
    ASSERT_NE(if_flow_a->False().target, nullptr);
    ASSERT_NE(if_flow_a->Merge().target, nullptr);

    auto* if_flow_b = As<ir::If>(block_exit(if_flow_a->Merge().target));
    ASSERT_NE(if_flow_b, nullptr);
    ASSERT_NE(if_flow_b->True().target, nullptr);
    ASSERT_NE(if_flow_b->False().target, nullptr);
    ASSERT_NE(if_flow_b->Merge().target, nullptr);

    auto* loop_flow_c = As<ir::Loop>(block_exit(loop_flow_b->Continuing().target));
    ASSERT_NE(loop_flow_c, nullptr);
    ASSERT_NE(loop_flow_c->Start().target, nullptr);
    ASSERT_NE(loop_flow_c->Continuing().target, nullptr);
    ASSERT_NE(loop_flow_c->Merge().target, nullptr);

    auto* loop_flow_d = As<ir::Loop>(block_exit(loop_flow_c->Merge().target));
    ASSERT_NE(loop_flow_d, nullptr);
    ASSERT_NE(loop_flow_d->Start().target, nullptr);
    ASSERT_NE(loop_flow_d->Continuing().target, nullptr);
    ASSERT_NE(loop_flow_d->Merge().target, nullptr);

    auto* if_flow_c = As<ir::If>(block_exit(loop_flow_d->Continuing().target));
    ASSERT_NE(if_flow_c, nullptr);
    ASSERT_NE(if_flow_c->True().target, nullptr);
    ASSERT_NE(if_flow_c->False().target, nullptr);
    ASSERT_NE(if_flow_c->Merge().target, nullptr);

    auto* if_flow_d = As<ir::If>(block_exit(loop_flow_b->Merge().target));
    ASSERT_NE(if_flow_d, nullptr);
    ASSERT_NE(if_flow_d->True().target, nullptr);
    ASSERT_NE(if_flow_d->False().target, nullptr);
    ASSERT_NE(if_flow_d->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, loop_flow_a->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow_a->Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_a->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_a->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_b->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow_b->Start().target->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow_b->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_b->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_c->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow_c->Start().target->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow_c->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_c->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_d->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow_d->Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_d->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow_d->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_a->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_a->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_a->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_a->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_b->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_b->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_b->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_b->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_c->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_c->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_c->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_c->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_d->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_d->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_d->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow_d->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->StartTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->Start().target, nullptr);
    ASSERT_NE(flow->Continuing().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_NE(flow->Start().target->As<ir::Block>()->Branch().target, nullptr);
    ASSERT_TRUE(flow->Start().target->As<ir::Block>()->Branch().target->Is<ir::If>());
    auto* if_flow = flow->Start().target->As<ir::Block>()->Branch().target->As<ir::If>();
    ASSERT_NE(if_flow->True().target, nullptr);
    ASSERT_NE(if_flow->False().target, nullptr);
    ASSERT_NE(if_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge().target->InboundBranches().Length());

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
    ASSERT_NE(flow->Start().target, nullptr);
    ASSERT_NE(flow->Continuing().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_NE(flow->Start().target->As<ir::Block>()->Branch().target, nullptr);
    ASSERT_TRUE(flow->Start().target->As<ir::Block>()->Branch().target->Is<ir::If>());
    auto* if_flow = flow->Start().target->As<ir::Block>()->Branch().target->As<ir::If>();
    ASSERT_NE(if_flow->True().target, nullptr);
    ASSERT_NE(if_flow->False().target, nullptr);
    ASSERT_NE(if_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(2u, func->EndTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge().target->InboundBranches().Length());

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
    ASSERT_NE(flow->Start().target, nullptr);
    ASSERT_NE(flow->Continuing().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_NE(flow->Start().target->As<ir::Block>()->Branch().target, nullptr);
    ASSERT_TRUE(flow->Start().target->As<ir::Block>()->Branch().target->Is<ir::If>());
    auto* if_flow = flow->Start().target->As<ir::Block>()->Branch().target->As<ir::If>();
    ASSERT_NE(if_flow->True().target, nullptr);
    ASSERT_NE(if_flow->False().target, nullptr);
    ASSERT_NE(if_flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False().target->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge().target->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m.Get()), R"()");
}

TEST_F(IR_BuilderImplTest, For_NoInitCondOrContinuing) {
    auto* ast_for = For(nullptr, nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    auto* flow = FindSingleFlowNode<ir::Loop>(m.Get());
    ASSERT_NE(flow->Start().target, nullptr);
    ASSERT_NE(flow->Continuing().target, nullptr);
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Start().target->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Continuing().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    auto cases = flow->Cases();
    ASSERT_EQ(3u, cases.Length());

    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    ASSERT_TRUE(cases[1].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(1_i,
              cases[1].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[2].selectors.Length());
    EXPECT_TRUE(cases[2].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, cases[0].Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, cases[1].Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, cases[2].Start().target->InboundBranches().Length());
    EXPECT_EQ(3u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    auto cases = flow->Cases();
    ASSERT_EQ(1u, cases.Length());
    ASSERT_EQ(3u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_TRUE(cases[0].selectors[1].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(1_i,
              cases[0].selectors[1].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    EXPECT_TRUE(cases[0].selectors[2].IsDefault());

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, cases[0].Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    auto cases = flow->Cases();
    ASSERT_EQ(1u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    EXPECT_TRUE(cases[0].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, cases[0].Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    auto cases = flow->Cases();
    ASSERT_EQ(2u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    EXPECT_TRUE(cases[1].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, cases[0].Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, cases[1].Start().target->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Merge().target->InboundBranches().Length());
    // This is 1 because the if is dead-code eliminated and the return doesn't happen.
    EXPECT_EQ(1u, func->EndTarget()->InboundBranches().Length());

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
    ASSERT_NE(flow->Merge().target, nullptr);

    ASSERT_EQ(1u, m->functions.Length());
    auto* func = m->functions[0];

    auto cases = flow->Cases();
    ASSERT_EQ(2u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    EXPECT_TRUE(cases[1].selectors[0].IsDefault());

    EXPECT_EQ(1u, flow->InboundBranches().Length());
    EXPECT_EQ(1u, cases[0].Start().target->InboundBranches().Length());
    EXPECT_EQ(1u, cases[1].Start().target->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Merge().target->InboundBranches().Length());
    EXPECT_EQ(2u, func->EndTarget()->InboundBranches().Length());

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

TEST_F(IR_BuilderImplTest, Emit_Phony) {
    Func("b", utils::Empty, ty.i32(), Return(1_i));
    WrapInFunction(Ignore(Call("b")));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%fn1 = func b():i32 {
  %fn2 = block {
  } -> %func_end 1i # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:i32 = call b
  } -> %func_end # return
} %func_end

)");
}

}  // namespace
}  // namespace tint::ir
