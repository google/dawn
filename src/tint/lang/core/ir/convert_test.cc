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

#include "src/tint/lang/core/ir/convert.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_ConvertTest = IRTestHelper;

TEST_F(IR_ConvertTest, Fail_NullToType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Convert(nullptr, 1_u);
        },
        "");
}

TEST_F(IR_ConvertTest, Results) {
    auto* c = b.Convert(mod.Types().i32(), 1_u);

    EXPECT_TRUE(c->HasResults());
    EXPECT_FALSE(c->HasMultiResults());
    EXPECT_TRUE(c->Result()->Is<InstructionResult>());
    EXPECT_EQ(c->Result()->Source(), c);
}

}  // namespace
}  // namespace tint::ir
