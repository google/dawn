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

#include "src/tint/ir/return.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ir/ir_test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_ReturnTest = IRTestHelper;

TEST_F(IR_ReturnTest, Fail_NullFunction) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Return(nullptr);
        },
        "");
}

TEST_F(IR_ReturnTest, Fail_NullValue) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            mod.values.Create<Return>(b.Function("myfunc", mod.Types().void_()), nullptr);
        },
        "");
}

TEST_F(IR_ReturnTest, ImplicitNoValue) {
    auto* ret = b.Return(b.Function("myfunc", ty.void_()));
    EXPECT_TRUE(ret->Args().IsEmpty());
}

TEST_F(IR_ReturnTest, WithValue) {
    auto* val = b.Constant(42_i);
    auto* ret = b.Return(b.Function("myfunc", ty.i32()), val);
    ASSERT_EQ(ret->Args().Length(), 1u);
    EXPECT_EQ(ret->Args()[0], val);
    EXPECT_THAT(val->Usages(), testing::UnorderedElementsAre(Usage{ret, 0u}));
}

}  // namespace
}  // namespace tint::ir
