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

#include "src/tint/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverCallValidationTest = ResolverTest;

TEST_F(ResolverCallValidationTest, TooFewArgs) {
    Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(), {Return()});
    auto* call = Call(Source{{12, 34}}, "foo", 1_i);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: too few arguments in call to 'foo', expected 2, got 1");
}

TEST_F(ResolverCallValidationTest, TooManyArgs) {
    Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(), {Return()});
    auto* call = Call(Source{{12, 34}}, "foo", 1_i, 1.0f, 1.0f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: too many arguments in call to 'foo', expected 2, got 3");
}

TEST_F(ResolverCallValidationTest, MismatchedArgs) {
    Func("foo", {Param(Sym(), ty.i32()), Param(Sym(), ty.f32())}, ty.void_(), {Return()});
    auto* call = Call("foo", Expr(Source{{12, 34}}, true), 1.0f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type mismatch for argument 1 in call to 'foo', "
              "expected 'i32', got 'bool'");
}

TEST_F(ResolverCallValidationTest, UnusedRetval) {
    // fn func() -> f32 { return 1.0; }
    // fn main() {func(); return; }

    Func("func", {}, ty.f32(), {Return(Expr(1.0f))}, {});

    Func("main", {}, ty.void_(),
         {
             CallStmt(Source{{12, 34}}, Call("func")),
             Return(),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, PointerArgument_VariableIdentExpr) {
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   var z: i32 = 1i;
    //   foo(&z);
    // }
    auto* param = Param("p", ty.pointer<i32>(ast::StorageClass::kFunction));
    Func("foo", {param}, ty.void_(), {});
    Func("main", {}, ty.void_(),
         {
             Decl(Var("z", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Expr("z")))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, PointerArgument_ConstIdentExpr) {
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   let z: i32 = 1i;
    //   foo(&z);
    // }
    auto* param = Param("p", ty.pointer<i32>(ast::StorageClass::kFunction));
    Func("foo", {param}, ty.void_(), {});
    Func("main", {}, ty.void_(),
         {
             Decl(Let("z", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf(Expr(Source{{12, 34}}, "z")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverCallValidationTest, PointerArgument_NotIdentExprVar) {
    // struct S { m: i32; };
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   var v: S;
    //   foo(&v.m);
    // }
    auto* S = Structure("S", {Member("m", ty.i32())});
    auto* param = Param("p", ty.pointer<i32>(ast::StorageClass::kFunction));
    Func("foo", {param}, ty.void_(), {});
    Func("main", {}, ty.void_(),
         {
             Decl(Var("v", ty.Of(S))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, MemberAccessor("v", "m")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: expected an address-of expression of a variable "
              "identifier expression or a function parameter");
}

TEST_F(ResolverCallValidationTest, PointerArgument_AddressOfMemberAccessor) {
    // struct S { m: i32; };
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   let v: S = S();
    //   foo(&v.m);
    // }
    auto* S = Structure("S", {Member("m", ty.i32())});
    auto* param = Param("p", ty.pointer<i32>(ast::StorageClass::kFunction));
    Func("foo", {param}, ty.void_(), {});
    Func("main", {}, ty.void_(),
         {
             Decl(Let("v", ty.Of(S), Construct(ty.Of(S)))),
             CallStmt(Call("foo", AddressOf(MemberAccessor(Source{{12, 34}}, "v", "m")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverCallValidationTest, PointerArgument_FunctionParam) {
    // fn foo(p: ptr<function, i32>) {}
    // fn bar(p: ptr<function, i32>) {
    // foo(p);
    // }
    Func("foo", {Param("p", ty.pointer<i32>(ast::StorageClass::kFunction))}, ty.void_(), {});
    Func("bar", {Param("p", ty.pointer<i32>(ast::StorageClass::kFunction))}, ty.void_(),
         ast::StatementList{CallStmt(Call("foo", Expr("p")))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, PointerArgument_FunctionParamWithMain) {
    // fn foo(p: ptr<function, i32>) {}
    // fn bar(p: ptr<function, i32>) {
    // foo(p);
    // }
    // @stage(fragment)
    // fn main() {
    //   var v: i32;
    //   bar(&v);
    // }
    Func("foo", {Param("p", ty.pointer<i32>(ast::StorageClass::kFunction))}, ty.void_(), {});
    Func("bar", {Param("p", ty.pointer<i32>(ast::StorageClass::kFunction))}, ty.void_(),
         ast::StatementList{CallStmt(Call("foo", Expr("p")))});
    Func("main", ast::VariableList{}, ty.void_(),
         {
             Decl(Var("v", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf(Expr("v")))),
         },
         {
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, LetPointer) {
    // fn x(p : ptr<function, i32>) -> i32 {}
    // @stage(fragment)
    // fn main() {
    //   var v: i32;
    //   let p: ptr<function, i32> = &v;
    //   var c: i32 = x(p);
    // }
    Func("x", {Param("p", ty.pointer<i32>(ast::StorageClass::kFunction))}, ty.void_(), {});
    auto* v = Var("v", ty.i32());
    auto* p = Let("p", ty.pointer(ty.i32(), ast::StorageClass::kFunction), AddressOf(v));
    auto* c = Var("c", ty.i32(), ast::StorageClass::kNone, Call("x", Expr(Source{{12, 34}}, p)));
    Func("main", ast::VariableList{}, ty.void_(),
         {
             Decl(v),
             Decl(p),
             Decl(c),
         },
         {
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: expected an address-of expression of a variable "
              "identifier expression or a function parameter");
}

TEST_F(ResolverCallValidationTest, LetPointerPrivate) {
    // let p: ptr<private, i32> = &v;
    // fn foo(p : ptr<private, i32>) -> i32 {}
    // var v: i32;
    // @stage(fragment)
    // fn main() {
    //   var c: i32 = foo(p);
    // }
    Func("foo", {Param("p", ty.pointer<i32>(ast::StorageClass::kPrivate))}, ty.void_(), {});
    auto* v = Global("v", ty.i32(), ast::StorageClass::kPrivate);
    auto* p = Let("p", ty.pointer(ty.i32(), ast::StorageClass::kPrivate), AddressOf(v));
    auto* c = Var("c", ty.i32(), ast::StorageClass::kNone, Call("foo", Expr(Source{{12, 34}}, p)));
    Func("main", ast::VariableList{}, ty.void_(),
         {
             Decl(p),
             Decl(c),
         },
         {
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: expected an address-of expression of a variable "
              "identifier expression or a function parameter");
}

TEST_F(ResolverCallValidationTest, CallVariable) {
    // var v : i32;
    // fn f() {
    //   v();
    // }
    Global("v", ty.i32(), ast::StorageClass::kPrivate);
    Func("f", {}, ty.void_(), {CallStmt(Call(Source{{12, 34}}, "v"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: cannot call variable 'v'
note: 'v' declared here)");
}

TEST_F(ResolverCallValidationTest, CallVariableShadowsFunction) {
    // fn x() {}
    // fn f() {
    //   var x : i32;
    //   x();
    // }
    Func("x", {}, ty.void_(), {});
    Func("f", {}, ty.void_(),
         {
             Decl(Var(Source{{56, 78}}, "x", ty.i32())),
             CallStmt(Call(Source{{12, 34}}, "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: cannot call variable 'x'
56:78 note: 'x' declared here)");
}

}  // namespace
}  // namespace tint::resolver
