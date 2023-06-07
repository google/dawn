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
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/program_test_helper.h"
#include "src/tint/ir/switch.h"

namespace tint::ir {
namespace {

/// Looks for the flow node with the given type T.
/// If no flow node is found, then nullptr is returned.
/// If multiple flow nodes are found with the type T, then an error is raised and the first is
/// returned.
template <typename T>
T* FindSingleValue(Module& mod) {
    T* found = nullptr;
    size_t count = 0;
    for (auto* node : mod.values.Objects()) {
        if (auto* as = node->As<T>()) {
            count++;
            if (!found) {
                found = as;
            }
        }
    }
    if (count > 1) {
        ADD_FAILURE() << "FindSingleValue() found " << count << " nodes of type "
                      << utils::TypeInfo::Of<T>().name;
    }
    return found;
}

using namespace tint::number_suffixes;  // NOLINT

using IR_FromProgramTest = ProgramTestHelper;

TEST_F(IR_FromProgramTest, Func) {
    Func("f", utils::Empty, ty.void_(), utils::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->StartTarget(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%f = func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam) {
    Func("f", utils::Vector{Param("a", ty.u32())}, ty.u32(), utils::Vector{Return("a")});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->StartTarget(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%f = func(%a:u32):u32 -> %b1 {
  %b1 = block {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithMultipleParam) {
    Func("f", utils::Vector{Param("a", ty.u32()), Param("b", ty.i32()), Param("c", ty.bool_())},
         ty.void_(), utils::Empty);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    ASSERT_EQ(1u, m->functions.Length());

    auto* f = m->functions[0];
    ASSERT_NE(f->StartTarget(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), Function::PipelineStage::kUndefined);

    EXPECT_EQ(Disassemble(m.Get()), R"(%f = func(%a:u32, %b:i32, %c:bool):void -> %b1 {
  %b1 = block {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, EntryPoint) {
    Func("f", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(m->functions[0]->Stage(), Function::PipelineStage::kFragment);
}

TEST_F(IR_FromProgramTest, IfStatement) {
    auto* ast_if = If(true, Block(), Else(Block()));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* flow = FindSingleValue<ir::If>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->False()->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3, m: %b4]
      # True block
      %b2 = block {
        exit_if %b4
      }

      # False block
      %b3 = block {
        exit_if %b4
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* flow = FindSingleValue<ir::If>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->False()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3, m: %b4]
      # True block
      %b2 = block {
        ret
      }

      # False block
      %b3 = block {
        exit_if %b4
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* flow = FindSingleValue<ir::If>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->False()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3, m: %b4]
      # True block
      %b2 = block {
        exit_if %b4
      }

      # False block
      %b3 = block {
        ret
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* flow = FindSingleValue<ir::If>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->False()->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3]
      # True block
      %b2 = block {
        ret
      }

      # False block
      %b3 = block {
        ret
      }

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
    auto* if_flow = FindSingleValue<ir::If>(m);
    ASSERT_NE(if_flow, nullptr);

    auto* loop_flow = FindSingleValue<ir::Loop>(m);
    ASSERT_NE(loop_flow, nullptr);

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    if true [t: %b2, f: %b3, m: %b4]
      # True block
      %b2 = block {
        loop [b: %b5, m: %b6]
          # Body block
          %b5 = block {
            exit_loop %b6
          }

        # Merge block
        %b6 = block {
          exit_if %b4
        }

      }

      # False block
      %b3 = block {
        exit_if %b4
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* flow = FindSingleValue<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, flow->Body()->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, m: %b3]
      # Body block
      %b2 = block {
        exit_loop %b3
      }

    # Merge block
    %b3 = block {
      ret
    }

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
    auto* loop_flow = FindSingleValue<ir::Loop>(m);

    auto* if_flow = FindSingleValue<ir::If>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(2u, loop_flow->Body()->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Merge()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3, m: %b4]
      # Body block
      %b2 = block {
        if true [t: %b5, f: %b6, m: %b7]
          # True block
          %b5 = block {
            exit_loop %b4
          }

          # False block
          %b6 = block {
            exit_if %b7
          }

        # Merge block
        %b7 = block {
          continue %b3
        }

      }

      # Continuing block
      %b3 = block {
        next_iteration %b2
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* loop_flow = FindSingleValue<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(2u, loop_flow->Body()->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3, m: %b4]
      # Body block
      %b2 = block {
        continue %b3
      }

      # Continuing block
      %b3 = block {
        break_if true %b2
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    loop [b: %b2, c: %b3, m: %b4]
      # Body block
      %b2 = block {
        continue %b3
      }

      # Continuing block
      %b3 = block {
        break_if true %b2
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* loop_flow = FindSingleValue<ir::Loop>(m);
    auto* if_flow = FindSingleValue<ir::If>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(2u, loop_flow->Body()->InboundBranches().Length());
    EXPECT_EQ(1u, loop_flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Merge()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3]
      # Body block
      %b2 = block {
        if true [t: %b4, f: %b5, m: %b6]
          # True block
          %b4 = block {
            ret
          }

          # False block
          %b5 = block {
            exit_if %b6
          }

        # Merge block
        %b6 = block {
          continue %b3
        }

      }

      # Continuing block
      %b3 = block {
        next_iteration %b2
      }

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
    auto* loop_flow = FindSingleValue<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop_flow->Body()->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2]
      # Body block
      %b2 = block {
        ret
      }

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
    auto* loop_flow = FindSingleValue<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop_flow->Body()->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2]
      # Body block
      %b2 = block {
        ret
      }

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
    auto* loop_flow = FindSingleValue<ir::Loop>(m);
    auto* if_flow = FindSingleValue<ir::If>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop_flow->Body()->InboundBranches().Length());
    EXPECT_EQ(0u, loop_flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(2u, loop_flow->Merge()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False()->InboundBranches().Length());
    EXPECT_EQ(0u, if_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, m: %b3]
      # Body block
      %b2 = block {
        if true [t: %b4, f: %b5]
          # True block
          %b4 = block {
            exit_loop %b3
          }

          # False block
          %b5 = block {
            exit_loop %b3
          }

      }

    # Merge block
    %b3 = block {
      ret
    }

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
    loop [b: %b2, c: %b3, m: %b4]
      # Body block
      %b2 = block {
        loop [b: %b5, c: %b6, m: %b7]
          # Body block
          %b5 = block {
            if true [t: %b8, f: %b9, m: %b10]
              # True block
              %b8 = block {
                exit_loop %b7
              }

              # False block
              %b9 = block {
                exit_if %b10
              }

            # Merge block
            %b10 = block {
              if true [t: %b11, f: %b12, m: %b13]
                # True block
                %b11 = block {
                  continue %b6
                }

                # False block
                %b12 = block {
                  exit_if %b13
                }

              # Merge block
              %b13 = block {
                continue %b6
              }

            }

          }

          # Continuing block
          %b6 = block {
            loop [b: %b14, m: %b15]
              # Body block
              %b14 = block {
                exit_loop %b15
              }

            # Merge block
            %b15 = block {
              loop [b: %b16, c: %b17, m: %b18]
                # Body block
                %b16 = block {
                  continue %b17
                }

                # Continuing block
                %b17 = block {
                  break_if true %b16
                }

              # Merge block
              %b18 = block {
                next_iteration %b5
              }

            }

          }

        # Merge block
        %b7 = block {
          if true [t: %b19, f: %b20, m: %b21]
            # True block
            %b19 = block {
              exit_loop %b4
            }

            # False block
            %b20 = block {
              exit_if %b21
            }

          # Merge block
          %b21 = block {
            continue %b3
          }

        }

      }

      # Continuing block
      %b3 = block {
        next_iteration %b2
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* flow = FindSingleValue<ir::Loop>(m);

    ASSERT_NE(flow->Body()->Branch(), nullptr);
    ASSERT_TRUE(flow->Body()->Branch()->Is<ir::If>());
    auto* if_flow = flow->Body()->Branch()->As<ir::If>();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(2u, flow->Body()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3, m: %b4]
      # Body block
      %b2 = block {
        if false [t: %b5, f: %b6, m: %b7]
          # True block
          %b5 = block {
            exit_if %b7
          }

          # False block
          %b6 = block {
            exit_loop %b4
          }

        # Merge block
        %b7 = block {
          continue %b3
        }

      }

      # Continuing block
      %b3 = block {
        next_iteration %b2
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* flow = FindSingleValue<ir::Loop>(m);

    ASSERT_NE(flow->Body()->Branch(), nullptr);
    ASSERT_TRUE(flow->Body()->Branch()->Is<ir::If>());
    auto* if_flow = flow->Body()->Branch()->As<ir::If>();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(2u, flow->Body()->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, c: %b3, m: %b4]
      # Body block
      %b2 = block {
        if true [t: %b5, f: %b6, m: %b7]
          # True block
          %b5 = block {
            exit_if %b7
          }

          # False block
          %b6 = block {
            exit_loop %b4
          }

        # Merge block
        %b7 = block {
          ret
        }

      }

      # Continuing block
      %b3 = block {
        next_iteration %b2
      }

    # Merge block
    %b4 = block {
      ret
    }

  }
}
)");
}

// TODO(dsinclair): Enable when variable declarations and increment are supported
TEST_F(IR_FromProgramTest, DISABLED_For) {
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

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* flow = FindSingleValue<ir::Loop>(m);

    ASSERT_NE(flow->Body()->Branch(), nullptr);
    ASSERT_TRUE(flow->Body()->Branch()->Is<ir::If>());
    auto* if_flow = flow->Body()->Branch()->As<ir::If>();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(2u, flow->Body()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Merge()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->True()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->False()->InboundBranches().Length());
    EXPECT_EQ(1u, if_flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m), R"()");
}

TEST_F(IR_FromProgramTest, For_Init_NoCondOrContinuing) {
    auto* ast_for = For(Decl(Var("i", ty.i32())), nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* flow = FindSingleValue<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, flow->Initializer()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Body()->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3, m: %b4]
      # Initializer block
      %b2 = block {
        %i:ptr<function, i32, read_write> = var
        next_iteration %b3
      }

      # Body block
      %b3 = block {
        exit_loop %b4
      }

    # Merge block
    %b4 = block {
      ret
    }

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
    auto* flow = FindSingleValue<ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, flow->Body()->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Continuing()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    loop [b: %b2, m: %b3]
      # Body block
      %b2 = block {
        exit_loop %b3
      }

    # Merge block
    %b3 = block {
      ret
    }

  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch) {
    auto* ast_switch = Switch(
        1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)}, Block()),
                           Case(utils::Vector{CaseSelector(1_i)}, Block()), DefaultCase(Block())});

    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* flow = FindSingleValue<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

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

    EXPECT_EQ(1u, cases[0].Start()->InboundBranches().Length());
    EXPECT_EQ(1u, cases[1].Start()->InboundBranches().Length());
    EXPECT_EQ(1u, cases[2].Start()->InboundBranches().Length());
    EXPECT_EQ(3u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (0i, %b2), c: (1i, %b3), c: (default, %b4), m: %b5]
      # Case block
      %b2 = block {
        exit_switch %b5
      }

      # Case block
      %b3 = block {
        exit_switch %b5
      }

      # Case block
      %b4 = block {
        exit_switch %b5
      }

    # Merge block
    %b5 = block {
      ret
    }

  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_MultiSelector) {
    auto* ast_switch = Switch(
        1_i,
        utils::Vector{Case(
            utils::Vector{CaseSelector(0_i), CaseSelector(1_i), DefaultCaseSelector()}, Block())});

    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* flow = FindSingleValue<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

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

    EXPECT_EQ(1u, cases[0].Start()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (0i 1i default, %b2), m: %b3]
      # Case block
      %b2 = block {
        exit_switch %b3
      }

    # Merge block
    %b3 = block {
      ret
    }

  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_OnlyDefault) {
    auto* ast_switch = Switch(1_i, utils::Vector{DefaultCase(Block())});
    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* flow = FindSingleValue<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto cases = flow->Cases();
    ASSERT_EQ(1u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    EXPECT_TRUE(cases[0].selectors[0].IsDefault());

    EXPECT_EQ(1u, cases[0].Start()->InboundBranches().Length());
    EXPECT_EQ(1u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (default, %b2), m: %b3]
      # Case block
      %b2 = block {
        exit_switch %b3
      }

    # Merge block
    %b3 = block {
      ret
    }

  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_WithBreak) {
    auto* ast_switch = Switch(1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)},
                                                      Block(Break(), If(true, Block(Return())))),
                                                 DefaultCase(Block())});
    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    auto* flow = FindSingleValue<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto cases = flow->Cases();
    ASSERT_EQ(2u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    EXPECT_TRUE(cases[1].selectors[0].IsDefault());

    EXPECT_EQ(1u, cases[0].Start()->InboundBranches().Length());
    EXPECT_EQ(1u, cases[1].Start()->InboundBranches().Length());
    EXPECT_EQ(2u, flow->Merge()->InboundBranches().Length());
    // This is 1 because the if is dead-code eliminated and the return doesn't happen.

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (0i, %b2), c: (default, %b3), m: %b4]
      # Case block
      %b2 = block {
        exit_switch %b4
      }

      # Case block
      %b3 = block {
        exit_switch %b4
      }

    # Merge block
    %b4 = block {
      ret
    }

  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_AllReturn) {
    auto* ast_switch =
        Switch(1_i, utils::Vector{Case(utils::Vector{CaseSelector(0_i)}, Block(Return())),
                                  DefaultCase(Block(Return()))});
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_switch, ast_if);

    auto res = Build();
    ASSERT_TRUE(res) << (!res ? res.Failure() : "");

    auto m = res.Move();
    ASSERT_EQ(FindSingleValue<ir::If>(m), nullptr);

    auto* flow = FindSingleValue<ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto cases = flow->Cases();
    ASSERT_EQ(2u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<constant::Scalar<tint::i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<constant::Scalar<tint::i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    EXPECT_TRUE(cases[1].selectors[0].IsDefault());

    EXPECT_EQ(1u, cases[0].Start()->InboundBranches().Length());
    EXPECT_EQ(1u, cases[1].Start()->InboundBranches().Length());
    EXPECT_EQ(0u, flow->Merge()->InboundBranches().Length());

    EXPECT_EQ(Disassemble(m),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    switch 1i [c: (0i, %b2), c: (default, %b3)]
      # Case block
      %b2 = block {
        ret
      }

      # Case block
      %b3 = block {
        ret
      }

  }
}
)");
}

TEST_F(IR_FromProgramTest, Emit_Phony) {
    Func("b", utils::Empty, ty.i32(), Return(1_i));
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
    Func(
        "f",
        utils::Vector{Param("a", ty.vec4<f32>(),
                            utils::Vector{Invariant(), Builtin(builtin::BuiltinValue::kPosition)})},
        ty.vec4<f32>(), utils::Vector{Return("a")},
        utils::Vector{Stage(ast::PipelineStage::kFragment)}, utils::Vector{Location(1_i)});
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
    Func("f", utils::Vector{Param("a", ty.f32(), utils::Vector{Location(2_i)})}, ty.f32(),
         utils::Vector{Return("a")}, utils::Vector{Stage(ast::PipelineStage::kFragment)},
         utils::Vector{Location(1_i)});

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
         utils::Vector{Param(
             "a", ty.f32(),
             utils::Vector{Location(2_i), Interpolate(builtin::InterpolationType::kLinear,
                                                      builtin::InterpolationSampling::kCentroid)})},
         ty.f32(), utils::Vector{Return("a")}, utils::Vector{Stage(ast::PipelineStage::kFragment)},
         utils::Vector{Location(1_i)});

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
         utils::Vector{
             Param("a", ty.f32(),
                   utils::Vector{Location(2_i), Interpolate(builtin::InterpolationType::kFlat)})},
         ty.f32(), utils::Vector{Return("a")}, utils::Vector{Stage(ast::PipelineStage::kFragment)},
         utils::Vector{Location(1_i)});

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
}  // namespace tint::ir
