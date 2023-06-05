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

#include "src/tint/ir/block.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ir/block_param.h"
#include "src/tint/ir/ir_test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_BlockTest = IRTestHelper;

TEST_F(IR_BlockTest, HasBranchTarget_Empty) {
    auto* blk = b.CreateBlock();
    EXPECT_FALSE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_NoBranch) {
    auto* blk = b.CreateBlock();
    blk->Append(b.Add(mod.Types().i32(), b.Constant(1_u), b.Constant(2_u)));
    EXPECT_FALSE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_BreakIf) {
    auto* blk = b.CreateBlock();
    auto* loop = b.CreateLoop();
    blk->Append(b.BreakIf(b.Constant(true), loop));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_Continue) {
    auto* blk = b.CreateBlock();
    auto* loop = b.CreateLoop();
    blk->Append(b.Continue(loop));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_ExitIf) {
    auto* blk = b.CreateBlock();
    auto* if_ = b.CreateIf(b.Constant(true));
    blk->Append(b.ExitIf(if_));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_ExitLoop) {
    auto* blk = b.CreateBlock();
    auto* loop = b.CreateLoop();
    blk->Append(b.ExitLoop(loop));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_ExitSwitch) {
    auto* blk = b.CreateBlock();
    auto* s = b.CreateSwitch(b.Constant(1_u));
    blk->Append(b.ExitSwitch(s));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_If) {
    auto* blk = b.CreateBlock();
    blk->Append(b.CreateIf(b.Constant(true)));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_Loop) {
    auto* blk = b.CreateBlock();
    blk->Append(b.CreateLoop());
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_NextIteration) {
    auto* blk = b.CreateBlock();
    auto* loop = b.CreateLoop();
    blk->Append(b.NextIteration(loop));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_Return) {
    auto* f = b.CreateFunction("myFunc", mod.Types().void_());

    auto* blk = b.CreateBlock();
    blk->Append(b.Return(f));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, HasBranchTarget_Switch) {
    auto* blk = b.CreateBlock();
    blk->Append(b.CreateSwitch(b.Constant(true)));
    EXPECT_TRUE(blk->HasBranchTarget());
}

TEST_F(IR_BlockTest, SetInstructions) {
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst3 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst1, inst2, inst3});

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

TEST_F(IR_BlockTest, Append) {
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst3 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->Append(inst1);
    blk->Append(inst2);
    blk->Append(inst3);

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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst3 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->Prepend(inst3);
    blk->Prepend(inst2);
    blk->Prepend(inst1);

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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();

    auto* blk = b.CreateBlock();
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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst3 = b.CreateLoop();

    auto* blk = b.CreateBlock();
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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();

    auto* blk = b.CreateBlock();
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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst3 = b.CreateLoop();

    auto* blk = b.CreateBlock();
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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst3 = b.CreateLoop();
    auto* inst4 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst1, inst4, inst3});
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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst4 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst4, inst2});
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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst4 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst1, inst4});
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
    auto* inst1 = b.CreateLoop();
    auto* inst4 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst4});
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
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* inst4 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst1, inst4, inst2});
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
    auto* inst1 = b.CreateLoop();
    auto* inst4 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst4, inst1});
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
    auto* inst1 = b.CreateLoop();
    auto* inst4 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst1, inst4});
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
    auto* inst4 = b.CreateLoop();

    auto* blk = b.CreateBlock();
    blk->SetInstructions(utils::Vector{inst4});
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

            auto* blk = b.CreateBlock();
            blk->Prepend(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_PrependAlreadyInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
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

            auto* blk = b.CreateBlock();
            blk->Append(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_AppendAlreadyInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
            blk->InsertBefore(nullptr, inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertBeforeInstNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            auto* blk1 = b.CreateBlock();
            auto* blk2 = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            auto* blk1 = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
            blk->InsertAfter(nullptr, inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_InsertAfterInstNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            auto* blk1 = b.CreateBlock();
            auto* blk2 = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            auto* blk1 = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
            blk->Replace(nullptr, inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_ReplaceInstNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            auto* blk1 = b.CreateBlock();
            auto* blk2 = b.CreateBlock();
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

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            auto* blk1 = b.CreateBlock();
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

            auto* blk = b.CreateBlock();
            blk->Remove(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_RemoveDifferentBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* blk1 = b.CreateBlock();
            auto* blk2 = b.CreateBlock();
            blk2->Append(inst1);
            blk1->Remove(inst1);
        },
        "internal compiler error");
}

TEST_F(IR_BlockTest, Fail_NullBlockParam) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* blk = b.CreateBlock();
            blk->SetParams(utils::Vector<const BlockParam*, 1>{nullptr});
        },
        "");
}

TEST_F(IR_BlockTest, Fail_NullInboundBranch) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* blk = b.CreateBlock();
            blk->AddInboundBranch(nullptr);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
