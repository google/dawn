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

#include "src/tint/lang/core/ir/return.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_ReturnTest = IRTestHelper;

TEST_F(IR_ReturnTest, ImplicitNoValue) {
    auto* func = b.Function("myfunc", ty.void_());
    auto* ret = b.Return(func);
    ASSERT_EQ(ret->Func(), func);
    EXPECT_TRUE(ret->Args().IsEmpty());
    EXPECT_EQ(ret->Value(), nullptr);
    EXPECT_THAT(func->Usages(), testing::UnorderedElementsAre(Usage{ret, 0u}));
}

TEST_F(IR_ReturnTest, WithValue) {
    auto* func = b.Function("myfunc", ty.i32());
    auto* val = b.Constant(42_i);
    auto* ret = b.Return(func, val);
    ASSERT_EQ(ret->Func(), func);
    ASSERT_EQ(ret->Args().Length(), 1u);
    EXPECT_EQ(ret->Args()[0], val);
    EXPECT_EQ(ret->Value(), val);
    EXPECT_THAT(func->Usages(), testing::UnorderedElementsAre(Usage{ret, 0u}));
    EXPECT_THAT(val->Usages(), testing::UnorderedElementsAre(Usage{ret, 1u}));
}

TEST_F(IR_ReturnTest, Result) {
    auto* vfunc = b.Function("vfunc", ty.void_());
    auto* ifunc = b.Function("ifunc", ty.i32());

    {
        auto* ret1 = b.Return(vfunc);
        EXPECT_FALSE(ret1->HasResults());
        EXPECT_FALSE(ret1->HasMultiResults());
    }

    {
        auto* ret2 = b.Return(ifunc, b.Constant(42_i));
        EXPECT_FALSE(ret2->HasResults());
        EXPECT_FALSE(ret2->HasMultiResults());
    }
}

}  // namespace
}  // namespace tint::ir
