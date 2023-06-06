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

#include "src/tint/ir/access.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ir/ir_test_helper.h"

namespace tint::ir {
namespace {

using IR_AccessTest = IRTestHelper;

TEST_F(IR_AccessTest, SetsUsage) {
    auto* ty = mod.Types().pointer(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                   builtin::Access::kReadWrite);
    auto* var = b.Declare(ty);
    auto* idx = b.Constant(u32(1));
    auto* a = b.Access(mod.Types().i32(), var, utils::Vector{idx});

    EXPECT_THAT(var->Usages(), testing::UnorderedElementsAre(Usage{a, 0u}));
    EXPECT_THAT(idx->Usages(), testing::UnorderedElementsAre(Usage{a, 1u}));
}

TEST_F(IR_AccessTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* ty = mod.Types().pointer(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                           builtin::Access::kReadWrite);
            auto* var = b.Declare(ty);
            b.Access(nullptr, var, utils::Vector{b.Constant(u32(1))});
        },
        "");
}

TEST_F(IR_AccessTest, Fail_NullObject) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Access(mod.Types().i32(), nullptr, utils::Vector{b.Constant(u32(1))});
        },
        "");
}

TEST_F(IR_AccessTest, Fail_EmptyIndices) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* ty = mod.Types().pointer(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                           builtin::Access::kReadWrite);
            auto* var = b.Declare(ty);
            b.Access(mod.Types().i32(), var, utils::Empty);
        },
        "");
}

TEST_F(IR_AccessTest, Fail_NullIndex) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* ty = mod.Types().pointer(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                           builtin::Access::kReadWrite);
            auto* var = b.Declare(ty);
            b.Access(mod.Types().i32(), var, utils::Vector<Value*, 1>{nullptr});
        },
        "");
}

}  // namespace
}  // namespace tint::ir
