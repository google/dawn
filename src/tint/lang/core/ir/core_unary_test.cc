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
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir {
namespace {

using IR_UnaryTest = IRTestHelper;
using IR_UnaryDeathTest = IR_UnaryTest;

TEST_F(IR_UnaryTest, CreateComplement) {
    auto* l = b.Let("l", 4_i);
    auto* inst = b.Complement(l)->AsInstruction<CoreUnary>();

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->Op(), UnaryOp::kComplement);

    ASSERT_NE(nullptr, inst->Val()->AsInstruction<core::ir::Let>());
    ASSERT_EQ(l->Result(), inst->Val());
}

TEST_F(IR_UnaryTest, CreateNegation) {
    auto* l = b.Let("l", 4_i);
    auto* inst = b.Negation(l)->AsInstruction<CoreUnary>();

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->Op(), UnaryOp::kNegation);

    ASSERT_NE(nullptr, inst->Val()->AsInstruction<core::ir::Let>());
    ASSERT_EQ(l->Result(), inst->Val());
}

TEST_F(IR_UnaryTest, CreateNot) {
    auto* l = b.Let("l", true);
    auto* inst = b.Not(l)->AsInstruction<CoreUnary>();

    ASSERT_TRUE(inst->Is<Unary>());
    EXPECT_EQ(inst->Op(), UnaryOp::kNot);

    ASSERT_NE(nullptr, inst->Val()->AsInstruction<core::ir::Let>());
    ASSERT_EQ(l->Result(), inst->Val());
}

TEST_F(IR_UnaryTest, Usage) {
    auto* l = b.Let("l", 4_i);
    auto* inst = b.Negation(l)->AsInstruction<CoreUnary>();

    EXPECT_EQ(inst->Op(), UnaryOp::kNegation);

    ASSERT_NE(inst->Val(), nullptr);
    EXPECT_THAT(inst->Val()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 0u}));
}

TEST_F(IR_UnaryTest, Result) {
    auto* l = b.Let("l", 4_i);
    auto* inst = b.Negation(l)->AsInstruction<CoreUnary>();
    EXPECT_EQ(inst->Results().Length(), 1u);
    EXPECT_TRUE(inst->Result()->Is<InstructionResult>());
    EXPECT_EQ(inst->Result()->Instruction(), inst);
}

TEST_F(IR_UnaryTest, Clone) {
    auto* l = b.Let("l", 4_i);
    auto* inst = b.Complement(l)->AsInstruction<CoreUnary>();
    auto* new_inst = clone_ctx.Clone(inst);

    EXPECT_NE(inst, new_inst);
    EXPECT_NE(nullptr, new_inst->Result());
    EXPECT_NE(inst->Result(), new_inst->Result());

    EXPECT_EQ(UnaryOp::kComplement, new_inst->Op());

    auto* new_val = new_inst->As<CoreUnary>()->Val();
    ASSERT_NE(nullptr, new_val->AsInstruction<core::ir::Let>());
    EXPECT_EQ(l->Result(), new_val);
}

}  // namespace
}  // namespace tint::core::ir
