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
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(utils::Vector{b.ExitIf(i)});
    i->False()->SetInstructions(utils::Vector{b.ExitIf(i)});
    i->Merge()->SetInstructions(utils::Vector{b.Return(func)});

    func->StartTarget()->SetInstructions(utils::Vector{i});

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
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* i = b.CreateIf(b.Constant(true));
    i->False()->SetInstructions(utils::Vector{b.ExitIf(i)});
    i->Merge()->SetInstructions(utils::Vector{b.Return(func)});

    auto* true_block = i->True();
    true_block->SetInstructions(
        utils::Vector{b.Add(mod.Types().i32(), b.Constant(1_i), b.Constant(1_i)), b.ExitIf(i)});

    func->StartTarget()->SetInstructions(utils::Vector{i});

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
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(utils::Vector{b.ExitIf(i)});
    i->Merge()->SetInstructions(utils::Vector{b.Return(func)});

    auto* false_block = i->False();
    false_block->SetInstructions(
        utils::Vector{b.Add(mod.Types().i32(), b.Constant(1_i), b.Constant(1_i)), b.ExitIf(i)});

    func->StartTarget()->SetInstructions(utils::Vector{i});

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
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(utils::Vector{b.Return(func)});
    i->False()->SetInstructions(utils::Vector{b.Return(func)});

    func->StartTarget()->SetInstructions(utils::Vector{i});

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
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* merge_param = b.BlockParam(b.ir.Types().i32());

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(utils::Vector{b.ExitIf(i, utils::Vector{b.Constant(10_i)})});
    i->False()->SetInstructions(utils::Vector{b.ExitIf(i, utils::Vector{b.Constant(20_i)})});
    i->Merge()->SetParams(utils::Vector{merge_param});
    i->Merge()->SetInstructions(utils::Vector{b.Return(func, utils::Vector{merge_param})});

    func->StartTarget()->SetInstructions(utils::Vector{i});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%11 = OpTypeInt 32 1
%12 = OpConstant %11 10
%13 = OpConstant %11 20
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpBranch %5
%7 = OpLabel
OpBranch %5
%5 = OpLabel
%10 = OpPhi %11 %12 %6 %13 %7
OpReturnValue %10
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_Phi_SingleValue_TrueReturn) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* merge_param = b.BlockParam(b.ir.Types().i32());

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(utils::Vector{b.Return(func, utils::Vector{b.Constant(42_i)})});
    i->False()->SetInstructions(utils::Vector{b.ExitIf(i, utils::Vector{b.Constant(20_i)})});
    i->Merge()->SetParams(utils::Vector{merge_param});
    i->Merge()->SetInstructions(utils::Vector{b.Return(func, utils::Vector{merge_param})});

    func->StartTarget()->SetInstructions(utils::Vector{i});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%11 = OpTypeInt 32 1
%10 = OpConstant %11 42
%13 = OpConstant %11 20
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpReturnValue %10
%7 = OpLabel
OpBranch %5
%5 = OpLabel
%12 = OpPhi %11 %13 %7
OpReturnValue %12
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_Phi_SingleValue_FalseReturn) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* merge_param = b.BlockParam(b.ir.Types().i32());

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(utils::Vector{b.ExitIf(i, utils::Vector{b.Constant(10_i)})});
    i->False()->SetInstructions(utils::Vector{b.Return(func, utils::Vector{b.Constant(42_i)})});
    i->Merge()->SetParams(utils::Vector{merge_param});
    i->Merge()->SetInstructions(utils::Vector{b.Return(func, utils::Vector{merge_param})});

    func->StartTarget()->SetInstructions(utils::Vector{i});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%11 = OpTypeInt 32 1
%10 = OpConstant %11 42
%13 = OpConstant %11 10
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpBranch %5
%7 = OpLabel
OpReturnValue %10
%5 = OpLabel
%12 = OpPhi %11 %13 %6
OpReturnValue %12
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, If_Phi_MultipleValue) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* merge_param_0 = b.BlockParam(b.ir.Types().i32());
    auto* merge_param_1 = b.BlockParam(b.ir.Types().bool_());

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(
        utils::Vector{b.ExitIf(i, utils::Vector{b.Constant(10_i), b.Constant(true)})});
    i->False()->SetInstructions(
        utils::Vector{b.ExitIf(i, utils::Vector{b.Constant(20_i), b.Constant(false)})});
    i->Merge()->SetParams(utils::Vector{merge_param_0, merge_param_1});
    i->Merge()->SetInstructions(utils::Vector{
        b.Return(func, utils::Vector{merge_param_0}),
    });

    func->StartTarget()->SetInstructions(utils::Vector{i});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%11 = OpTypeInt 32 1
%12 = OpConstant %11 10
%13 = OpConstant %11 20
%15 = OpConstantFalse %9
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpBranch %5
%7 = OpLabel
OpBranch %5
%5 = OpLabel
%10 = OpPhi %11 %12 %6 %13 %7
%14 = OpPhi %9 %8 %6 %15 %7
OpReturnValue %10
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
