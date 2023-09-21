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

#include <string>

#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_BlockParamTest = IRTestHelper;

TEST_F(IR_BlockParamTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.BlockParam(nullptr);
        },
        "");
}

TEST_F(IR_BlockParamTest, Clone) {
    auto* inst = b.BlockParam(mod.Types().i32());

    auto* new_inst = clone_ctx.Clone(inst);

    EXPECT_NE(inst, new_inst);
    EXPECT_EQ(mod.Types().i32(), new_inst->Type());
}

TEST_F(IR_BlockParamTest, CloneWithName) {
    auto* inst = b.BlockParam("p", mod.Types().i32());

    auto* new_inst = clone_ctx.Clone(inst);
    EXPECT_EQ(mod.Types().i32(), new_inst->Type());

    EXPECT_EQ(std::string("p"), mod.NameOf(new_inst).Name());
}

}  // namespace
}  // namespace tint::core::ir
