// Copyright 2022 The Dawn & Tint Authors
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

using IR_BinaryTest = IRTestHelper;
using IR_BinaryDeathTest = IR_BinaryTest;

TEST_F(IR_BinaryDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            Builder b{mod};
            auto bin = b.ir.CreateInstruction<ir::CoreBinary>(
                b.InstructionResult(nullptr), BinaryOp::kAdd, b.Constant(1_u), b.Constant(2_u));
            b.Append(bin);
        },
        "internal compiler error");
}

TEST_F(IR_BinaryTest, Result) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* a = b.Add(l, 2_i);

    EXPECT_TRUE(a->Is<InstructionResult>());
    EXPECT_EQ(a->AsInstruction()->Results().Length(), 1u);
    EXPECT_EQ(a, a->AsInstruction()->Result());
}

TEST_F(IR_BinaryTest, CreateAnd) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.And(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kAnd);
    ASSERT_NE(inst->Result()->Type(), nullptr);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateOr) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.Or(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kOr);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateXor) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.Xor(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kXor);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateEqual) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.Equal(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kEqual);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateNotEqual) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.NotEqual(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kNotEqual);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateLessThan) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.LessThan(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kLessThan);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateGreaterThan) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.GreaterThan(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kGreaterThan);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateLessThanEqual) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.LessThanEqual(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kLessThanEqual);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateGreaterThanEqual) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.GreaterThanEqual(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kGreaterThanEqual);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateShiftLeft) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.ShiftLeft(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kShiftLeft);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateShiftRight) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.ShiftRight(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kShiftRight);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateAdd) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.Add(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kAdd);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateSubtract) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.Subtract(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kSubtract);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateMultiply) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.Multiply(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kMultiply);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateDivide) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.Divide(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kDivide);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, CreateModulo) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.Modulo(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    ASSERT_TRUE(inst->Is<Binary>());
    EXPECT_EQ(inst->Op(), BinaryOp::kModulo);

    ASSERT_EQ(inst->LHS(), l->Result());

    ASSERT_TRUE(inst->RHS()->Is<Constant>());
    auto rhs = inst->RHS()->As<Constant>()->Value();
    ASSERT_TRUE(rhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, rhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BinaryTest, Binary_Usage) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.And(l, 2_i);
    auto* inst = v->AsInstruction<CoreBinary>();

    EXPECT_EQ(inst->Op(), BinaryOp::kAnd);

    ASSERT_NE(inst->LHS(), nullptr);
    EXPECT_THAT(inst->LHS()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 0u}));

    ASSERT_NE(inst->RHS(), nullptr);
    EXPECT_THAT(inst->RHS()->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_BinaryTest, Binary_Usage_DuplicateValue) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* v = b.And(l, l);
    auto* inst = v->AsInstruction<CoreBinary>();

    EXPECT_EQ(inst->Op(), BinaryOp::kAnd);
    ASSERT_EQ(inst->LHS(), inst->RHS());

    ASSERT_NE(inst->LHS(), nullptr);
    EXPECT_THAT(inst->LHS()->UsagesUnsorted(),
                testing::UnorderedElementsAre(Usage{inst, 0u}, Usage{inst, 1u}));
}

TEST_F(IR_BinaryTest, Binary_Usage_SetOperand) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* rhs_a = b.Constant(2_i);
    auto* rhs_b = b.Constant(3_i);
    auto* v = b.And(l, rhs_a);
    auto* inst = v->AsInstruction<CoreBinary>();

    EXPECT_EQ(inst->Op(), BinaryOp::kAnd);

    EXPECT_THAT(rhs_a->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 1u}));
    EXPECT_THAT(rhs_b->UsagesUnsorted(), testing::UnorderedElementsAre());
    inst->SetOperand(1, rhs_b);
    EXPECT_THAT(rhs_a->UsagesUnsorted(), testing::UnorderedElementsAre());
    EXPECT_THAT(rhs_b->UsagesUnsorted(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_BinaryTest, Clone) {
    auto* l = b.Let("l", b.Constant(i32(4)));
    auto* lhs = b.Constant(2_i);
    auto* v = b.And(lhs, l);
    auto* inst = v->AsInstruction<CoreBinary>();

    auto* c = clone_ctx.Clone(inst);

    EXPECT_NE(inst, c);

    EXPECT_EQ(mod.Types().i32(), c->Result()->Type());
    EXPECT_EQ(BinaryOp::kAnd, c->Op());

    auto new_lhs = c->LHS()->As<Constant>()->Value();
    ASSERT_TRUE(new_lhs->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(2_i, new_lhs->As<core::constant::Scalar<i32>>()->ValueAs<i32>());

    auto new_rhs = c->RHS();
    ASSERT_EQ(new_rhs, l->Result());
}

TEST_F(IR_BinaryTest, Fold) {
    auto* v = b.Add(4_u, 2_u);

    ASSERT_TRUE(v->Is<Constant>());
    ASSERT_EQ(v->As<Constant>()->Value()->ValueAs<uint32_t>(), 6u);
}

}  // namespace
}  // namespace tint::core::ir
