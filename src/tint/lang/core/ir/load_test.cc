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

using IR_LoadTest = IRTestHelper;

TEST_F(IR_LoadTest, Create) {
    auto* store_type = ty.i32();
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* inst = b.Load(var);

    ASSERT_TRUE(inst->Is<Load>());
    ASSERT_EQ(inst->From(), var->Result());
    EXPECT_EQ(inst->Result()->Type(), store_type);

    auto* result = inst->From()->As<InstructionResult>();
    ASSERT_NE(result, nullptr);
    ASSERT_TRUE(result->Source()->Is<ir::Var>());
    EXPECT_EQ(result->Source(), var);
}

TEST_F(IR_LoadTest, Usage) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* inst = b.Load(var);

    ASSERT_NE(inst->From(), nullptr);
    EXPECT_THAT(inst->From()->Usages(), testing::UnorderedElementsAre(Usage{inst, 0u}));
}

TEST_F(IR_LoadTest, Results) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* inst = b.Load(var);

    EXPECT_TRUE(inst->HasResults());
    EXPECT_FALSE(inst->HasMultiResults());
    EXPECT_TRUE(inst->Result()->Is<InstructionResult>());
    EXPECT_EQ(inst->Result()->Source(), inst);
}

TEST_F(IR_LoadTest, Fail_NonPtr_Builder) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Load(b.Constant(1_i));
        },
        "");
}

}  // namespace
}  // namespace tint::ir
