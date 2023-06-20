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

#include "src/tint/writer/spirv/ir/test_helper_ir.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, If_TrueEmpty_FalseEmpty) {
    auto* func = b.Function("foo", ty.void_());

    auto* i = b.If(true);
    i->True()->Append(b.ExitIf(i));
    i->False()->Append(b.ExitIf(i));
    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Return(func));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeBool
%6 = OpConstantTrue %7
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %6 %5 %5
%5 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_FalseEmpty) {
    auto* func = b.Function("foo", ty.void_());

    auto* i = b.If(true);
    i->False()->Append(b.ExitIf(i));

    auto tb = b.With(i->True());
    tb.Add(ty.i32(), 1_i, 1_i);
    tb.ExitIf(i);

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Return(func));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeBool
%7 = OpConstantTrue %8
%10 = OpTypeInt 32 1
%11 = OpConstant %10 1
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %7 %6 %5
%6 = OpLabel
%9 = OpIAdd %10 %11 %11
OpBranch %5
%5 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_TrueEmpty) {
    auto* func = b.Function("foo", ty.void_());

    auto* i = b.If(true);
    i->True()->Append(b.ExitIf(i));

    auto fb = b.With(i->False());
    fb.Add(ty.i32(), 1_i, 1_i);
    fb.ExitIf(i);

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Return(func));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%8 = OpTypeBool
%7 = OpConstantTrue %8
%10 = OpTypeInt 32 1
%11 = OpConstant %10 1
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %7 %5 %6
%6 = OpLabel
%9 = OpIAdd %10 %11 %11
OpBranch %5
%5 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_BothBranchesReturn) {
    auto* func = b.Function("foo", ty.void_());

    auto* i = b.If(true);
    i->True()->Append(b.Return(func));
    i->False()->Append(b.Return(func));

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Unreachable());

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpReturn
%7 = OpLabel
OpReturn
%5 = OpLabel
OpUnreachable
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_Phi_SingleValue) {
    auto* func = b.Function("foo", ty.i32());

    auto* i = b.If(true);
    i->SetResults(b.InstructionResult(ty.i32()));
    i->True()->Append(b.ExitIf(i, 10_i));
    i->False()->Append(b.ExitIf(i, 20_i));

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Return(func, i));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeInt 32 1
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%11 = OpConstant %2 10
%12 = OpConstant %2 20
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpBranch %5
%7 = OpLabel
OpBranch %5
%5 = OpLabel
%10 = OpPhi %2 %11 %6 %12 %7
OpReturnValue %10
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_Phi_SingleValue_TrueReturn) {
    auto* func = b.Function("foo", ty.i32());

    auto* i = b.If(true);
    i->SetResults(b.InstructionResult(ty.i32()));
    i->True()->Append(b.Return(func, 42_i));
    i->False()->Append(b.ExitIf(i, 20_i));

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Return(func, i));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeInt 32 1
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%10 = OpConstant %2 42
%12 = OpConstant %2 20
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpReturnValue %10
%7 = OpLabel
OpBranch %5
%5 = OpLabel
%11 = OpPhi %2 %12 %7
OpReturnValue %11
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_Phi_SingleValue_FalseReturn) {
    auto* func = b.Function("foo", ty.i32());

    auto* i = b.If(true);
    i->SetResults(b.InstructionResult(ty.i32()));
    i->True()->Append(b.ExitIf(i, 10_i));
    i->False()->Append(b.Return(func, 42_i));

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Return(func, i));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeInt 32 1
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%10 = OpConstant %2 42
%12 = OpConstant %2 10
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpBranch %5
%7 = OpLabel
OpReturnValue %10
%5 = OpLabel
%11 = OpPhi %2 %12 %6
OpReturnValue %11
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_Phi_MultipleValue_0) {
    auto* func = b.Function("foo", ty.i32());

    auto* i = b.If(true);
    i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
    i->True()->Append(b.ExitIf(i, 10_i, true));
    i->False()->Append(b.ExitIf(i, 20_i, false));

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Return(func, i->Result(0)));

    ASSERT_TRUE(IRIsValid()) << Error();

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeInt 32 1
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%11 = OpConstant %2 10
%12 = OpConstant %2 20
%14 = OpConstantFalse %9
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpBranch %5
%7 = OpLabel
OpBranch %5
%5 = OpLabel
%10 = OpPhi %2 %11 %6 %12 %7
%13 = OpPhi %9 %8 %6 %14 %7
OpReturnValue %10
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_Phi_MultipleValue_1) {
    auto* func = b.Function("foo", ty.bool_());

    auto* i = b.If(true);
    i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
    i->True()->Append(b.ExitIf(i, 10_i, true));
    i->False()->Append(b.ExitIf(i, 20_i, false));

    func->StartTarget()->Append(i);
    func->StartTarget()->Append(b.Return(func, i->Result(1)));

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeBool
%3 = OpTypeFunction %2
%8 = OpConstantTrue %2
%9 = OpTypeInt 32 1
%11 = OpConstant %9 10
%12 = OpConstant %9 20
%14 = OpConstantFalse %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpBranch %5
%7 = OpLabel
OpBranch %5
%5 = OpLabel
%10 = OpPhi %9 %11 %6 %12 %7
%13 = OpPhi %2 %8 %6 %14 %7
OpReturnValue %13
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
