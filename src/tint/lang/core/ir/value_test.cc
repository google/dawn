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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir {
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
}  // namespace tint::core::ir
