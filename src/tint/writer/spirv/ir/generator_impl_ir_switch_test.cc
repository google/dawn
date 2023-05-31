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

TEST_F(SpvGeneratorImplTest, Switch_Basic) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* swtch = b.CreateSwitch(b.Constant(42_i));

    auto* def_case = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector()});
    def_case->AddInstruction(b.ExitSwitch(swtch));

    swtch->Merge()->AddInstruction(b.Return(func));

    func->StartTarget()->AddInstruction(swtch);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpConstant %7 42
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %8 None
OpSwitch %6 %5
%5 = OpLabel
OpBranch %8
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Switch_MultipleCases) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* swtch = b.CreateSwitch(b.Constant(42_i));

    auto* case_a = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector{b.Constant(1_i)}});
    case_a->AddInstruction(b.ExitSwitch(swtch));

    auto* case_b = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector{b.Constant(2_i)}});
    case_b->AddInstruction(b.ExitSwitch(swtch));

    auto* def_case = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector()});
    def_case->AddInstruction(b.ExitSwitch(swtch));

    swtch->Merge()->AddInstruction(b.Return(func));

    func->StartTarget()->AddInstruction(swtch);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpConstant %7 42
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %10 None
OpSwitch %6 %5 1 %8 2 %9
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

TEST_F(SpvGeneratorImplTest, Switch_MultipleSelectorsPerCase) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* swtch = b.CreateSwitch(b.Constant(42_i));

    auto* case_a = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector{b.Constant(1_i)},
                                                     ir::Switch::CaseSelector{b.Constant(3_i)}});
    case_a->AddInstruction(b.ExitSwitch(swtch));

    auto* case_b = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector{b.Constant(2_i)},
                                                     ir::Switch::CaseSelector{b.Constant(4_i)}});
    case_b->AddInstruction(b.ExitSwitch(swtch));

    auto* def_case = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector{b.Constant(5_i)},
                                                       ir::Switch::CaseSelector()});
    def_case->AddInstruction(b.ExitSwitch(swtch));

    swtch->Merge()->AddInstruction(b.Return(func));

    func->StartTarget()->AddInstruction(swtch);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpConstant %7 42
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %10 None
OpSwitch %6 %5 1 %8 3 %8 2 %9 4 %9 5 %5
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

TEST_F(SpvGeneratorImplTest, Switch_AllCasesReturn) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* swtch = b.CreateSwitch(b.Constant(42_i));

    auto* case_a = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector{b.Constant(1_i)}});
    case_a->AddInstruction(b.Return(func));

    auto* case_b = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector{b.Constant(2_i)}});
    case_b->AddInstruction(b.Return(func));

    auto* def_case = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector()});
    def_case->AddInstruction(b.Return(func));

    func->StartTarget()->AddInstruction(swtch);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpConstant %7 42
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %10 None
OpSwitch %6 %5 1 %8 2 %9
%8 = OpLabel
OpReturn
%9 = OpLabel
OpReturn
%5 = OpLabel
OpReturn
%10 = OpLabel
OpUnreachable
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Switch_ConditionalBreak) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* swtch = b.CreateSwitch(b.Constant(42_i));

    auto* cond_break = b.CreateIf(b.Constant(true));
    cond_break->True()->AddInstruction(b.ExitSwitch(swtch));
    cond_break->False()->AddInstruction(b.ExitIf(cond_break));
    cond_break->Merge()->AddInstruction(b.Return(func));

    auto* case_a = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector{b.Constant(1_i)}});
    case_a->AddInstruction(cond_break);

    auto* def_case = b.CreateCase(swtch, utils::Vector{ir::Switch::CaseSelector()});
    def_case->AddInstruction(b.ExitSwitch(swtch));

    swtch->Merge()->AddInstruction(b.Return(func));

    func->StartTarget()->AddInstruction(swtch);

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpConstant %7 42
%13 = OpTypeBool
%12 = OpConstantTrue %13
%1 = OpFunction %2 None %3
%4 = OpLabel
OpSelectionMerge %9 None
OpSwitch %6 %5 1 %8
%8 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %12 %11 %10
%11 = OpLabel
OpBranch %9
%10 = OpLabel
OpReturn
%5 = OpLabel
OpBranch %9
%9 = OpLabel
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
