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

#include "src/tint/lang/spirv/writer/common/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

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
            b.NextIteration(loop, 1_i, false);
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

}  // namespace
}  // namespace tint::spirv::writer
