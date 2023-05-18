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

#include "src/tint/ir/builder.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_InstructionTest = TestHelper;

TEST_F(IR_InstructionTest, CreateLoad) {
    Module mod;
    Builder b{mod};

    auto* store_type = b.ir.types.Get<type::I32>();
    auto* var = b.Declare(b.ir.types.Get<type::Pointer>(
        store_type, builtin::AddressSpace::kFunction, builtin::Access::kReadWrite));
    const auto* inst = b.Load(var);

    ASSERT_TRUE(inst->Is<Load>());
    ASSERT_EQ(inst->From(), var);

    EXPECT_EQ(inst->Type(), store_type);

    ASSERT_TRUE(inst->From()->Is<ir::Var>());
    EXPECT_EQ(inst->From(), var);
}

TEST_F(IR_InstructionTest, Load_Usage) {
    Module mod;
    Builder b{mod};

    auto* store_type = b.ir.types.Get<type::I32>();
    auto* var = b.Declare(b.ir.types.Get<type::Pointer>(
        store_type, builtin::AddressSpace::kFunction, builtin::Access::kReadWrite));
    const auto* inst = b.Load(var);

    ASSERT_NE(inst->From(), nullptr);
    ASSERT_EQ(inst->From()->Usage().Length(), 1u);
    EXPECT_EQ(inst->From()->Usage()[0], inst);
}

}  // namespace
}  // namespace tint::ir
