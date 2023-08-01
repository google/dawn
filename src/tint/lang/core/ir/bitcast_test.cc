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
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_BitcastTest = IRTestHelper;

TEST_F(IR_BitcastTest, Bitcast) {
    auto* inst = b.Bitcast(mod.Types().i32(), 4_i);

    ASSERT_TRUE(inst->Is<ir::Bitcast>());
    ASSERT_NE(inst->Result()->Type(), nullptr);

    auto args = inst->Args();
    ASSERT_EQ(args.Length(), 1u);
    ASSERT_TRUE(args[0]->Is<Constant>());
    auto val = args[0]->As<Constant>()->Value();
    ASSERT_TRUE(val->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, val->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_BitcastTest, Result) {
    auto* a = b.Bitcast(mod.Types().i32(), 4_i);

    EXPECT_TRUE(a->HasResults());
    EXPECT_FALSE(a->HasMultiResults());
    EXPECT_TRUE(a->Result()->Is<InstructionResult>());
    EXPECT_EQ(a, a->Result()->Source());
}

TEST_F(IR_BitcastTest, Bitcast_Usage) {
    auto* inst = b.Bitcast(mod.Types().i32(), 4_i);

    auto args = inst->Args();
    ASSERT_EQ(args.Length(), 1u);
    ASSERT_NE(args[0], nullptr);
    EXPECT_THAT(args[0]->Usages(), testing::UnorderedElementsAre(Usage{inst, 0u}));
}

TEST_F(IR_BitcastTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Bitcast(nullptr, 1_i);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
