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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

TEST_F(SpirvWriterTest, If_TrueEmpty_FalseEmpty) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {  //
            b.ExitIf(i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %5 %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_FalseEmpty) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {
            b.Add(ty.i32(), 1_i, 1_i);
            b.ExitIf(i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %5
          %6 = OpLabel
          %9 = OpIAdd %int %int_1 %int_1
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_TrueEmpty) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {  //
            b.ExitIf(i);
        });
        b.Append(i->False(), [&] {
            b.Add(ty.i32(), 1_i, 1_i);
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %5 %6
          %6 = OpLabel
          %9 = OpIAdd %int %int_1 %int_1
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_BothBranchesReturn) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {  //
            b.Return(func);
        });
        b.Append(i->False(), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %5 %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_SingleValue) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i);
        });
        b.Return(func, i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %7
          %6 = OpLabel
               OpBranch %5
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
         %10 = OpPhi %int %int_10 %6 %int_20 %7
               OpReturnValue %10
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_SingleValue_TrueReturn) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.Return(func, 42_i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i);
        });
        b.Return(func, i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%18 = OpUndef %int");
    EXPECT_INST(R"(
               OpSelectionMerge %12 None
               OpBranchConditional %true %13 %14
         %13 = OpLabel
               OpStore %continue_execution %false
               OpStore %return_value %int_42
               OpBranch %12
         %14 = OpLabel
               OpBranch %12
         %12 = OpLabel
         %17 = OpPhi %int %18 %13 %int_20 %14
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

TEST_F(SpirvWriterTest, If_Phi_SingleValue_FalseReturn) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i);
        });
        b.Append(i->False(), [&] {  //
            b.Return(func, 42_i);
        });
        b.Return(func, i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%19 = OpUndef %int");
    EXPECT_INST(R"(
               OpSelectionMerge %12 None
               OpBranchConditional %true %13 %14
         %13 = OpLabel
               OpBranch %12
         %14 = OpLabel
               OpStore %continue_execution %false
               OpStore %return_value %int_42
               OpBranch %12
         %12 = OpLabel
         %17 = OpPhi %int %int_10 %13 %19 %14
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

TEST_F(SpirvWriterTest, If_Phi_MultipleValue_0) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func, i->Result(0));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %7
          %6 = OpLabel
               OpBranch %5
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
         %10 = OpPhi %int %int_10 %6 %int_20 %7
         %13 = OpPhi %bool %true %6 %false %7
               OpReturnValue %10
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_MultipleValue_1) {
    auto* func = b.Function("foo", ty.bool_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func, i->Result(1));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %7
          %6 = OpLabel
               OpBranch %5
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
         %10 = OpPhi %int %int_10 %6 %int_20 %7
         %13 = OpPhi %bool %true %6 %false %7
               OpReturnValue %13
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
