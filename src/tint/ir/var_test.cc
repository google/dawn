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

#include "src/tint/ir/var.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ir/builder.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/ir_test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_VarTest = IRTestHelper;

TEST_F(IR_VarTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Declare(nullptr);
        },
        "");
}

TEST_F(IR_VarTest, Initializer_Usage) {
    Module mod;
    Builder b{mod};
    auto* var = b.Declare(mod.Types().f32());
    auto* init = b.Constant(1_f);
    var->SetInitializer(init);

    EXPECT_THAT(init->Usages(), testing::UnorderedElementsAre(Usage{var, 0u}));
    var->SetInitializer(nullptr);
    EXPECT_TRUE(init->Usages().IsEmpty());
}

}  // namespace
}  // namespace tint::ir
