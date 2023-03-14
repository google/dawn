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
    Func("foo",
         utils::Vector{
             Param(Sym(), ty.i32()),
             Param(Sym(), ty.f32()),
         },
         ty.void_(),
         utils::Vector{
             Return(),
         });
    auto* call = Call(Source{{12, 34}}, "foo", 1_i);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: too few arguments in call to 'foo', expected 2, got 1");
}

TEST_F(ResolverCallValidationTest, TooManyArgs) {
    Func("foo",
         utils::Vector{
             Param(Sym(), ty.i32()),
             Param(Sym(), ty.f32()),
         },
         ty.void_(),
         utils::Vector{
             Return(),
         });
    auto* call = Call(Source{{12, 34}}, "foo", 1_i, 1_f, 1_f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: too many arguments in call to 'foo', expected 2, got 3");
}

TEST_F(ResolverCallValidationTest, MismatchedArgs) {
    Func("foo",
         utils::Vector{
             Param(Sym(), ty.i32()),
             Param(Sym(), ty.f32()),
         },
         ty.void_(),
         utils::Vector{
             Return(),
         });
    auto* call = Call("foo", Expr(Source{{12, 34}}, true), 1_f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type mismatch for argument 1 in call to 'foo', "
              "expected 'i32', got 'bool'");
}

TEST_F(ResolverCallValidationTest, UnusedRetval) {
    // fn func() -> f32 { return 1.0; }
    // fn main() {func(); return; }

    Func("func", utils::Empty, ty.f32(),
         utils::Vector{
             Return(Expr(1_f)),
         },
         utils::Empty);

    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
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
    auto* param = Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction));
    Func("foo", utils::Vector{param}, ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("z", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Expr("z")))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, PointerArgument_LetIdentExpr) {
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   let z: i32 = 1i;
    //   foo(&z);
    // }
    auto* param = Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction));
    Func("foo", utils::Vector{param}, ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("z", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf(Expr(Source{{12, 34}}, "z")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverCallValidationTest, PointerArgument_AddressOfFunctionMember) {
    // struct S { m: i32; };
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   var v : S;
    //   foo(&v.m);
    // }
    auto* S = Structure("S", utils::Vector{
                                 Member("m", ty.i32()),
                             });
    auto* param = Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction));
    Func("foo", utils::Vector{param}, ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.Of(S))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, MemberAccessor("v", "m")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: arguments of pointer type must not point to a subset of the "
              "originating variable");
}

TEST_F(ResolverCallValidationTest,
       PointerArgument_AddressOfFunctionMember_WithFullPtrParametersExt) {
    // enable chromium_experimental_full_ptr_parameters;
    // struct S { m: i32; };
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   var v : S;
    //   foo(&v.m);
    // }
    Enable(builtin::Extension::kChromiumExperimentalFullPtrParameters);
    auto* S = Structure("S", utils::Vector{
                                 Member("m", ty.i32()),
                             });
    auto* param = Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction));
    Func("foo", utils::Vector{param}, ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.Of(S))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, MemberAccessor("v", "m")))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
TEST_F(ResolverCallValidationTest, PointerArgument_AddressOfLetMember) {
    // struct S { m: i32; };
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   let v: S = S();
    //   foo(&v.m);
    // }
    auto* S = Structure("S", utils::Vector{
                                 Member("m", ty.i32()),
                             });
    auto* param = Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction));
    Func("foo", utils::Vector{param}, ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("v", ty.Of(S), Call(ty.Of(S)))),
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
    Func("foo",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(), utils::Empty);
    Func("bar",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(),
         utils::Vector{
             CallStmt(Call("foo", Expr("p"))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, PointerArgument_FunctionParamWithMain) {
    // fn foo(p: ptr<function, i32>) {}
    // fn bar(p: ptr<function, i32>) {
    // foo(p);
    // }
    // @fragment
    // fn main() {
    //   var v: i32;
    //   bar(&v);
    // }
    Func("foo",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(), utils::Empty);
    Func("bar",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(),
         utils::Vector{
             CallStmt(Call("foo", "p")),
         });
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf("v"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, LetPointer) {
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: i32;
    //   let p: ptr<function, i32> = &v;
    //   x(p);
    // }
    Func("x",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.i32())),
             Decl(Let("p", ty.pointer(ty.i32(), builtin::AddressSpace::kFunction), AddressOf("v"))),
             CallStmt(Call("x", "p")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, LetPointerPrivate) {
    // fn foo(p : ptr<private, i32>) {}
    // var v : i32;
    // @fragment
    // fn main() {
    //   let p : ptr<private, i32> = &v;
    //   foo(p);
    // }
    Func("foo",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kPrivate)),
         },
         ty.void_(), utils::Empty);
    GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("p", ty.pointer(ty.i32(), builtin::AddressSpace::kPrivate), AddressOf("v"))),
             CallStmt(Call("foo", Expr(Source{{12, 34}}, "p"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, LetPointer_NotWholeVar) {
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p: ptr<function, i32> = &(v[0]);
    //   x(p);
    // }
    Func("foo",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p", ty.pointer(ty.i32(), builtin::AddressSpace::kFunction),
                      AddressOf(IndexAccessor("v", 0_a)))),
             CallStmt(Call("foo", Expr(Source{{12, 34}}, "p"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: arguments of pointer type must not point to a subset of the "
              "originating variable");
}

TEST_F(ResolverCallValidationTest, LetPointer_NotWholeVar_WithFullPtrParametersExt) {
    // enable chromium_experimental_full_ptr_parameters;
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p: ptr<function, i32> = &(v[0]);
    //   x(p);
    // }
    Enable(builtin::Extension::kChromiumExperimentalFullPtrParameters);
    Func("foo",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p", ty.pointer(ty.i32(), builtin::AddressSpace::kFunction),
                      AddressOf(IndexAccessor("v", 0_a)))),
             CallStmt(Call("foo", Expr(Source{{12, 34}}, "p"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverCallValidationTest, ComplexPointerChain) {
    // fn foo(p : ptr<function, array<i32, 4>>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p1 = &v;
    //   let p2 = p1;
    //   let p3 = &*p2;
    //   foo(&*p);
    // }
    Func("foo",
         utils::Vector{
             Param("p", ty.pointer(ty.array<i32, 4>(), builtin::AddressSpace::kFunction)),
         },
         ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p1", AddressOf("v"))),
             Decl(Let("p2", Expr("p1"))),
             Decl(Let("p3", AddressOf(Deref("p2")))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Deref("p3")))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, ComplexPointerChain_NotWholeVar) {
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p1 = &v;
    //   let p2 = p1;
    //   let p3 = &(*p2)[0];
    //   foo(&*p);
    // }
    Func("foo",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p1", AddressOf("v"))),
             Decl(Let("p2", Expr("p1"))),
             Decl(Let("p3", AddressOf(IndexAccessor(Deref("p2"), 0_a)))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Deref("p3")))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: arguments of pointer type must not point to a subset of the "
              "originating variable");
}

TEST_F(ResolverCallValidationTest, ComplexPointerChain_NotWholeVar_WithFullPtrParametersExt) {
    // enable chromium_experimental_full_ptr_parameters;
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p1 = &v;
    //   let p2 = p1;
    //   let p3 = &(*p2)[0];
    //   foo(&*p);
    // }
    Enable(builtin::Extension::kChromiumExperimentalFullPtrParameters);
    Func("foo",
         utils::Vector{
             Param("p", ty.pointer<i32>(builtin::AddressSpace::kFunction)),
         },
         ty.void_(), utils::Empty);
    Func("main", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p1", AddressOf("v"))),
             Decl(Let("p2", Expr("p1"))),
             Decl(Let("p3", AddressOf(IndexAccessor(Deref("p2"), 0_a)))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Deref("p3")))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverCallValidationTest, MustUseFunction) {
    Func(Source{{56, 78}}, "fn_must_use", utils::Empty, ty.i32(), utils::Vector{Return(1_i)},
         utils::Vector{MustUse()});
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{CallStmt(Call(Source{{12, 34}}, "fn_must_use"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: ignoring return value of function 'fn_must_use' annotated with @must_use
56:78 note: function 'fn_must_use' declared here)");
}

TEST_F(ResolverCallValidationTest, MustUseBuiltin) {
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{CallStmt(Call(Source{{12, 34}}, "max", 1_a, 2_a))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: ignoring return value of builtin 'max'");
}

TEST_F(ResolverCallValidationTest, UnexpectedFunctionTemplateArgs) {
    // fn a() {}
    // fn b() {
    //   a<i32>();
    // }
    Func(Source{{56, 78}}, "a", utils::Empty, ty.void_(), utils::Empty);
    Func("b", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call(Ident(Source{{12, 34}}, "a", "i32"))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: function 'a' does not take template arguments
56:78 note: function 'a' declared here)");
}

TEST_F(ResolverCallValidationTest, UnexpectedBuiltinTemplateArgs) {
    // fn f() {
    //   min<i32>(1, 2);
    // }
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", Call(Ident(Source{{12, 34}}, "min", "i32"), 1_a, 2_a))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: builtin 'min' does not take template arguments)");
}

}  // namespace
}  // namespace tint::resolver
