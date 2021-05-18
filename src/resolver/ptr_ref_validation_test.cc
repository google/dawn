// Copyright 2021 The Tint Authors.
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

#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/reference_type.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {
namespace {

struct ResolverPtrRefValidationTest : public resolver::TestHelper,
                                      public testing::Test {};

TEST_F(ResolverPtrRefValidationTest, AddressOfLiteral) {
  // &1

  auto* expr = AddressOf(Expr(Source{{12, 34}}, 1));

  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfLet) {
  // let l : i32 = 1;
  // &l
  auto* l = Const("l", ty.i32(), Expr(1));
  auto* expr = AddressOf(Expr(Source{{12, 34}}, "l"));

  WrapInFunction(l, expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverPtrRefValidationTest, DerefOfLiteral) {
  // *1

  auto* expr = Deref(Expr(Source{{12, 34}}, 1));

  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: cannot dereference expression of type 'i32'");
}

TEST_F(ResolverPtrRefValidationTest, DerefOfVar) {
  // var v : i32 = 1;
  // *1
  auto* v = Var("v", ty.i32());
  auto* expr = Deref(Expr(Source{{12, 34}}, "v"));

  WrapInFunction(v, expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: cannot dereference expression of type 'i32'");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
