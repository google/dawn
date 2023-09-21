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

#include "src/tint/lang/core/ir/let.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_LetTest = IRTestHelper;

TEST_F(IR_LetTest, Fail_NullValue) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            ir::Value* value = nullptr;
            b.Let("l", value);
        },
        "");
}

TEST_F(IR_LetTest, Results) {
    auto* value = b.Constant(1_f);
    auto* let = b.Let("l", value);
    EXPECT_TRUE(let->HasResults());
    EXPECT_FALSE(let->HasMultiResults());
    EXPECT_TRUE(let->Result()->Is<InstructionResult>());
    EXPECT_EQ(let->Result()->Source(), let);
    EXPECT_EQ(let->Result()->Type(), value->Type());
}

TEST_F(IR_LetTest, Clone) {
    auto* value = b.Constant(4_f);
    auto* let = b.Let("l", value);

    auto* new_let = clone_ctx.Clone(let);

    EXPECT_NE(let, new_let);
    EXPECT_NE(nullptr, new_let->Result());
    EXPECT_NE(let->Result(), new_let->Result());

    auto new_val = new_let->Value()->As<Constant>()->Value();
    ASSERT_TRUE(new_val->Is<core::constant::Scalar<f32>>());
    EXPECT_FLOAT_EQ(4_f, new_val->As<core::constant::Scalar<f32>>()->ValueAs<f32>());

    EXPECT_EQ(std::string("l"), mod.NameOf(new_let).Name());
    EXPECT_EQ(std::string("l"), mod.NameOf(new_let->Result()).Name());
}

}  // namespace
}  // namespace tint::core::ir
