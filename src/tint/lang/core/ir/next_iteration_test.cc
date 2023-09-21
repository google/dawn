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

#include "src/tint/lang/core/ir/next_iteration.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_NextIterationTest = IRTestHelper;

TEST_F(IR_NextIterationTest, Fail_NullLoop) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.NextIteration(nullptr);
        },
        "");
}

TEST_F(IR_NextIterationTest, Result) {
    auto* inst = b.NextIteration(b.Loop());

    EXPECT_FALSE(inst->HasResults());
    EXPECT_FALSE(inst->HasMultiResults());
}

TEST_F(IR_NextIterationTest, Clone) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);

    auto* loop = b.Loop();
    auto* inst = b.NextIteration(loop, arg1, arg2);

    auto* new_loop = clone_ctx.Clone(loop);
    auto* new_inst = clone_ctx.Clone(inst);

    EXPECT_NE(inst, new_inst);
    EXPECT_EQ(new_loop, new_inst->Loop());

    auto args = new_inst->Args();
    EXPECT_EQ(2u, args.Length());

    auto* val0 = args[0]->As<Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<u32>>()->ValueAs<u32>());

    auto* val1 = args[1]->As<Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<u32>>()->ValueAs<u32>());
}

TEST_F(IR_NextIterationTest, CloneNoArgs) {
    auto* loop = b.Loop();
    auto* inst = b.NextIteration(loop);

    auto* new_loop = clone_ctx.Clone(loop);
    auto* new_inst = clone_ctx.Clone(inst);

    EXPECT_EQ(new_loop, new_inst->Loop());
}

}  // namespace
}  // namespace tint::core::ir
