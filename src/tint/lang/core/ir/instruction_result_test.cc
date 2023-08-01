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
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::number_suffixes;        // NOLINT
using namespace tint::builtin::fluent_types;  // NOLINT

namespace tint::ir {
namespace {

using IR_InstructionResultTest = IRTestHelper;

TEST_F(IR_InstructionResultTest, Destroy_HasSource) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* val = b.Add(mod.Types().i32(), 1_i, 2_i)->Result();
            val->Destroy();
        },
        "");
}

}  // namespace
}  // namespace tint::ir
