// Copyright 2022 The Tint Authors.
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

#include "src/tint/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverStaticAssertTest = ResolverTest;

TEST_F(ResolverStaticAssertTest, Global_True_Pass) {
    GlobalStaticAssert(true);
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStaticAssertTest, Global_False_Fail) {
    GlobalStaticAssert(Source{{12, 34}}, false);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: static assertion failed");
}

TEST_F(ResolverStaticAssertTest, Global_Const_Pass) {
    GlobalConst("C", ty.bool_(), Expr(true));
    GlobalStaticAssert("C");
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStaticAssertTest, Global_Const_Fail) {
    GlobalConst("C", ty.bool_(), Expr(false));
    GlobalStaticAssert(Source{{12, 34}}, "C");
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: static assertion failed");
}

TEST_F(ResolverStaticAssertTest, Global_LessThan_Pass) {
    GlobalStaticAssert(LessThan(2_i, 3_i));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStaticAssertTest, Global_LessThan_Fail) {
    GlobalStaticAssert(Source{{12, 34}}, LessThan(4_i, 3_i));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: static assertion failed");
}

TEST_F(ResolverStaticAssertTest, Local_True_Pass) {
    WrapInFunction(StaticAssert(true));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStaticAssertTest, Local_False_Fail) {
    WrapInFunction(StaticAssert(Source{{12, 34}}, false));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: static assertion failed");
}

TEST_F(ResolverStaticAssertTest, Local_Const_Pass) {
    GlobalConst("C", ty.bool_(), Expr(true));
    WrapInFunction(StaticAssert("C"));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStaticAssertTest, Local_Const_Fail) {
    GlobalConst("C", ty.bool_(), Expr(false));
    WrapInFunction(StaticAssert(Source{{12, 34}}, "C"));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: static assertion failed");
}

TEST_F(ResolverStaticAssertTest, Local_NonConst) {
    GlobalVar("V", ty.bool_(), Expr(true), ast::AddressSpace::kPrivate);
    WrapInFunction(StaticAssert(Expr(Source{{12, 34}}, "V")));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: static assertion requires a const-expression, but expression is a "
              "runtime-expression");
}

TEST_F(ResolverStaticAssertTest, Local_LessThan_Pass) {
    WrapInFunction(StaticAssert(LessThan(2_i, 3_i)));
    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStaticAssertTest, Local_LessThan_Fail) {
    WrapInFunction(StaticAssert(Source{{12, 34}}, LessThan(4_i, 3_i)));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: static assertion failed");
}

}  // namespace
}  // namespace tint::resolver
