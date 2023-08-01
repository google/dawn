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

#include "src/tint/lang/core/ir/var.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_VarTest = IRTestHelper;

TEST_F(IR_VarTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Var(nullptr);
        },
        "");
}

TEST_F(IR_VarTest, Results) {
    auto* var = b.Var(ty.ptr<function, f32>());
    EXPECT_TRUE(var->HasResults());
    EXPECT_FALSE(var->HasMultiResults());
    EXPECT_TRUE(var->Result()->Is<InstructionResult>());
    EXPECT_EQ(var->Result()->Source(), var);
}

TEST_F(IR_VarTest, Initializer_Usage) {
    Module mod;
    Builder b{mod};
    auto* var = b.Var(ty.ptr<function, f32>());
    auto* init = b.Constant(1_f);
    var->SetInitializer(init);

    EXPECT_THAT(init->Usages(), testing::UnorderedElementsAre(Usage{var, 0u}));
    var->SetInitializer(nullptr);
    EXPECT_TRUE(init->Usages().IsEmpty());
}

}  // namespace
}  // namespace tint::ir
