// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/writer/common/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

TEST_F(SpirvWriterTest, Loop_BreakIf) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true);
            });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranchConditional %true %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

// Test that we still emit the continuing block with a back-edge, even when it is unreachable.
TEST_F(SpirvWriterTest, Loop_UnconditionalBreakInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.ExitLoop(loop);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_ConditionalBreakInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* cond_break = b.If(true);
            b.Append(cond_break->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Append(cond_break->False(), [&] {  //
                b.ExitIf(cond_break);
            });
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.NextIteration(loop);
            });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %10 %9
         %10 = OpLabel
               OpBranch %8
          %9 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_ConditionalContinueInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* cond_break = b.If(true);
            b.Append(cond_break->True(), [&] {  //
                b.Continue(loop);
            });
            b.Append(cond_break->False(), [&] {  //
                b.ExitIf(cond_break);
            });
            b.ExitLoop(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.NextIteration(loop);
            });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %10 %9
         %10 = OpLabel
               OpBranch %6
          %9 = OpLabel
               OpBranch %8
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

// Test that we still emit the continuing block with a back-edge, and the merge block, even when
// they are unreachable.
TEST_F(SpirvWriterTest, Loop_UnconditionalReturnInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_UseResultFromBodyInContinuing) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* result = b.Equal(ty.bool_(), 1_i, 2_i);
            b.Continue(loop, result);

            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, result);
            });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
          %9 = OpIEqual %bool %int_1 %int_2
               OpBranch %6
          %6 = OpLabel
               OpBranchConditional %9 %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_NestedLoopInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
            auto* inner_loop = b.Loop();
            b.Append(inner_loop->Body(), [&] {
                b.ExitLoop(inner_loop);

                b.Append(inner_loop->Continuing(), [&] {  //
                    b.NextIteration(inner_loop);
                });
            });
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(),
                     [&] {  //
                         b.BreakIf(outer_loop, true);
                     });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %12 %10 None
               OpBranch %9
          %9 = OpLabel
               OpBranch %12
         %10 = OpLabel
               OpBranch %11
         %12 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranchConditional %true %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_NestedLoopInContinuing) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(), [&] {
                auto* inner_loop = b.Loop();
                b.Append(inner_loop->Body(), [&] {
                    b.Continue(inner_loop);

                    b.Append(inner_loop->Continuing(), [&] {  //
                        b.BreakIf(inner_loop, true);
                    });
                });
                b.BreakIf(outer_loop, true);
            });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %12 %10 None
               OpBranch %9
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranchConditional %true %12 %11
         %12 = OpLabel
               OpBranchConditional %true %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_SingleValue) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_i);
        });

        auto* loop_param = b.BlockParam(ty.i32());
        loop->Body()->SetParams({loop_param});

        b.Append(loop->Body(), [&] {
            auto* inc = b.Add(ty.i32(), loop_param, 1_i);
            b.Continue(loop, inc);
        });

        auto* cont_param = b.BlockParam(ty.i32());
        loop->Continuing()->SetParams({cont_param});
        b.Append(loop->Continuing(), [&] {
            auto* cmp = b.GreaterThan(ty.bool_(), cont_param, 5_i);
            b.BreakIf(loop, cmp, cont_param);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %int %int_1 %5 %13 %7
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
         %14 = OpIAdd %int %11 %int_1
               OpBranch %7
          %7 = OpLabel
         %13 = OpPhi %int %14 %6
         %15 = OpSGreaterThan %bool %13 %int_5
               OpBranchConditional %15 %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_MultipleValue) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_i, false);
        });

        auto* loop_param_a = b.BlockParam(ty.i32());
        auto* loop_param_b = b.BlockParam(ty.bool_());
        loop->Body()->SetParams({loop_param_a, loop_param_b});

        b.Append(loop->Body(), [&] {
            auto* inc = b.Add(ty.i32(), loop_param_a, 1_i);
            b.Continue(loop, inc, loop_param_b);
        });

        auto* cont_param_a = b.BlockParam(ty.i32());
        auto* cont_param_b = b.BlockParam(ty.bool_());
        loop->Continuing()->SetParams({cont_param_a, cont_param_b});
        b.Append(loop->Continuing(), [&] {
            auto* cmp = b.GreaterThan(ty.bool_(), cont_param_a, 5_i);
            auto* not_b = b.Not(ty.bool_(), cont_param_b);
            b.BreakIf(loop, cmp, cont_param_a, not_b);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %int %int_1 %5 %13 %7
         %15 = OpPhi %bool %false %5 %17 %7
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
         %18 = OpIAdd %int %11 %int_1
               OpBranch %7
          %7 = OpLabel
         %13 = OpPhi %int %18 %6
         %19 = OpPhi %bool %15 %6
         %20 = OpSGreaterThan %bool %13 %int_5
         %17 = OpLogicalEqual %bool %19 %false
               OpBranchConditional %20 %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_NestedIf) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_i);
        });

        auto* loop_param = b.BlockParam(ty.i32());
        loop->Body()->SetParams({loop_param});
        b.Append(loop->Body(), [&] {
            auto* inner = b.If(true);
            inner->SetResults(b.InstructionResult(ty.i32()));
            b.Append(inner->True(), [&] {  //
                b.ExitIf(inner, 10_i);
            });
            b.Append(inner->False(), [&] {  //
                b.ExitIf(inner, 20_i);
            });
            b.Continue(loop, inner->Result());
        });

        auto* cont_param = b.BlockParam(ty.i32());
        loop->Continuing()->SetParams({cont_param});
        b.Append(loop->Continuing(), [&] {
            auto* cmp = b.GreaterThan(ty.bool_(), cont_param, 5_i);
            b.BreakIf(loop, cmp, cont_param);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %int %int_1 %5 %13 %7
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
               OpSelectionMerge %14 None
               OpBranchConditional %true %15 %16
         %15 = OpLabel
               OpBranch %14
         %16 = OpLabel
               OpBranch %14
         %14 = OpLabel
         %19 = OpPhi %int %int_10 %15 %int_20 %16
               OpBranch %7
          %7 = OpLabel
         %13 = OpPhi %int %19 %14
         %22 = OpSGreaterThan %bool %13 %int_5
               OpBranchConditional %22 %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_NestedLoop) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* outer = b.Loop();
        b.Append(outer->Initializer(), [&] {  //
            b.NextIteration(outer, 1_i);
        });

        auto* outer_param = b.BlockParam(ty.i32());
        outer->Body()->SetParams({outer_param});
        b.Append(outer->Body(), [&] {
            auto* inner = b.Loop();
            b.Append(inner->Initializer(), [&] {  //
                b.NextIteration(inner);
            });
            b.Append(inner->Body(), [&] {  //
                b.Continue(inner);
            });
            b.Append(inner->Continuing(), [&] {  //
                b.BreakIf(inner, true);
            });

            b.Continue(outer, outer_param);
        });

        auto* cont_param = b.BlockParam(ty.i32());
        outer->Continuing()->SetParams({cont_param});
        b.Append(outer->Continuing(), [&] {
            auto* cmp = b.GreaterThan(ty.bool_(), cont_param, 5_i);
            b.BreakIf(outer, cmp, cont_param);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %int %int_1 %5 %13 %7
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpLoopMerge %18 %16 None
               OpBranch %15
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %true %18 %17
         %18 = OpLabel
               OpBranch %7
          %7 = OpLabel
         %13 = OpPhi %int %11 %18
         %21 = OpSGreaterThan %bool %13 %int_5
               OpBranchConditional %21 %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
