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
    EXPECT_INST("%17 = OpUndef %int");
    EXPECT_INST(R"(
               OpSelectionMerge %11 None
               OpBranchConditional %true %12 %13
         %12 = OpLabel
               OpStore %continue_execution %false
               OpStore %return_value %int_42
               OpBranch %11
         %13 = OpLabel
               OpBranch %11
         %11 = OpLabel
         %16 = OpPhi %int %17 %12 %int_20 %13
         %19 = OpLoad %bool %continue_execution
               OpSelectionMerge %20 None
               OpBranchConditional %19 %21 %20
         %21 = OpLabel
               OpStore %return_value %16
               OpBranch %20
         %20 = OpLabel
         %22 = OpLoad %int %return_value
               OpReturnValue %22
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
    EXPECT_INST("%18 = OpUndef %int");
    EXPECT_INST(R"(
               OpSelectionMerge %11 None
               OpBranchConditional %true %12 %13
         %12 = OpLabel
               OpBranch %11
         %13 = OpLabel
               OpStore %continue_execution %false
               OpStore %return_value %int_42
               OpBranch %11
         %11 = OpLabel
         %16 = OpPhi %int %int_10 %12 %18 %13
         %19 = OpLoad %bool %continue_execution
               OpSelectionMerge %20 None
               OpBranchConditional %19 %21 %20
         %21 = OpLabel
               OpStore %return_value %16
               OpBranch %20
         %20 = OpLabel
         %22 = OpLoad %int %return_value
               OpReturnValue %22
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
