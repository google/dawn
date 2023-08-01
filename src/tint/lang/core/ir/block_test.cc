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

#include "src/tint/lang/core/ir/block.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_BlockTest = IRTestHelper;

TEST_F(IR_BlockTest, HasTerminator_Empty) {
    auto* blk = b.Block();
    EXPECT_FALSE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, HasTerminator_None) {
    auto* blk = b.Block();
    blk->Append(b.Add(mod.Types().i32(), 1_u, 2_u));
    EXPECT_FALSE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, HasTerminator_BreakIf) {
    auto* blk = b.Block();
    auto* loop = b.Loop();
    blk->Append(b.BreakIf(loop, true));
    EXPECT_TRUE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, HasTerminator_Continue) {
    auto* blk = b.Block();
    auto* loop = b.Loop();
    blk->Append(b.Continue(loop));
    EXPECT_TRUE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, HasTerminator_ExitIf) {
    auto* blk = b.Block();
    auto* if_ = b.If(true);
    blk->Append(b.ExitIf(if_));
    EXPECT_TRUE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, HasTerminator_ExitLoop) {
    auto* blk = b.Block();
    auto* loop = b.Loop();
    blk->Append(b.ExitLoop(loop));
    EXPECT_TRUE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, HasTerminator_ExitSwitch) {
    auto* blk = b.Block();
    auto* s = b.Switch(1_u);
    blk->Append(b.ExitSwitch(s));
    EXPECT_TRUE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, HasTerminator_NextIteration) {
    auto* blk = b.Block();
    auto* loop = b.Loop();
    blk->Append(b.NextIteration(loop));
    EXPECT_TRUE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, HasTerminator_Return) {
    auto* f = b.Function("myFunc", mod.Types().void_());

    auto* blk = b.Block();
    blk->Append(b.Return(f));
    EXPECT_TRUE(blk->HasTerminator());
}

TEST_F(IR_BlockTest, Append) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* inst3 = b.Loop();

    auto* blk = b.Block();
    EXPECT_EQ(blk->Append(inst1), inst1);
    EXPECT_EQ(blk->Append(inst2), inst2);
    EXPECT_EQ(blk->Append(inst3), inst3);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Prepend) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* inst3 = b.Loop();

    auto* blk = b.Block();
    EXPECT_EQ(blk->Prepend(inst3), inst3);
    EXPECT_EQ(blk->Prepend(inst2), inst2);
    EXPECT_EQ(blk->Prepend(inst1), inst1);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, InsertBefore_AtStart) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();

    auto* blk = b.Block();
    blk->Append(inst2);
    blk->InsertBefore(inst2, inst1);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, InsertBefore_Middle) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* inst3 = b.Loop();

    auto* blk = b.Block();
    blk->Append(inst1);
    blk->Append(inst3);
    blk->InsertBefore(inst3, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, InsertAfter_AtEnd) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();

    auto* blk = b.Block();
    blk->Append(inst1);
    blk->InsertAfter(inst1, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, InsertAfter_Middle) {
    auto* inst1 = b.Loop();
    auto* inst2 = b.Loop();
    auto* inst3 = b.Loop();

    auto* blk = b.Block();
    blk->Append(inst1);
    blk->Append(inst3);
    blk->InsertAfter(inst1, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Replace_Middle) {
    auto* blk = b.Block();
    auto* inst1 = blk->Append(b.Loop());
    auto* inst4 = blk->Append(b.Loop());
    auto* inst3 = blk->Append(b.Loop());

    auto* inst2 = b.Loop();
    blk->Replace(inst4, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    ASSERT_EQ(inst3->Block(), blk);
    EXPECT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(3u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    inst = inst->next;

    ASSERT_EQ(inst, inst3);
    ASSERT_EQ(inst->prev, inst2);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Replace_Start) {
    auto* blk = b.Block();
    auto* inst4 = blk->Append(b.Loop());
    auto* inst2 = blk->Append(b.Loop());

    auto* inst1 = b.Loop();
    blk->Replace(inst4, inst1);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    EXPECT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Replace_End) {
    auto* blk = b.Block();
    auto* inst1 = blk->Append(b.Loop());
    auto* inst4 = blk->Append(b.Loop());

    auto* inst2 = b.Loop();
    blk->Replace(inst4, inst2);

    ASSERT_EQ(inst1->Block(), blk);
    ASSERT_EQ(inst2->Block(), blk);
    EXPECT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Replace_OnlyNode) {
    auto* blk = b.Block();
    auto* inst4 = blk->Append(b.Loop());

    auto* inst1 = b.Loop();
    blk->Replace(inst4, inst1);

    ASSERT_EQ(inst1->Block(), blk);
    EXPECT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(1u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Remove_Middle) {
    auto* blk = b.Block();
    auto* inst1 = blk->Append(b.Loop());
    auto* inst4 = blk->Append(b.Loop());
    auto* inst2 = blk->Append(b.Loop());
    blk->Remove(inst4);

    ASSERT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(2u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    inst = inst->next;

    ASSERT_EQ(inst, inst2);
    ASSERT_EQ(inst->prev, inst1);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Remove_Start) {
    auto* blk = b.Block();
    auto* inst4 = blk->Append(b.Loop());
    auto* inst1 = blk->Append(b.Loop());
    blk->Remove(inst4);

    ASSERT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(1u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Remove_End) {
    auto* blk = b.Block();
    auto* inst1 = blk->Append(b.Loop());
    auto* inst4 = blk->Append(b.Loop());
    blk->Remove(inst4);

    ASSERT_EQ(inst4->Block(), nullptr);

    EXPECT_FALSE(blk->IsEmpty());
    EXPECT_EQ(1u, blk->Length());

    auto* inst = blk->Instructions();
    ASSERT_EQ(inst, inst1);
    ASSERT_EQ(inst->prev, nullptr);
    ASSERT_EQ(inst->next, nullptr);
}

TEST_F(IR_BlockTest, Remove_OnlyNode) {
    auto* blk = b.Block();
    auto* inst4 = blk->Append(b.Loop());
    blk->Remove(inst4);

    ASSERT_EQ(inst4->Block(), nullptr);

    EXPECT_TRUE(blk->IsEmpty());
    EXPECT_EQ(0u, blk->Length());
}

TEST_F(IR_BlockTest, Fail_PrependNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* blk = b.Block();
            blk->Prepend(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_PrependAlreadyInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->Prepend(inst1);

            blk->Prepend(inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_AppendNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* blk = b.Block();
            blk->Append(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_AppendAlreadyInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->Append(inst1);
            blk->Append(inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertBeforeNullptrInst) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->InsertBefore(nullptr, inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertBeforeInstNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->Append(inst1);
            blk->InsertBefore(inst1, nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertBeforeDifferentBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            auto* blk1 = b.Block();
            auto* blk2 = b.Block();
            blk2->Append(inst1);
            blk1->InsertBefore(inst1, inst2);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertBeforeAlreadyInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            auto* blk1 = b.Block();
            blk1->Append(inst1);
            blk1->Append(inst2);
            blk1->InsertBefore(inst1, inst2);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertAfterNullptrInst) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->InsertAfter(nullptr, inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertAfterInstNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->Append(inst1);
            blk->InsertAfter(inst1, nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertAfterDifferentBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            auto* blk1 = b.Block();
            auto* blk2 = b.Block();
            blk2->Append(inst1);
            blk1->InsertAfter(inst1, inst2);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertAfterAlreadyInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            auto* blk1 = b.Block();
            blk1->Append(inst1);
            blk1->Append(inst2);
            blk1->InsertAfter(inst1, inst2);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_ReplaceNullptrInst) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->Replace(nullptr, inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_ReplaceInstNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk = b.Block();
            blk->Append(inst1);
            blk->Replace(inst1, nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_ReplaceDifferentBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            auto* blk1 = b.Block();
            auto* blk2 = b.Block();
            blk2->Append(inst1);
            blk1->Replace(inst1, inst2);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_ReplaceAlreadyInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* inst2 = b.Loop();
            auto* blk1 = b.Block();
            blk1->Append(inst1);
            blk1->Append(inst2);
            blk1->Replace(inst1, inst2);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_RemoveNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* blk = b.Block();
            blk->Remove(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_RemoveDifferentBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.Loop();
            auto* blk1 = b.Block();
            auto* blk2 = b.Block();
            blk2->Append(inst1);
            blk1->Remove(inst1);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ir
