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
// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <int N, typename T>
using vec = builder::vec<N, T>;
template <typename T>
using vec2 = builder::vec2<T>;
template <typename T>
using vec3 = builder::vec3<T>;
template <typename T>
using vec4 = builder::vec4<T>;
template <int N, int M, typename T>
using mat = builder::mat<N, M, T>;
template <typename T>
using mat2x2 = builder::mat2x2<T>;
template <typename T>
using mat2x3 = builder::mat2x3<T>;
template <typename T>
using mat3x2 = builder::mat3x2<T>;
template <typename T>
using mat3x3 = builder::mat3x3<T>;
template <typename T>
using mat4x4 = builder::mat4x4<T>;
template <typename T, int ID = 0>
using alias = builder::alias<T, ID>;
template <typename T>
using alias1 = builder::alias1<T>;
template <typename T>
using alias2 = builder::alias2<T>;
template <typename T>
using alias3 = builder::alias3<T>;
using f32 = builder::f32;
using i32 = builder::i32;
using u32 = builder::u32;

using ResolverCallTest = ResolverTest;

TEST_F(ResolverCallTest, Recursive_Invalid) {
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
            "12:34 error v-0004: recursion is not permitted. 'main' attempted "
            "to call "
            "itself.");
}

TEST_F(ResolverCallTest, Undeclared_Invalid) {
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

  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: unable to find called function: func");
}

struct Params {
  builder::ast_expr_func_ptr create_value;
  builder::ast_type_func_ptr create_type;
};

template <typename T>
constexpr Params ParamsFor() {
  return Params{DataType<T>::Expr, DataType<T>::AST};
}

static constexpr Params all_param_types[] = {
    ParamsFor<bool>(),         //
    ParamsFor<u32>(),          //
    ParamsFor<i32>(),          //
    ParamsFor<f32>(),          //
    ParamsFor<vec3<bool>>(),   //
    ParamsFor<vec3<i32>>(),    //
    ParamsFor<vec3<u32>>(),    //
    ParamsFor<vec3<f32>>(),    //
    ParamsFor<mat3x3<i32>>(),  //
    ParamsFor<mat3x3<u32>>(),  //
    ParamsFor<mat3x3<f32>>(),  //
    ParamsFor<mat2x3<i32>>(),  //
    ParamsFor<mat2x3<u32>>(),  //
    ParamsFor<mat2x3<f32>>(),  //
    ParamsFor<mat3x2<i32>>(),  //
    ParamsFor<mat3x2<u32>>(),  //
    ParamsFor<mat3x2<f32>>()   //
};

TEST_F(ResolverCallTest, Valid) {
  ast::VariableList params;
  ast::ExpressionList args;
  for (auto& p : all_param_types) {
    params.push_back(Param(Sym(), p.create_type(*this)));
    args.push_back(p.create_value(*this, 0));
  }

  Func("foo", std::move(params), ty.void_(), {Return()});
  auto* call = Call("foo", std::move(args));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallTest, TooFewArgs) {
  Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(),
       {Return()});
  auto* call = Call(Source{{12, 34}}, "foo", 1);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: too few arguments in call to 'foo', expected 2, got 1");
}

TEST_F(ResolverCallTest, TooManyArgs) {
  Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(),
       {Return()});
  auto* call = Call(Source{{12, 34}}, "foo", 1, 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: too many arguments in call to 'foo', expected 2, got 3");
}

TEST_F(ResolverCallTest, MismatchedArgs) {
  Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(),
       {Return()});
  auto* call = Call("foo", Expr(Source{{12, 34}}, true), 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type mismatch for argument 1 in call to 'foo', "
            "expected 'i32', got 'bool'");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
