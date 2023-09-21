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

#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::spirv::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
                                              //
using IR_SpirvBuiltinCallTest = core::ir::IRTestHelper;

TEST_F(IR_SpirvBuiltinCallTest, Clone) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().f32(), BuiltinFn::kArrayLength, 1_u, 2_u);

    auto* new_b = clone_ctx.Clone(builtin);

    EXPECT_NE(builtin, new_b);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().f32(), new_b->Result()->Type());

    EXPECT_EQ(BuiltinFn::kArrayLength, new_b->Func());

    auto args = new_b->Args();
    EXPECT_EQ(2u, args.Length());

    auto* val0 = args[0]->As<core::ir::Constant>()->Value();
    EXPECT_EQ(1_u, val0->As<core::constant::Scalar<core::u32>>()->ValueAs<core::u32>());

    auto* val1 = args[1]->As<core::ir::Constant>()->Value();
    EXPECT_EQ(2_u, val1->As<core::constant::Scalar<core::u32>>()->ValueAs<core::u32>());
}

TEST_F(IR_SpirvBuiltinCallTest, CloneNoArgs) {
    auto* builtin = b.Call<BuiltinCall>(mod.Types().f32(), BuiltinFn::kArrayLength);

    auto* new_b = clone_ctx.Clone(builtin);
    EXPECT_NE(builtin->Result(), new_b->Result());
    EXPECT_EQ(mod.Types().f32(), new_b->Result()->Type());

    EXPECT_EQ(BuiltinFn::kArrayLength, new_b->Func());

    auto args = new_b->Args();
    EXPECT_TRUE(args.IsEmpty());
}

}  // namespace
}  // namespace tint::spirv::ir
