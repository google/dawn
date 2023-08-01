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

#include "src/tint/lang/core/ir/loop.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_LoopTest = IRTestHelper;

TEST_F(IR_LoopTest, Parent) {
    auto* loop = b.Loop();
    EXPECT_EQ(loop->Initializer()->Parent(), loop);
    EXPECT_EQ(loop->Body()->Parent(), loop);
    EXPECT_EQ(loop->Continuing()->Parent(), loop);
}

TEST_F(IR_LoopTest, Result) {
    auto* loop = b.Loop();
    EXPECT_FALSE(loop->HasResults());
    EXPECT_FALSE(loop->HasMultiResults());
}

TEST_F(IR_LoopTest, Fail_NullInitializerBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            Loop loop(nullptr, b.MultiInBlock(), b.MultiInBlock());
        },
        "");
}

TEST_F(IR_LoopTest, Fail_NullBodyBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            Loop loop(b.Block(), nullptr, b.MultiInBlock());
        },
        "");
}

TEST_F(IR_LoopTest, Fail_NullContinuingBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            Loop loop(b.Block(), b.MultiInBlock(), nullptr);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
