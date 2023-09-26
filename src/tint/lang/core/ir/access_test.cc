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

#include "src/tint/lang/core/ir/access.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir {
namespace {

using IR_AccessTest = IRTestHelper;

TEST_F(IR_AccessTest, SetsUsage) {
    auto* type = ty.ptr<function, i32>();
    auto* var = b.Var(type);
    auto* idx = b.Constant(u32(1));
    auto* a = b.Access(ty.i32(), var, idx);

    EXPECT_THAT(var->Result()->Usages(), testing::UnorderedElementsAre(Usage{a, 0u}));
    EXPECT_THAT(idx->Usages(), testing::UnorderedElementsAre(Usage{a, 1u}));
}

TEST_F(IR_AccessTest, Result) {
    auto* type = ty.ptr<function, i32>();
    auto* var = b.Var(type);
    auto* idx = b.Constant(u32(1));
    auto* a = b.Access(ty.i32(), var, idx);

    EXPECT_TRUE(a->HasResults());
    EXPECT_FALSE(a->HasMultiResults());

    EXPECT_TRUE(a->Result()->Is<InstructionResult>());
    EXPECT_EQ(a, a->Result()->Source());
}

TEST_F(IR_AccessTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* ty = (mod.Types().ptr<function, i32>());
            auto* var = b.Var(ty);
            b.Access(nullptr, var, u32(1));
        },
        "");
}

TEST_F(IR_AccessTest, Clone) {
    auto* type = ty.ptr<function, i32>();
    auto* var = b.Var(type);
    auto* idx1 = b.Constant(u32(1));
    auto* idx2 = b.Constant(u32(2));
    auto* a = b.Access(type, var, idx1, idx2);

    auto* new_a = clone_ctx.Clone(a);

    EXPECT_NE(a, new_a);

    EXPECT_NE(a->Result(), new_a->Result());
    EXPECT_EQ(type, new_a->Result()->Type());

    EXPECT_NE(nullptr, new_a->Object());
    EXPECT_EQ(a->Object(), new_a->Object());

    auto indices = new_a->Indices();
    EXPECT_EQ(2u, indices.Length());

    auto* val0 = indices[0]->As<Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto* val1 = indices[1]->As<Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_AccessTest, CloneNoIndices) {
    auto* type = ty.ptr<function, i32>();
    auto* var = b.Var(type);
    auto* a = b.Access(type, var);

    auto* new_a = clone_ctx.Clone(a);

    auto indices = new_a->Indices();
    EXPECT_EQ(0u, indices.Length());
}

}  // namespace
}  // namespace tint::core::ir
