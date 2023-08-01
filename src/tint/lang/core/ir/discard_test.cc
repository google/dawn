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

#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using IR_DiscardTest = IRTestHelper;

TEST_F(IR_DiscardTest, Discard) {
    auto* inst = b.Discard();
    ASSERT_TRUE(inst->Is<ir::Discard>());
}

TEST_F(IR_DiscardTest, Result) {
    auto* inst = b.Discard();

    EXPECT_FALSE(inst->HasResults());
    EXPECT_FALSE(inst->HasMultiResults());
}

}  // namespace
}  // namespace tint::ir
