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
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;        // NOLINT
using namespace tint::builtin::fluent_types;  // NOLINT

using IR_OperandInstructionTest = IRTestHelper;

TEST_F(IR_OperandInstructionTest, Destroy) {
    auto* block = b.Block();
    auto* inst = b.Add(ty.i32(), 1_i, 2_i);
    block->Append(inst);
    auto* lhs = inst->LHS();
    auto* rhs = inst->RHS();
    EXPECT_EQ(inst->Block(), block);
    EXPECT_THAT(lhs->Usages(), testing::ElementsAre(Usage{inst, 0u}));
    EXPECT_THAT(rhs->Usages(), testing::ElementsAre(Usage{inst, 1u}));
    EXPECT_TRUE(inst->Result()->Alive());

    inst->Destroy();

    EXPECT_EQ(inst->Block(), nullptr);
    EXPECT_TRUE(lhs->Usages().IsEmpty());
    EXPECT_TRUE(rhs->Usages().IsEmpty());
    EXPECT_FALSE(inst->Result()->Alive());
}

TEST_F(IR_OperandInstructionTest, ClearOperands_WithNullOperand) {
    auto* block = b.Block();
    // The var initializer is a nullptr
    auto* inst = b.Var(ty.ptr<private_, f32>());
    block->Append(inst);

    inst->Destroy();
    EXPECT_EQ(inst->Block(), nullptr);
    EXPECT_FALSE(inst->Result()->Alive());
}

TEST_F(IR_OperandInstructionTest, SetOperands_WithNullOperand) {
    auto* inst = b.Var(ty.ptr<private_, f32>());
    Vector<Value*, 1> ops;
    ops.Push(nullptr);

    inst->SetOperands(ops);
}

}  // namespace
}  // namespace tint::ir
