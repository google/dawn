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

#include "src/tint/lang/core/ir/construct.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_ConstructTest = IRTestHelper;

TEST_F(IR_ConstructTest, Usage) {
    auto* arg1 = b.Constant(true);
    auto* arg2 = b.Constant(false);
    auto* c = b.Construct(mod.Types().f32(), arg1, arg2);

    EXPECT_THAT(arg1->Usages(), testing::UnorderedElementsAre(Usage{c, 0u}));
    EXPECT_THAT(arg2->Usages(), testing::UnorderedElementsAre(Usage{c, 1u}));
}

TEST_F(IR_ConstructTest, Result) {
    auto* arg1 = b.Constant(true);
    auto* arg2 = b.Constant(false);
    auto* c = b.Construct(mod.Types().f32(), arg1, arg2);

    EXPECT_TRUE(c->HasResults());
    EXPECT_FALSE(c->HasMultiResults());
    EXPECT_TRUE(c->Result()->Is<InstructionResult>());
    EXPECT_EQ(c, c->Result()->Source());
}

TEST_F(IR_ConstructTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Construct(nullptr);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
