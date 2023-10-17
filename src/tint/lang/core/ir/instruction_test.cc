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

#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/module.h"

namespace tint::core::ir {
namespace {

using IR_InstructionTest = IRTestHelper;

TEST_F(IR_InstructionTest, InsertBefore) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* blk = b.Block();
    blk->Append(inst2);
    inst1->InsertBefore(inst2);
    EXPECT_EQ(2u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
}

TEST_F(IR_InstructionTest, Fail_InsertBeforeNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            inst1->InsertBefore(nullptr);
        },
        "");
}

TEST_F(IR_InstructionTest, Fail_InsertBeforeNotInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            inst1->InsertBefore(inst2);
        },
        "");
}

TEST_F(IR_InstructionTest, InsertAfter) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* blk = b.Block();
    blk->Append(inst2);
    inst1->InsertAfter(inst2);
    EXPECT_EQ(2u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
}

TEST_F(IR_InstructionTest, Fail_InsertAfterNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            inst1->InsertAfter(nullptr);
        },
        "");
}

TEST_F(IR_InstructionTest, Fail_InsertAfterNotInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            inst1->InsertAfter(inst2);
        },
        "");
}

TEST_F(IR_InstructionTest, ReplaceWith) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* blk = b.Block();
    blk->Append(inst2);
    inst2->ReplaceWith(inst1);
    EXPECT_EQ(1u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
    EXPECT_EQ(inst2->Block(), nullptr);
}

TEST_F(IR_InstructionTest, Fail_ReplaceWithNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->Append(inst1);
            inst1->ReplaceWith(nullptr);
        },
        "");
}

TEST_F(IR_InstructionTest, Fail_ReplaceWithNotInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            inst1->ReplaceWith(inst2);
        },
        "");
}

TEST_F(IR_InstructionTest, Remove) {
    auto* inst1 = b.Loop();
    auto* blk = b.Block();
    blk->Append(inst1);
    EXPECT_EQ(1u, blk->Length());

    inst1->Remove();
    EXPECT_EQ(0u, blk->Length());
    EXPECT_EQ(inst1->Block(), nullptr);
}

TEST_F(IR_InstructionTest, Fail_RemoveNotInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            inst1->Remove();
        },
        "");
}

}  // namespace
}  // namespace tint::core::ir
