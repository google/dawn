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

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_IntrinsicCallTest = IRTestHelper;

TEST_F(IR_IntrinsicCallTest, Usage) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* intrinsic = b.Call(mod.Types().f32(), IntrinsicCall::Kind::kSpirvSelect, arg1, arg2);

    ASSERT_TRUE(intrinsic->Is<IntrinsicCall>());
    EXPECT_THAT(arg1->Usages(), testing::UnorderedElementsAre(Usage{intrinsic, 0u}));
    EXPECT_THAT(arg2->Usages(), testing::UnorderedElementsAre(Usage{intrinsic, 1u}));
}

TEST_F(IR_IntrinsicCallTest, Result) {
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_u);
    auto* intrinsic = b.Call(mod.Types().f32(), IntrinsicCall::Kind::kSpirvSelect, arg1, arg2);

    ASSERT_TRUE(intrinsic->Is<IntrinsicCall>());
    EXPECT_TRUE(intrinsic->HasResults());
    EXPECT_FALSE(intrinsic->HasMultiResults());
    EXPECT_TRUE(intrinsic->Result()->Is<InstructionResult>());
    EXPECT_EQ(intrinsic->Result()->Source(), intrinsic);
}

TEST_F(IR_IntrinsicCallTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Call(nullptr, IntrinsicCall::Kind::kSpirvSelect);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
