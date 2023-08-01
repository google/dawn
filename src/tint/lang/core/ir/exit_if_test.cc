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

#include "src/tint/lang/core/ir/exit_if.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_ExitIfTest = IRTestHelper;

TEST_F(IR_ExitIfTest, Usage) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* if_ = b.If(true);
    auto* e = b.ExitIf(if_, arg1, arg2);

    EXPECT_THAT(arg1->Usages(), testing::UnorderedElementsAre(Usage{e, 0u}));
    EXPECT_THAT(arg2->Usages(), testing::UnorderedElementsAre(Usage{e, 1u}));
    EXPECT_EQ(if_->Result(), nullptr);
}

TEST_F(IR_ExitIfTest, Result) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* if_ = b.If(true);
    auto* e = b.ExitIf(if_, arg1, arg2);

    EXPECT_FALSE(e->HasResults());
    EXPECT_FALSE(e->HasMultiResults());
}

TEST_F(IR_ExitIfTest, Destroy) {
    auto* if_ = b.If(true);
    auto* exit = b.ExitIf(if_);
    EXPECT_THAT(if_->Exits(), testing::UnorderedElementsAre(exit));
    exit->Destroy();
    EXPECT_TRUE(if_->Exits().IsEmpty());
    EXPECT_FALSE(exit->Alive());
}

}  // namespace
}  // namespace tint::ir
