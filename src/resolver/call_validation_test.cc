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

#include "gmock/gmock.h"
#include "src/ast/call_statement.h"
#include "src/resolver/resolver_test_helper.h"

namespace tint {
namespace resolver {
namespace {

using ResolverCallValidationTest = ResolverTest;

TEST_F(ResolverCallValidationTest, Recursive_Invalid) {
  // fn main() {main(); }

  SetSource(Source::Location{12, 34});
  auto* call_expr = Call("main");
  ast::VariableList params0;

  Func("main", params0, ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(call_expr),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: recursion is not permitted. 'main' attempted to call "
            "itself.");
}

TEST_F(ResolverCallValidationTest, Undeclared_Invalid) {
  // fn main() {func(); return; }
  // fn func() { return; }

  SetSource(Source::Location{12, 34});
  auto* call_expr = Call("func");
  ast::VariableList params0;

  Func("main", params0, ty.f32(),
       ast::StatementList{
           create<ast::CallStatement>(call_expr),
           Return(),
       },
       ast::DecorationList{});

  Func("func", params0, ty.f32(),
       ast::StatementList{
           Return(),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "12:34 error: unable to find called function: func");
}

TEST_F(ResolverCallValidationTest, TooFewArgs) {
  Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(),
       {Return()});
  auto* call = Call(Source{{12, 34}}, "foo", 1);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: too few arguments in call to 'foo', expected 2, got 1");
}

TEST_F(ResolverCallValidationTest, TooManyArgs) {
  Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(),
       {Return()});
  auto* call = Call(Source{{12, 34}}, "foo", 1, 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: too many arguments in call to 'foo', expected 2, got 3");
}

TEST_F(ResolverCallValidationTest, MismatchedArgs) {
  Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(),
       {Return()});
  auto* call = Call("foo", Expr(Source{{12, 34}}, true), 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type mismatch for argument 1 in call to 'foo', "
            "expected 'i32', got 'bool'");
}

TEST_F(ResolverCallValidationTest, UnusedRetval) {
  // fn main() {func(); return; }
  // fn func() { return; }

  Func("func", {}, ty.f32(), {Return(Expr(1.0f))}, {});

  Func("main", {}, ty.f32(),
       ast::StatementList{
           create<ast::CallStatement>(Source{{12, 34}}, Call("func")),
           Return(),
       },
       {});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: result of called function was not used. If this was "
            "intentional wrap the function call in ignore()");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
