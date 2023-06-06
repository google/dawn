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

using IR_LoadTest = IRTestHelper;

TEST_F(IR_LoadTest, Create) {
    auto* store_type = mod.Types().i32();
    auto* var = b.Declare(mod.Types().pointer(store_type, builtin::AddressSpace::kFunction,
                                              builtin::Access::kReadWrite));
    const auto* inst = b.Load(var);

    ASSERT_TRUE(inst->Is<Load>());
    ASSERT_EQ(inst->From(), var);

    EXPECT_EQ(inst->Type(), store_type);

    ASSERT_TRUE(inst->From()->Is<ir::Var>());
    EXPECT_EQ(inst->From(), var);
}

TEST_F(IR_LoadTest, Usage) {
    auto* store_type = mod.Types().i32();
    auto* var = b.Declare(mod.Types().pointer(store_type, builtin::AddressSpace::kFunction,
                                              builtin::Access::kReadWrite));
    auto* inst = b.Load(var);

    ASSERT_NE(inst->From(), nullptr);
    EXPECT_THAT(inst->From()->Usages(), testing::UnorderedElementsAre(Usage{inst, 0u}));
}

TEST_F(IR_LoadTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* store_type = mod.Types().i32();
            auto* var = b.Declare(mod.Types().pointer(store_type, builtin::AddressSpace::kFunction,
                                                      builtin::Access::kReadWrite));
            Load l(nullptr, var);
        },
        "");
}

TEST_F(IR_LoadTest, Fail_NonPtr_Builder) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Load(b.Declare(mod.Types().f32()));
        },
        "");
}

TEST_F(IR_LoadTest, Fail_NullValue_Builder) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Load(nullptr);
        },
        "");
}

TEST_F(IR_LoadTest, Fail_NullValue) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            Load l(mod.Types().f32(), nullptr);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
