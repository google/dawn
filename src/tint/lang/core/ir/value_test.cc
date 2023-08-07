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
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::number_suffixes;     // NOLINT
using namespace tint::core::fluent_types;  // NOLINT

namespace tint::ir {
namespace {

using IR_ValueTest = IRTestHelper;

TEST_F(IR_ValueTest, ReplaceAllUsesWith_Value) {
    auto* val_old = b.InstructionResult(ty.i32());
    auto* val_new = b.InstructionResult(ty.i32());
    auto* inst = b.Add(mod.Types().i32(), val_old, 1_i);
    EXPECT_EQ(inst->LHS(), val_old);
    val_old->ReplaceAllUsesWith(val_new);
    EXPECT_EQ(inst->LHS(), val_new);
}

TEST_F(IR_ValueTest, ReplaceAllUsesWith_Fn) {
    auto* val_old = b.InstructionResult(ty.i32());
    auto* val_new = b.InstructionResult(ty.i32());
    auto* inst = b.Add(mod.Types().i32(), val_old, 1_i);
    EXPECT_EQ(inst->LHS(), val_old);
    val_old->ReplaceAllUsesWith([&](Usage use) {
        EXPECT_EQ(use.instruction, inst);
        EXPECT_EQ(use.operand_index, 0u);
        return val_new;
    });
    EXPECT_EQ(inst->LHS(), val_new);
}

TEST_F(IR_ValueTest, Destroy) {
    auto* val = b.InstructionResult(ty.i32());
    EXPECT_TRUE(val->Alive());
    val->Destroy();
    EXPECT_FALSE(val->Alive());
}

TEST_F(IR_ValueTest, Destroy_HasUsage) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* val = b.InstructionResult(mod.Types().i32());
            b.Add(mod.Types().i32(), val, 1_i);
            val->Destroy();
        },
        "");
}

TEST_F(IR_ValueTest, Destroy_HasSource) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* val = b.Add(mod.Types().i32(), 1_i, 2_i)->Result();
            val->Destroy();
        },
        "");
}

}  // namespace
}  // namespace tint::ir
