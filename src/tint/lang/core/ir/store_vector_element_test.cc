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

using IR_StoreVectorElementTest = IRTestHelper;

TEST_F(IR_StoreVectorElementTest, Create) {
    auto* to = b.Var(ty.ptr<private_, vec3<i32>>());
    auto* inst = b.StoreVectorElement(to, 2_i, 4_i);

    ASSERT_TRUE(inst->Is<StoreVectorElement>());
    ASSERT_EQ(inst->To(), to->Result());

    ASSERT_TRUE(inst->Index()->Is<Constant>());
    auto index = inst->Index()->As<Constant>()->Value();
    ASSERT_TRUE(index->Is<constant::Scalar<i32>>());
    EXPECT_EQ(2_i, index->As<constant::Scalar<i32>>()->ValueAs<i32>());

    ASSERT_TRUE(inst->Value()->Is<Constant>());
    auto value = inst->Value()->As<Constant>()->Value();
    ASSERT_TRUE(value->Is<constant::Scalar<i32>>());
    EXPECT_EQ(4_i, value->As<constant::Scalar<i32>>()->ValueAs<i32>());
}

TEST_F(IR_StoreVectorElementTest, Usage) {
    auto* to = b.Var(ty.ptr<private_, vec3<i32>>());
    auto* inst = b.StoreVectorElement(to, 2_i, 4_i);

    ASSERT_NE(inst->To(), nullptr);
    EXPECT_THAT(inst->To()->Usages(), testing::UnorderedElementsAre(Usage{inst, 0u}));

    ASSERT_NE(inst->Index(), nullptr);
    EXPECT_THAT(inst->Index()->Usages(), testing::UnorderedElementsAre(Usage{inst, 1u}));

    ASSERT_NE(inst->Value(), nullptr);
    EXPECT_THAT(inst->Value()->Usages(), testing::UnorderedElementsAre(Usage{inst, 2u}));
}

TEST_F(IR_StoreVectorElementTest, Result) {
    auto* to = b.Var(ty.ptr<private_, vec3<i32>>());
    auto* inst = b.StoreVectorElement(to, 2_i, 4_i);

    EXPECT_FALSE(inst->HasResults());
    EXPECT_FALSE(inst->HasMultiResults());
}

}  // namespace
}  // namespace tint::ir
