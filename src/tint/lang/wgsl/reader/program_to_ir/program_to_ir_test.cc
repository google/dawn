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

#include "gmock/gmock.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/helpers/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

/// Looks for the instruction with the given type T.
/// If no instruction is found, then nullptr is returned.
/// If multiple instructions are found with the type T, then an error is raised and the first is
/// returned.
template <typename T>
T* FindSingleInstruction(ir::Module& mod) {
    T* found = nullptr;
    size_t count = 0;
    for (auto* node : mod.instructions.Objects()) {
        if (auto* as = node->As<T>()) {
            count++;
            if (!found) {
                found = as;
            }
        }
    }
    if (count > 1) {
        ADD_FAILURE() << "FindSingleInstruction() found " << count << " nodes of type "
                      << tint::TypeInfo::Of<T>().name;
    }
    return found;
}

using namespace tint::number_suffixes;  // NOLINT

using IR_FromProgramTest = helpers::IRProgramTest;

TEST_F(IR_FromProgramTest, Func) {
    Func("f", tint::Empty, ty.void_(), tint::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->Block(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), ir::Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%f = func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam) {
    Func("f", Vector{Param("a", ty.u32())}, ty.u32(), Vector{Return("a")});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->Block(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), ir::Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%f = func(%a:u32):u32 -> %b1 {
  %b1 = block {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithMultipleParam) {
    Func("f", Vector{Param("a", ty.u32()), Param("b", ty.i32()), Param("c", ty.bool_())},
         ty.void_(), tint::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->Block(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), ir::Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%f = func(%a:u32, %b:i32, %c:bool):void -> %b1 {
  %b1 = block {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, EntryPoint) {
    Func("f", tint::Empty, ty.void_(), tint::Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(m->functions[0]->Stage(), ir::Function::PipelineStage::kFragment);
}

TEST_F(IR_FromProgramTest, IfStatement) {
    auto* ast_if = If(true, Block(), Else(Block()));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        exit_if  # if_1
      }
      %b3 = block {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, IfStatement_TrueReturns) {
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, IfStatement_FalseReturns) {
    auto* ast_if = If(true, Block(), Else(Block(Return())));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        exit_if  # if_1
      }
      %b3 = block {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, IfStatement_BothReturn) {
    auto* ast_if = If(true, Block(Return()), Else(Block(Return())));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret
      }
      %b3 = block {  # false
        ret
      }
    }
    unreachable
  }
}
)");
}

TEST_F(IR_FromProgramTest, IfStatement_JumpChainToMerge) {
    auto* ast_loop = Loop(Block(Break()));
    auto* ast_if = If(true, Block(ast_loop));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        loop [b: %b3, c: %b4] {  # loop_1
          %b3 = block {  # body
            exit_loop  # loop_1
          }
          %b4 = block {  # continuing
            next_iteration %b3
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithBreak) {
    auto* ast_loop = Loop(Block(Break()));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        exit_loop  # loop_1
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithContinue) {
    auto* ast_if = If(true, Block(Break()));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        if true [t: %b4] {  # if_1
          %b4 = block {  # true
            exit_loop  # loop_1
          }
        }
        continue %b3
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithContinuing_BreakIf) {
    auto* ast_break_if = BreakIf(true);
    auto* ast_loop = Loop(Block(), Block(ast_break_if));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        continue %b3
      }
      %b3 = block {  # continuing
        break_if true %b2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_Continuing_Body_Scope) {
    auto* a = Decl(Let("a", Expr(true)));
    auto* ast_break_if = BreakIf("a");
    auto* ast_loop = Loop(Block(a), Block(ast_break_if));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        %a:bool = let true
        continue %b3
      }
      %b3 = block {  # continuing
        break_if %a %b2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithReturn) {
    auto* ast_if = If(true, Block(Return()));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        if true [t: %b4] {  # if_1
          %b4 = block {  # true
            ret
          }
        }
        continue %b3
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    unreachable
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithOnlyReturn) {
    auto* ast_loop = Loop(Block(Return(), Continue()));
    WrapInFunction(ast_loop, If(true, Block(Return())));

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        ret
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    unreachable
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithOnlyReturn_ContinuingBreakIf) {
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

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        ret
      }
      %b3 = block {  # continuing
        break_if true %b2
      }
    }
    if true [t: %b4] {  # if_1
      %b4 = block {  # true
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithIf_BothBranchesBreak) {
    auto* ast_if = If(true, Block(Break()), Else(Block(Break())));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        if true [t: %b4, f: %b5] {  # if_1
          %b4 = block {  # true
            exit_loop  # loop_1
          }
          %b5 = block {  # false
            exit_loop  # loop_1
          }
        }
        continue %b3
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_Nested) {
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

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        loop [b: %b4, c: %b5] {  # loop_2
          %b4 = block {  # body
            if true [t: %b6] {  # if_1
              %b6 = block {  # true
                exit_loop  # loop_2
              }
            }
            if true [t: %b7] {  # if_2
              %b7 = block {  # true
                continue %b5
              }
            }
            continue %b5
          }
          %b5 = block {  # continuing
            loop [b: %b8, c: %b9] {  # loop_3
              %b8 = block {  # body
                exit_loop  # loop_3
              }
              %b9 = block {  # continuing
                next_iteration %b8
              }
            }
            loop [b: %b10, c: %b11] {  # loop_4
              %b10 = block {  # body
                continue %b11
              }
              %b11 = block {  # continuing
                break_if true %b10
              }
            }
            next_iteration %b4
          }
        }
        if true [t: %b12] {  # if_3
          %b12 = block {  # true
            exit_loop  # loop_1
          }
        }
        continue %b3
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, While) {
    auto* ast_while = While(false, Block());
    WrapInFunction(ast_while);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        if false [t: %b4, f: %b5] {  # if_1
          %b4 = block {  # true
            exit_if  # if_1
          }
          %b5 = block {  # false
            exit_loop  # loop_1
          }
        }
        continue %b3
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, While_Return) {
    auto* ast_while = While(true, Block(Return()));
    WrapInFunction(ast_while);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3] {  # loop_1
      %b2 = block {  # body
        if true [t: %b4, f: %b5] {  # if_1
          %b4 = block {  # true
            exit_if  # if_1
          }
          %b5 = block {  # false
            exit_loop  # loop_1
          }
        }
        ret
      }
      %b3 = block {  # continuing
        next_iteration %b2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, For) {
    auto* ast_for = For(Decl(Var("i", ty.i32())), LessThan("i", 10_a), Increment("i"), Block());
    WrapInFunction(ast_for);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(2u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3, c: %b4] {  # loop_1
      %b2 = block {  # initializer
        %i:ptr<function, i32, read_write> = var
        next_iteration %b3
      }
      %b3 = block {  # body
        %3:i32 = load %i
        %4:bool = lt %3, 10i
        if %4 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            exit_if  # if_1
          }
          %b6 = block {  # false
            exit_loop  # loop_1
          }
        }
        continue %b4
      }
      %b4 = block {  # continuing
        %5:i32 = load %i
        %6:i32 = add %5, 1i
        store %i, %6
        next_iteration %b3
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, For_Init_NoCondOrContinuing) {
    auto* ast_for = For(Decl(Var("i", ty.i32())), nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3] {  # loop_1
      %b2 = block {  # initializer
        %i:ptr<function, i32, read_write> = var
        next_iteration %b3
      }
      %b3 = block {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, For_NoInitCondOrContinuing) {
    auto* ast_for = For(nullptr, nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* loop = FindSingleInstruction<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(0u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2] {  # loop_1
      %b2 = block {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch) {
    auto* ast_switch =
        Switch(1_i, Vector{Case(Vector{CaseSelector(0_i)}, Block()),
                           Case(Vector{CaseSelector(1_i)}, Block()), DefaultCase(Block())});

    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* swtch = FindSingleInstruction<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto cases = swtch->Cases();
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

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (0i, %b2), c: (1i, %b3), c: (default, %b4)] {  # switch_1
      %b2 = block {  # case
        exit_switch  # switch_1
      }
      %b3 = block {  # case
        exit_switch  # switch_1
      }
      %b4 = block {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_MultiSelector) {
    auto* ast_switch = Switch(
        1_i,
        Vector{Case(Vector{CaseSelector(0_i), CaseSelector(1_i), DefaultCaseSelector()}, Block())});

    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* swtch = FindSingleInstruction<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto cases = swtch->Cases();
    ASSERT_EQ(1u, cases.Length());
    ASSERT_EQ(3u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_TRUE(cases[0].selectors[1].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(1_i,
              cases[0].selectors[1].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    EXPECT_TRUE(cases[0].selectors[2].IsDefault());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (0i 1i default, %b2)] {  # switch_1
      %b2 = block {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_OnlyDefault) {
    auto* ast_switch = Switch(1_i, Vector{DefaultCase(Block())});
    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* swtch = FindSingleInstruction<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto cases = swtch->Cases();
    ASSERT_EQ(1u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    EXPECT_TRUE(cases[0].selectors[0].IsDefault());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_WithBreak) {
    auto* ast_switch = Switch(
        1_i, Vector{Case(Vector{CaseSelector(0_i)}, Block(Break(), If(true, Block(Return())))),
                    DefaultCase(Block())});
    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* swtch = FindSingleInstruction<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto cases = swtch->Cases();
    ASSERT_EQ(2u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    EXPECT_TRUE(cases[1].selectors[0].IsDefault());

    // This is 1 because the if is dead-code eliminated and the return doesn't happen.

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (0i, %b2), c: (default, %b3)] {  # switch_1
      %b2 = block {  # case
        exit_switch  # switch_1
      }
      %b3 = block {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_AllReturn) {
    auto* ast_switch = Switch(1_i, Vector{Case(Vector{CaseSelector(0_i)}, Block(Return())),
                                          DefaultCase(Block(Return()))});
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_switch, ast_if);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();

    auto* swtch = FindSingleInstruction<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto cases = swtch->Cases();
    ASSERT_EQ(2u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    EXPECT_TRUE(cases[1].selectors[0].IsDefault());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (0i, %b2), c: (default, %b3)] {  # switch_1
      %b2 = block {  # case
        ret
      }
      %b3 = block {  # case
        ret
      }
    }
    unreachable
  }
}
)");
}

TEST_F(IR_FromProgramTest, Emit_Phony) {
    Func("b", tint::Empty, ty.i32(), Return(1_i));
    WrapInFunction(Ignore(Call("b")));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%b = func():i32 -> %b1 {
  %b1 = block {
    ret 1i
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:i32 = call %b
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Invariant) {
    Func("f",
         Vector{Param("a", ty.vec4<f32>(),
                      Vector{Invariant(), Builtin(core::BuiltinValue::kPosition)})},
         ty.vec4<f32>(), Vector{Return("a")}, Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Location(1_i)});
    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(
        Disassemble(m.Get()),
        R"(%f = @fragment func(%a:vec4<f32> [@invariant, @position]):vec4<f32> [@location(1)] -> %b1 {
  %b1 = block {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Location) {
    Func("f", Vector{Param("a", ty.f32(), Vector{Location(2_i)})}, ty.f32(), Vector{Return("a")},
         Vector{Stage(ast::PipelineStage::kFragment)}, Vector{Location(1_i)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%f = @fragment func(%a:f32 [@location(2)]):f32 [@location(1)] -> %b1 {
  %b1 = block {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Location_WithInterpolation_LinearCentroid) {
    Func("f",
         Vector{Param("a", ty.f32(),
                      Vector{Location(2_i), Interpolate(core::InterpolationType::kLinear,
                                                        core::InterpolationSampling::kCentroid)})},
         ty.f32(), Vector{Return("a")}, Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Location(1_i)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(
        Disassemble(m.Get()),
        R"(%f = @fragment func(%a:f32 [@location(2), @interpolate(linear, centroid)]):f32 [@location(1)] -> %b1 {
  %b1 = block {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Location_WithInterpolation_Flat) {
    Func("f",
         Vector{Param("a", ty.f32(),
                      Vector{Location(2_i), Interpolate(core::InterpolationType::kFlat)})},
         ty.f32(), Vector{Return("a")}, Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Location(1_i)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(
        Disassemble(m.Get()),
        R"(%f = @fragment func(%a:f32 [@location(2), @interpolate(flat)]):f32 [@location(1)] -> %b1 {
  %b1 = block {
    ret %a
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
