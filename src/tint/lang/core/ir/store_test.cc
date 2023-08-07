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
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

using IR_StoreTest = IRTestHelper;

TEST_F(IR_StoreTest, CreateStore) {
    auto* to = b.Var(ty.ptr<private_, i32>());
    auto* inst = b.Store(to, 4_i);

    ASSERT_TRUE(inst->Is<Store>());
    ASSERT_EQ(inst->To(), to->Result());

    ASSERT_TRUE(inst->From()->Is<Constant>());
    auto lhs = inst->From()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_StoreTest, Usage) {
    auto* to = b.Var(ty.ptr<private_, i32>());
    auto* inst = b.Store(to, 4_i);

    ASSERT_NE(inst->To(), nullptr);
    EXPECT_THAT(inst->To()->Usages(), testing::UnorderedElementsAre(Usage{inst, 0u}));

    ASSERT_NE(inst->From(), nullptr);
    EXPECT_THAT(inst->From()->Usages(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_StoreTest, Result) {
    auto* to = b.Var(ty.ptr<private_, i32>());
    auto* inst = b.Store(to, 4_i);

    EXPECT_FALSE(inst->HasResults());
    EXPECT_FALSE(inst->HasMultiResults());
}

}  // namespace
}  // namespace tint::ir
