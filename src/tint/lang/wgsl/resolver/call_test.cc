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

#include "src/tint/lang/wgsl/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

template <typename T, int ID = 0>
using alias = builder::alias<T, ID>;

template <typename T>
using alias1 = builder::alias1<T>;

template <typename T>
using alias2 = builder::alias2<T>;

template <typename T>
using alias3 = builder::alias3<T>;

using ResolverCallTest = ResolverTest;

struct Params {
    builder::ast_expr_from_double_func_ptr create_value;
    builder::ast_type_func_ptr create_type;
};

template <typename T>
constexpr Params ParamsFor() {
    return Params{builder::DataType<T>::ExprFromDouble, builder::DataType<T>::AST};
}

static constexpr Params all_param_types[] = {
    ParamsFor<bool>(),         //
    ParamsFor<u32>(),          //
    ParamsFor<i32>(),          //
    ParamsFor<f32>(),          //
    ParamsFor<f16>(),          //
    ParamsFor<vec3<bool>>(),   //
    ParamsFor<vec3<i32>>(),    //
    ParamsFor<vec3<u32>>(),    //
    ParamsFor<vec3<f32>>(),    //
    ParamsFor<mat3x3<f32>>(),  //
    ParamsFor<mat2x3<f32>>(),  //
    ParamsFor<mat3x2<f32>>()   //
};

TEST_F(ResolverCallTest, Valid) {
    Enable(builtin::Extension::kF16);

    Vector<const ast::Parameter*, 4> params;
    Vector<const ast::Expression*, 4> args;
    for (auto& p : all_param_types) {
        params.Push(Param(Sym(), p.create_type(*this)));
        args.Push(p.create_value(*this, 0));
    }

    auto* func = Func("foo", std::move(params), ty.f32(), Vector{Return(1.23_f)});
    auto* call_expr = Call("foo", std::move(args));
    WrapInFunction(call_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(call_expr);
    EXPECT_NE(call, nullptr);
    EXPECT_EQ(call->Target(), Sem().Get(func));
}

TEST_F(ResolverCallTest, OutOfOrder) {
    auto* call_expr = Call("b");
    Func("a", tint::Empty, ty.void_(), Vector{CallStmt(call_expr)});
    auto* b = Func("b", tint::Empty, ty.void_(), tint::Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* call = Sem().Get<sem::Call>(call_expr);
    EXPECT_NE(call, nullptr);
    EXPECT_EQ(call->Target(), Sem().Get(b));
}

}  // namespace
}  // namespace tint::resolver
