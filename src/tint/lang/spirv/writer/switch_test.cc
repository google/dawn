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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

TEST_F(SpirvWriterTest, Switch_Basic) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* def_case = b.Case(swtch, Vector{ir::Switch::CaseSelector()});
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %int_42 %5
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_MultipleCases) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* case_a = b.Case(swtch, Vector{ir::Switch::CaseSelector{b.Constant(1_i)}});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(swtch);
        });

        auto* case_b = b.Case(swtch, Vector{ir::Switch::CaseSelector{b.Constant(2_i)}});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(swtch);
        });

        auto* def_case = b.Case(swtch, Vector{ir::Switch::CaseSelector()});
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %int_42 %5 1 %8 2 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_MultipleSelectorsPerCase) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* case_a = b.Case(swtch, Vector{ir::Switch::CaseSelector{b.Constant(1_i)},
                                            ir::Switch::CaseSelector{b.Constant(3_i)}});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(swtch);
        });

        auto* case_b = b.Case(swtch, Vector{ir::Switch::CaseSelector{b.Constant(2_i)},
                                            ir::Switch::CaseSelector{b.Constant(4_i)}});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(swtch);
        });

        auto* def_case = b.Case(
            swtch, Vector{ir::Switch::CaseSelector{b.Constant(5_i)}, ir::Switch::CaseSelector()});
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %int_42 %5 1 %8 3 %8 2 %9 4 %9 5 %5
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_AllCasesReturn) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* case_a = b.Case(swtch, Vector{ir::Switch::CaseSelector{b.Constant(1_i)}});
        b.Append(case_a, [&] {  //
            b.Return(func);
        });

        auto* case_b = b.Case(swtch, Vector{ir::Switch::CaseSelector{b.Constant(2_i)}});
        b.Append(case_b, [&] {  //
            b.Return(func);
        });

        auto* def_case = b.Case(swtch, Vector{ir::Switch::CaseSelector()});
        b.Append(def_case, [&] {  //
            b.Return(func);
        });

        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %int_42 %5 1 %8 2 %9
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_ConditionalBreak) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* case_a = b.Case(swtch, Vector{ir::Switch::CaseSelector{b.Constant(1_i)}});
        b.Append(case_a, [&] {
            auto* cond_break = b.If(true);
            b.Append(cond_break->True(), [&] {  //
                b.ExitSwitch(swtch);
            });
            b.Append(cond_break->False(), [&] {  //
                b.ExitIf(cond_break);
            });

            b.Return(func);
        });

        auto* def_case = b.Case(swtch, Vector{ir::Switch::CaseSelector()});
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %9 None
               OpSwitch %int_42 %5 1 %8
          %8 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %true %11 %10
         %11 = OpLabel
               OpBranch %9
         %10 = OpLabel
               OpBranch %9
          %5 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_SingleValue) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(42_i);
        s->SetResults(b.InstructionResult(ty.i32()));
        auto* case_a = b.Case(s, Vector{ir::Switch::CaseSelector{b.Constant(1_i)},
                                        ir::Switch::CaseSelector{nullptr}});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(s, 10_i);
        });

        auto* case_b = b.Case(s, Vector{ir::Switch::CaseSelector{b.Constant(2_i)}});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i);
        });

        b.Return(func, s);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %int_42 %5 1 %5 2 %7
          %5 = OpLabel
               OpBranch %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
          %9 = OpPhi %int %int_10 %5 %int_20 %7
               OpReturnValue %9
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_SingleValue_CaseReturn) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(42_i);
        s->SetResults(b.InstructionResult(ty.i32()));
        auto* case_a = b.Case(s, Vector{ir::Switch::CaseSelector{b.Constant(1_i)},
                                        ir::Switch::CaseSelector{nullptr}});
        b.Append(case_a, [&] {  //
            b.Return(func, 10_i);
        });

        auto* case_b = b.Case(s, Vector{ir::Switch::CaseSelector{b.Constant(2_i)}});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i);
        });

        b.Return(func, s);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
%return_value = OpVariable %_ptr_Function_int Function
%continue_execution = OpVariable %_ptr_Function_bool Function
               OpStore %continue_execution %true
               OpSelectionMerge %14 None
               OpSwitch %int_42 %11 1 %11 2 %13
         %11 = OpLabel
               OpStore %continue_execution %false
               OpStore %return_value %int_10
               OpBranch %14
         %13 = OpLabel
               OpBranch %14
         %14 = OpLabel
         %17 = OpPhi %int %18 %11 %int_20 %13
         %20 = OpLoad %bool %continue_execution
               OpSelectionMerge %21 None
               OpBranchConditional %20 %22 %21
         %22 = OpLabel
               OpStore %return_value %17
               OpBranch %21
         %21 = OpLabel
         %23 = OpLoad %int %return_value
               OpReturnValue %23
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_MultipleValue_0) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(42_i);
        s->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        auto* case_a = b.Case(s, Vector{ir::Switch::CaseSelector{b.Constant(1_i)},
                                        ir::Switch::CaseSelector{nullptr}});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(s, 10_i, true);
        });

        auto* case_b = b.Case(s, Vector{ir::Switch::CaseSelector{b.Constant(2_i)}});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i, false);
        });

        b.Return(func, s->Result(0));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %int_42 %5 1 %5 2 %7
          %5 = OpLabel
               OpBranch %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
          %9 = OpPhi %int %int_10 %5 %int_20 %7
         %13 = OpPhi %bool %true %5 %false %7
               OpReturnValue %9
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_MultipleValue_1) {
    auto* func = b.Function("foo", ty.bool_());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(b.Constant(42_i));
        s->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        auto* case_a = b.Case(s, Vector{ir::Switch::CaseSelector{b.Constant(1_i)},
                                        ir::Switch::CaseSelector{nullptr}});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(s, 10_i, true);
        });

        auto* case_b = b.Case(s, Vector{ir::Switch::CaseSelector{b.Constant(2_i)}});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i, false);
        });

        b.Return(func, s->Result(1));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %9 None
               OpSwitch %int_42 %5 1 %5 2 %8
          %5 = OpLabel
               OpBranch %9
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %10 = OpPhi %int %int_10 %5 %int_20 %8
         %13 = OpPhi %bool %true %5 %false %8
               OpReturnValue %13
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
