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
#include "src/tint/ir/builder.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/ir_test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_StoreTest = IRTestHelper;

TEST_F(IR_StoreTest, CreateStore) {
    auto* to = b.Var(mod.Types().ptr(builtin::AddressSpace::kPrivate, mod.Types().i32(),
                                     builtin::Access::kReadWrite));
    auto* inst = b.Store(to, 4_i);

    ASSERT_TRUE(inst->Is<Store>());
    ASSERT_EQ(inst->To(), to);

    ASSERT_TRUE(inst->From()->Is<Constant>());
    auto lhs = inst->From()->As<Constant>()->Value();
    ASSERT_TRUE(lhs->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, lhs->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_StoreTest, Store_Usage) {
    auto* to = b.Discard();
    auto* inst = b.Store(to, 4_i);

    ASSERT_NE(inst->To(), nullptr);
    EXPECT_THAT(inst->To()->Usages(), testing::UnorderedElementsAre(Usage{inst, 0u}));

    ASSERT_NE(inst->From(), nullptr);
    EXPECT_THAT(inst->From()->Usages(), testing::UnorderedElementsAre(Usage{inst, 1u}));
}

TEST_F(IR_StoreTest, Fail_NullTo) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Store(nullptr, 1_u);
        },
        "");
}

TEST_F(IR_StoreTest, Fail_NullFrom) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* to = b.Var(mod.Types().ptr(builtin::AddressSpace::kPrivate, mod.Types().i32(),
                                             builtin::Access::kReadWrite));
            b.Store(to, nullptr);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
