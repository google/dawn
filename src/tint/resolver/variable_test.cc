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
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/reference.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::number_suffixes;  // NOLINT

struct ResolverVariableTest : public resolver::TestHelper, public testing::Test {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function-scope 'var'
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, LocalVar_NoConstructor) {
    // struct S { i : i32; }
    // alias A = S;
    // fn F(){
    //   var i : i32;
    //   var u : u32;
    //   var f : f32;
    //   var h : f16;
    //   var b : bool;
    //   var s : S;
    //   var a : A;
    // }

    Enable(ast::Extension::kF16);

    auto* S = Structure("S", utils::Vector{Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));

    auto* i = Var("i", ty.i32());
    auto* u = Var("u", ty.u32());
    auto* f = Var("f", ty.f32());
    auto* h = Var("h", ty.f16());
    auto* b = Var("b", ty.bool_());
    auto* s = Var("s", ty.Of(S));
    auto* a = Var("a", ty.Of(A));

    Func("F", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(i),
             Decl(u),
             Decl(f),
             Decl(h),
             Decl(b),
             Decl(s),
             Decl(a),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    // `var` declarations are always of reference type
    ASSERT_TRUE(TypeOf(i)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(u)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(f)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(h)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(b)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(s)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(a)->Is<sem::Reference>());

    EXPECT_TRUE(TypeOf(i)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(u)->As<sem::Reference>()->StoreType()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(f)->As<sem::Reference>()->StoreType()->Is<sem::F32>());
    EXPECT_TRUE(TypeOf(h)->As<sem::Reference>()->StoreType()->Is<sem::F16>());
    EXPECT_TRUE(TypeOf(b)->As<sem::Reference>()->StoreType()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(s)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());
    EXPECT_TRUE(TypeOf(a)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());

    EXPECT_EQ(Sem().Get(i)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(u)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(f)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(h)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(b)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(s)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(a)->Constructor(), nullptr);
}

TEST_F(ResolverVariableTest, LocalVar_WithConstructor) {
    // struct S { i : i32; }
    // alias A = S;
    // fn F(){
    //   var i : i32 = 1i;
    //   var u : u32 = 1u;
    //   var f : f32 = 1.f;
    //   var h : f16 = 1.h;
    //   var b : bool = true;
    //   var s : S = S(1);
    //   var a : A = A(1);
    // }

    Enable(ast::Extension::kF16);

    auto* S = Structure("S", utils::Vector{Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));

    auto* i_c = Expr(1_i);
    auto* u_c = Expr(1_u);
    auto* f_c = Expr(1_f);
    auto* h_c = Expr(1_h);
    auto* b_c = Expr(true);
    auto* s_c = Construct(ty.Of(S), Expr(1_i));
    auto* a_c = Construct(ty.Of(A), Expr(1_i));

    auto* i = Var("i", ty.i32(), i_c);
    auto* u = Var("u", ty.u32(), u_c);
    auto* f = Var("f", ty.f32(), f_c);
    auto* h = Var("h", ty.f16(), h_c);
    auto* b = Var("b", ty.bool_(), b_c);
    auto* s = Var("s", ty.Of(S), s_c);
    auto* a = Var("a", ty.Of(A), a_c);

    Func("F", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(i),
             Decl(u),
             Decl(f),
             Decl(h),
             Decl(b),
             Decl(s),
             Decl(a),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    // `var` declarations are always of reference type
    ASSERT_TRUE(TypeOf(i)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(u)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(f)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(h)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(b)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(s)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(a)->Is<sem::Reference>());

    EXPECT_EQ(TypeOf(i)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(u)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(f)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(b)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(s)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(a)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);

    EXPECT_TRUE(TypeOf(i)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(u)->As<sem::Reference>()->StoreType()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(f)->As<sem::Reference>()->StoreType()->Is<sem::F32>());
    EXPECT_TRUE(TypeOf(h)->As<sem::Reference>()->StoreType()->Is<sem::F16>());
    EXPECT_TRUE(TypeOf(b)->As<sem::Reference>()->StoreType()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(s)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());
    EXPECT_TRUE(TypeOf(a)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());

    EXPECT_EQ(Sem().Get(i)->Constructor()->Declaration(), i_c);
    EXPECT_EQ(Sem().Get(u)->Constructor()->Declaration(), u_c);
    EXPECT_EQ(Sem().Get(f)->Constructor()->Declaration(), f_c);
    EXPECT_EQ(Sem().Get(h)->Constructor()->Declaration(), h_c);
    EXPECT_EQ(Sem().Get(b)->Constructor()->Declaration(), b_c);
    EXPECT_EQ(Sem().Get(s)->Constructor()->Declaration(), s_c);
    EXPECT_EQ(Sem().Get(a)->Constructor()->Declaration(), a_c);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsAlias) {
    // type a = i32;
    //
    // fn F() {
    //   var a = false;
    // }

    auto* t = Alias("a", ty.i32());
    auto* v = Var("a", Expr(false));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsStruct) {
    // struct a {
    //   m : i32;
    // };
    //
    // fn F() {
    //   var a = true;
    // }

    auto* t = Structure("a", utils::Vector{Member("m", ty.i32())});
    auto* v = Var("a", Expr(false));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsFunction) {
    // fn a() {
    //   var a = true;
    // }

    auto* v = Var("a", Expr(false));
    auto* f = Func("a", utils::Empty, ty.void_(), utils::Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(f);
    ASSERT_NE(func, nullptr);

    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), func);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F() {
    //   var a = a;
    // }

    auto* g = GlobalVar("a", ty.i32(), ast::AddressSpace::kPrivate);
    auto* v = Var("a", Expr("a"));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user_v = Sem().Get<sem::VariableUser>(local->Declaration()->constructor);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsGlobalConst) {
    // const a : i32 = 1i;
    //
    // fn X() {
    //   var a = (a == 123);
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", Expr("a"));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(v)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(v);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user_v = Sem().Get<sem::VariableUser>(local->Declaration()->constructor);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsLocalVar) {
    // fn F() {
    //   var a : i32 = 1i; // x
    //   {
    //     var a = a; // y
    //   }
    // }

    auto* x = Var("a", ty.i32(), Expr(1_i));
    auto* y = Var("a", Expr("a"));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(x), Block(Decl(y))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_x = Sem().Get<sem::LocalVariable>(x);
    auto* local_y = Sem().Get<sem::LocalVariable>(y);

    ASSERT_NE(local_x, nullptr);
    ASSERT_NE(local_y, nullptr);
    EXPECT_EQ(local_y->Shadows(), local_x);

    auto* user_y = Sem().Get<sem::VariableUser>(local_y->Declaration()->constructor);
    ASSERT_NE(user_y, nullptr);
    EXPECT_EQ(user_y->Variable(), local_x);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsLocalConst) {
    // fn F() {
    //   const a : i32 = 1i;
    //   {
    //     var a = (a == 123);
    //   }
    // }

    auto* c = Const("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", Expr("a"));
    Func("X", utils::Empty, ty.void_(), utils::Vector{Decl(c), Block(Decl(v))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_c = Sem().Get<sem::LocalVariable>(c);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);

    ASSERT_NE(local_c, nullptr);
    ASSERT_NE(local_v, nullptr);
    EXPECT_EQ(local_v->Shadows(), local_c);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->constructor);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), local_c);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsLocalLet) {
    // fn F() {
    //   let a : i32 = 1i;
    //   {
    //     var a = (a == 123);
    //   }
    // }

    auto* l = Let("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", Expr("a"));
    Func("X", utils::Empty, ty.void_(), utils::Vector{Decl(l), Block(Decl(v))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_l = Sem().Get<sem::LocalVariable>(l);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);

    ASSERT_NE(local_l, nullptr);
    ASSERT_NE(local_v, nullptr);
    EXPECT_EQ(local_v->Shadows(), local_l);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->constructor);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), local_l);
}

TEST_F(ResolverVariableTest, LocalVar_ShadowsParam) {
    // fn F(a : i32) {
    //   {
    //     var a = a;
    //   }
    // }

    auto* p = Param("a", ty.i32());
    auto* v = Var("a", Expr("a"));
    Func("X", utils::Vector{p}, ty.void_(), utils::Vector{Block(Decl(v))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* param = Sem().Get<sem::Parameter>(p);
    auto* local = Sem().Get<sem::LocalVariable>(v);

    ASSERT_NE(param, nullptr);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), param);

    auto* user_v = Sem().Get<sem::VariableUser>(local->Declaration()->constructor);
    ASSERT_NE(user_v, nullptr);
    EXPECT_EQ(user_v->Variable(), param);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function-scope 'let'
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, LocalLet) {
    // struct S { i : i32; }
    // fn F(){
    //   var v : i32;
    //   let i : i32 = 1i;
    //   let u : u32 = 1u;
    //   let f : f32 = 1.f;
    //   let h : h32 = 1.h;
    //   let b : bool = true;
    //   let s : S = S(1);
    //   let a : A = A(1);
    //   let p : pointer<function, i32> = &v;
    // }

    Enable(ast::Extension::kF16);

    auto* S = Structure("S", utils::Vector{Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));
    auto* v = Var("v", ty.i32());

    auto* i_c = Expr(1_i);
    auto* u_c = Expr(1_u);
    auto* f_c = Expr(1_f);
    auto* h_c = Expr(1_h);
    auto* b_c = Expr(true);
    auto* s_c = Construct(ty.Of(S), Expr(1_i));
    auto* a_c = Construct(ty.Of(A), Expr(1_i));
    auto* p_c = AddressOf(v);

    auto* i = Let("i", ty.i32(), i_c);
    auto* u = Let("u", ty.u32(), u_c);
    auto* f = Let("f", ty.f32(), f_c);
    auto* h = Let("h", ty.f16(), h_c);
    auto* b = Let("b", ty.bool_(), b_c);
    auto* s = Let("s", ty.Of(S), s_c);
    auto* a = Let("a", ty.Of(A), a_c);
    auto* p = Let("p", ty.pointer<i32>(ast::AddressSpace::kFunction), p_c);

    Func("F", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(v),
             Decl(i),
             Decl(u),
             Decl(f),
             Decl(h),
             Decl(b),
             Decl(s),
             Decl(a),
             Decl(p),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    // `let` declarations are always of the storage type
    ASSERT_TRUE(TypeOf(i)->Is<sem::I32>());
    ASSERT_TRUE(TypeOf(u)->Is<sem::U32>());
    ASSERT_TRUE(TypeOf(f)->Is<sem::F32>());
    ASSERT_TRUE(TypeOf(h)->Is<sem::F16>());
    ASSERT_TRUE(TypeOf(b)->Is<sem::Bool>());
    ASSERT_TRUE(TypeOf(s)->Is<sem::Struct>());
    ASSERT_TRUE(TypeOf(a)->Is<sem::Struct>());
    ASSERT_TRUE(TypeOf(p)->Is<sem::Pointer>());
    ASSERT_TRUE(TypeOf(p)->As<sem::Pointer>()->StoreType()->Is<sem::I32>());

    EXPECT_EQ(Sem().Get(i)->Constructor()->Declaration(), i_c);
    EXPECT_EQ(Sem().Get(u)->Constructor()->Declaration(), u_c);
    EXPECT_EQ(Sem().Get(f)->Constructor()->Declaration(), f_c);
    EXPECT_EQ(Sem().Get(h)->Constructor()->Declaration(), h_c);
    EXPECT_EQ(Sem().Get(b)->Constructor()->Declaration(), b_c);
    EXPECT_EQ(Sem().Get(s)->Constructor()->Declaration(), s_c);
    EXPECT_EQ(Sem().Get(a)->Constructor()->Declaration(), a_c);
    EXPECT_EQ(Sem().Get(p)->Constructor()->Declaration(), p_c);
}

TEST_F(ResolverVariableTest, LocalLet_InheritsAccessFromOriginatingVariable) {
    // struct Inner {
    //    arr: array<i32, 4>;
    // }
    // struct S {
    //    inner: Inner;
    // }
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // fn f() {
    //   let p = &s.inner.arr[3];
    // }
    auto* inner = Structure("Inner", utils::Vector{Member("arr", ty.array<i32, 4>())});
    auto* buf = Structure("S", utils::Vector{Member("inner", ty.Of(inner))});
    auto* storage = GlobalVar("s", ty.Of(buf), ast::AddressSpace::kStorage, ast::Access::kReadWrite,
                              Binding(0_a), Group(0_a));

    auto* expr = IndexAccessor(MemberAccessor(MemberAccessor(storage, "inner"), "arr"), 3_i);
    auto* ptr = Let("p", AddressOf(expr));

    WrapInFunction(ptr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(expr)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(ptr)->Is<sem::Pointer>());

    EXPECT_EQ(TypeOf(expr)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(ptr)->As<sem::Pointer>()->Access(), ast::Access::kReadWrite);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsAlias) {
    // type a = i32;
    //
    // fn F() {
    //   let a = true;
    // }

    auto* t = Alias("a", ty.i32());
    auto* l = Let("a", Expr(false));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsStruct) {
    // struct a {
    //   m : i32;
    // };
    //
    // fn F() {
    //   let a = false;
    // }

    auto* t = Structure("a", utils::Vector{Member("m", ty.i32())});
    auto* l = Let("a", Expr(false));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsFunction) {
    // fn a() {
    //   let a = false;
    // }

    auto* l = Let("a", Expr(false));
    auto* fb = Func("a", utils::Empty, ty.void_(), utils::Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(fb);
    ASSERT_NE(func, nullptr);

    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), func);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F() {
    //   let a = a;
    // }

    auto* g = GlobalVar("a", ty.i32(), ast::AddressSpace::kPrivate);
    auto* l = Let("a", Expr("a"));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user = Sem().Get<sem::VariableUser>(local->Declaration()->constructor);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsGlobalConst) {
    // const a : i32 = 1i;
    //
    // fn F() {
    //   let a = a;
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* l = Let("a", Expr("a"));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(l);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user = Sem().Get<sem::VariableUser>(local->Declaration()->constructor);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsLocalVar) {
    // fn F() {
    //   var a : i32 = 1i;
    //   {
    //     let a = a;
    //   }
    // }

    auto* v = Var("a", ty.i32(), Expr(1_i));
    auto* l = Let("a", Expr("a"));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(v), Block(Decl(l))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);
    EXPECT_EQ(local_l->Shadows(), local_v);

    auto* user = Sem().Get<sem::VariableUser>(local_l->Declaration()->constructor);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), local_v);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsLocalConst) {
    // fn X() {
    //   const a : i32 = 1i; // x
    //   {
    //     let a = a; // y
    //   }
    // }

    auto* x = Const("a", ty.i32(), Expr(1_i));
    auto* y = Let("a", Expr("a"));
    Func("X", utils::Empty, ty.void_(), utils::Vector{Decl(x), Block(Decl(y))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_x = Sem().Get<sem::LocalVariable>(x);
    auto* local_y = Sem().Get<sem::LocalVariable>(y);

    ASSERT_NE(local_x, nullptr);
    ASSERT_NE(local_y, nullptr);
    EXPECT_EQ(local_y->Shadows(), local_x);

    auto* user = Sem().Get<sem::VariableUser>(local_y->Declaration()->constructor);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), local_x);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsLocalLet) {
    // fn X() {
    //   let a : i32 = 1i; // x
    //   {
    //     let a = a; // y
    //   }
    // }

    auto* x = Let("a", ty.i32(), Expr(1_i));
    auto* y = Let("a", Expr("a"));
    Func("X", utils::Empty, ty.void_(), utils::Vector{Decl(x), Block(Decl(y))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_x = Sem().Get<sem::LocalVariable>(x);
    auto* local_y = Sem().Get<sem::LocalVariable>(y);

    ASSERT_NE(local_x, nullptr);
    ASSERT_NE(local_y, nullptr);
    EXPECT_EQ(local_y->Shadows(), local_x);

    auto* user = Sem().Get<sem::VariableUser>(local_y->Declaration()->constructor);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), local_x);
}

TEST_F(ResolverVariableTest, LocalLet_ShadowsParam) {
    // fn F(a : i32) {
    //   {
    //     let a = a;
    //   }
    // }

    auto* p = Param("a", ty.i32());
    auto* l = Let("a", Expr("a"));
    Func("X", utils::Vector{p}, ty.void_(), utils::Vector{Block(Decl(l))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* param = Sem().Get<sem::Parameter>(p);
    auto* local = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(param, nullptr);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), param);

    auto* user = Sem().Get<sem::VariableUser>(local->Declaration()->constructor);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), param);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function-scope const
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, LocalConst_ShadowsAlias) {
    // type a = i32;
    //
    // fn F() {
    //   const a = true;
    // }

    auto* t = Alias("a", ty.i32());
    auto* c = Const("a", Expr(false));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsStruct) {
    // struct a {
    //   m : i32;
    // };
    //
    // fn F() {
    //   const a = false;
    // }

    auto* t = Structure("a", utils::Vector{Member("m", ty.i32())});
    auto* c = Const("a", Expr(false));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), type_t);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsFunction) {
    // fn a() {
    //   const a = false;
    // }

    auto* c = Const("a", Expr(false));
    auto* fb = Func("a", utils::Empty, ty.void_(), utils::Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(fb);
    ASSERT_NE(func, nullptr);

    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), func);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F() {
    //   const a = 1i;
    // }

    auto* g = GlobalVar("a", ty.i32(), ast::AddressSpace::kPrivate);
    auto* c = Const("a", Expr(1_i));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsGlobalConst) {
    // const a : i32 = 1i;
    //
    // fn F() {
    //   const a = a;
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* c = Const("a", Expr("a"));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(c)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local = Sem().Get<sem::LocalVariable>(c);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), global);

    auto* user = Sem().Get<sem::VariableUser>(local->Declaration()->constructor);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), global);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsLocalVar) {
    // fn F() {
    //   var a = 1i;
    //   {
    //     const a = 1i;
    //   }
    // }

    auto* v = Var("a", ty.i32(), Expr(1_i));
    auto* c = Const("a", Expr(1_i));
    Func("F", utils::Empty, ty.void_(), utils::Vector{Decl(v), Block(Decl(c))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_c = Sem().Get<sem::LocalVariable>(c);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_c, nullptr);
    EXPECT_EQ(local_c->Shadows(), local_v);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsLocalConst) {
    // fn X() {
    //   const a = 1i; // x
    //   {
    //     const a = a; // y
    //   }
    // }

    auto* x = Const("a", ty.i32(), Expr(1_i));
    auto* y = Const("a", Expr("a"));
    Func("X", utils::Empty, ty.void_(), utils::Vector{Decl(x), Block(Decl(y))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_x = Sem().Get<sem::LocalVariable>(x);
    auto* local_y = Sem().Get<sem::LocalVariable>(y);

    ASSERT_NE(local_x, nullptr);
    ASSERT_NE(local_y, nullptr);
    EXPECT_EQ(local_y->Shadows(), local_x);

    auto* user = Sem().Get<sem::VariableUser>(local_y->Declaration()->constructor);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->Variable(), local_x);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsLocalLet) {
    // fn X() {
    //   let a = 1i; // x
    //   {
    //     const a = 1i; // y
    //   }
    // }

    auto* l = Let("a", ty.i32(), Expr(1_i));
    auto* c = Const("a", Expr(1_i));
    Func("X", utils::Empty, ty.void_(), utils::Vector{Decl(l), Block(Decl(c))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_l = Sem().Get<sem::LocalVariable>(l);
    auto* local_c = Sem().Get<sem::LocalVariable>(c);

    ASSERT_NE(local_l, nullptr);
    ASSERT_NE(local_c, nullptr);
    EXPECT_EQ(local_c->Shadows(), local_l);
}

TEST_F(ResolverVariableTest, LocalConst_ShadowsParam) {
    // fn F(a : i32) {
    //   {
    //     const a = 1i;
    //   }
    // }

    auto* p = Param("a", ty.i32());
    auto* c = Const("a", Expr(1_i));
    Func("X", utils::Vector{p}, ty.void_(), utils::Vector{Block(Decl(c))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* param = Sem().Get<sem::Parameter>(p);
    auto* local = Sem().Get<sem::LocalVariable>(c);

    ASSERT_NE(param, nullptr);
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->Shadows(), param);
}

TEST_F(ResolverVariableTest, LocalConst_ExplicitType_Decls) {
    Structure("S", utils::Vector{Member("m", ty.u32())});

    auto* c_i32 = Const("a", ty.i32(), Expr(0_i));
    auto* c_u32 = Const("b", ty.u32(), Expr(0_u));
    auto* c_f32 = Const("c", ty.f32(), Expr(0_f));
    auto* c_vi32 = Const("d", ty.vec3<i32>(), vec3<i32>());
    auto* c_vu32 = Const("e", ty.vec3<u32>(), vec3<u32>());
    auto* c_vf32 = Const("f", ty.vec3<f32>(), vec3<f32>());
    auto* c_mf32 = Const("g", ty.mat3x3<f32>(), mat3x3<f32>());
    auto* c_s = Const("h", ty.type_name("S"), Construct(ty.type_name("S")));

    WrapInFunction(c_i32, c_u32, c_f32, c_vi32, c_vu32, c_vf32, c_mf32, c_s);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(c_i32)->Declaration(), c_i32);
    EXPECT_EQ(Sem().Get(c_u32)->Declaration(), c_u32);
    EXPECT_EQ(Sem().Get(c_f32)->Declaration(), c_f32);
    EXPECT_EQ(Sem().Get(c_vi32)->Declaration(), c_vi32);
    EXPECT_EQ(Sem().Get(c_vu32)->Declaration(), c_vu32);
    EXPECT_EQ(Sem().Get(c_vf32)->Declaration(), c_vf32);
    EXPECT_EQ(Sem().Get(c_mf32)->Declaration(), c_mf32);
    EXPECT_EQ(Sem().Get(c_s)->Declaration(), c_s);

    ASSERT_TRUE(TypeOf(c_i32)->Is<sem::I32>());
    ASSERT_TRUE(TypeOf(c_u32)->Is<sem::U32>());
    ASSERT_TRUE(TypeOf(c_f32)->Is<sem::F32>());
    ASSERT_TRUE(TypeOf(c_vi32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vu32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vf32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_mf32)->Is<sem::Matrix>());
    ASSERT_TRUE(TypeOf(c_s)->Is<sem::Struct>());

    EXPECT_TRUE(Sem().Get(c_i32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_u32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_f32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vi32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vu32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_mf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_s)->ConstantValue()->AllZero());
}

TEST_F(ResolverVariableTest, LocalConst_ImplicitType_Decls) {
    Structure("S", utils::Vector{Member("m", ty.u32())});

    auto* c_i32 = Const("a", Expr(0_i));
    auto* c_u32 = Const("b", Expr(0_u));
    auto* c_f32 = Const("c", Expr(0_f));
    auto* c_ai = Const("d", Expr(0_a));
    auto* c_af = Const("e", Expr(0._a));
    auto* c_vi32 = Const("f", vec3<i32>());
    auto* c_vu32 = Const("g", vec3<u32>());
    auto* c_vf32 = Const("h", vec3<f32>());
    auto* c_vai = Const("i", Construct(ty.vec(nullptr, 3), Expr(0_a)));
    auto* c_vaf = Const("j", Construct(ty.vec(nullptr, 3), Expr(0._a)));
    auto* c_mf32 = Const("k", mat3x3<f32>());
    auto* c_maf32 = Const("l", Construct(ty.mat(nullptr, 3, 3),  //
                                         Construct(ty.vec(nullptr, 3), Expr(0._a)),
                                         Construct(ty.vec(nullptr, 3), Expr(0._a)),
                                         Construct(ty.vec(nullptr, 3), Expr(0._a))));
    auto* c_s = Const("m", Construct(ty.type_name("S")));

    WrapInFunction(c_i32, c_u32, c_f32, c_ai, c_af, c_vi32, c_vu32, c_vf32, c_vai, c_vaf, c_mf32,
                   c_maf32, c_s);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(c_i32)->Declaration(), c_i32);
    EXPECT_EQ(Sem().Get(c_u32)->Declaration(), c_u32);
    EXPECT_EQ(Sem().Get(c_f32)->Declaration(), c_f32);
    EXPECT_EQ(Sem().Get(c_ai)->Declaration(), c_ai);
    EXPECT_EQ(Sem().Get(c_af)->Declaration(), c_af);
    EXPECT_EQ(Sem().Get(c_vi32)->Declaration(), c_vi32);
    EXPECT_EQ(Sem().Get(c_vu32)->Declaration(), c_vu32);
    EXPECT_EQ(Sem().Get(c_vf32)->Declaration(), c_vf32);
    EXPECT_EQ(Sem().Get(c_vai)->Declaration(), c_vai);
    EXPECT_EQ(Sem().Get(c_vaf)->Declaration(), c_vaf);
    EXPECT_EQ(Sem().Get(c_mf32)->Declaration(), c_mf32);
    EXPECT_EQ(Sem().Get(c_maf32)->Declaration(), c_maf32);
    EXPECT_EQ(Sem().Get(c_s)->Declaration(), c_s);

    ASSERT_TRUE(TypeOf(c_i32)->Is<sem::I32>());
    ASSERT_TRUE(TypeOf(c_u32)->Is<sem::U32>());
    ASSERT_TRUE(TypeOf(c_f32)->Is<sem::F32>());
    ASSERT_TRUE(TypeOf(c_ai)->Is<sem::AbstractInt>());
    ASSERT_TRUE(TypeOf(c_af)->Is<sem::AbstractFloat>());
    ASSERT_TRUE(TypeOf(c_vi32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vu32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vf32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vai)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vaf)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_mf32)->Is<sem::Matrix>());
    ASSERT_TRUE(TypeOf(c_maf32)->Is<sem::Matrix>());
    ASSERT_TRUE(TypeOf(c_s)->Is<sem::Struct>());

    EXPECT_TRUE(Sem().Get(c_i32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_u32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_f32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_ai)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_af)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vi32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vu32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vai)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vaf)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_mf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_maf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_s)->ConstantValue()->AllZero());
}

TEST_F(ResolverVariableTest, LocalConst_PropagateConstValue) {
    auto* a = Const("a", Expr(42_i));
    auto* b = Const("b", Expr("a"));
    auto* c = Const("c", Expr("b"));

    WrapInFunction(a, b, c);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(c)->Is<sem::I32>());

    EXPECT_EQ(Sem().Get(c)->ConstantValue()->As<i32>(), 42_i);
}

TEST_F(ResolverVariableTest, LocalConst_ConstEval) {
    auto* c = Const("c", Div(Mul(Add(1_i, 2_i), 3_i), 3_i));

    WrapInFunction(c);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(c)->Is<sem::I32>());

    EXPECT_EQ(Sem().Get(c)->ConstantValue()->As<i32>(), 3_i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Module-scope 'var'
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, GlobalVar_AddressSpace) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

    auto* buf = Structure("S", utils::Vector{Member("m", ty.i32())});
    auto* private_ = GlobalVar("p", ty.i32(), ast::AddressSpace::kPrivate);
    auto* workgroup = GlobalVar("w", ty.i32(), ast::AddressSpace::kWorkgroup);
    auto* uniform =
        GlobalVar("ub", ty.Of(buf), ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));
    auto* storage =
        GlobalVar("sb", ty.Of(buf), ast::AddressSpace::kStorage, Binding(1_a), Group(0_a));
    auto* handle =
        GlobalVar("h", ty.depth_texture(ast::TextureDimension::k2d), Binding(2_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(private_)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(workgroup)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(uniform)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(storage)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(handle)->Is<sem::Reference>());

    EXPECT_EQ(TypeOf(private_)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(workgroup)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(uniform)->As<sem::Reference>()->Access(), ast::Access::kRead);
    EXPECT_EQ(TypeOf(storage)->As<sem::Reference>()->Access(), ast::Access::kRead);
    EXPECT_EQ(TypeOf(handle)->As<sem::Reference>()->Access(), ast::Access::kRead);
}

TEST_F(ResolverVariableTest, GlobalVar_ExplicitAddressSpace) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

    auto* buf = Structure("S", utils::Vector{Member("m", ty.i32())});
    auto* storage = GlobalVar("sb", ty.Of(buf), ast::AddressSpace::kStorage,
                              ast::Access::kReadWrite, Binding(1_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(storage)->Is<sem::Reference>());

    EXPECT_EQ(TypeOf(storage)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Module-scope const
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, GlobalConst_ExplicitType_Decls) {
    auto* c_i32 = GlobalConst("a", ty.i32(), Expr(0_i));
    auto* c_u32 = GlobalConst("b", ty.u32(), Expr(0_u));
    auto* c_f32 = GlobalConst("c", ty.f32(), Expr(0_f));
    auto* c_vi32 = GlobalConst("d", ty.vec3<i32>(), vec3<i32>());
    auto* c_vu32 = GlobalConst("e", ty.vec3<u32>(), vec3<u32>());
    auto* c_vf32 = GlobalConst("f", ty.vec3<f32>(), vec3<f32>());
    auto* c_mf32 = GlobalConst("g", ty.mat3x3<f32>(), mat3x3<f32>());

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(c_i32)->Declaration(), c_i32);
    EXPECT_EQ(Sem().Get(c_u32)->Declaration(), c_u32);
    EXPECT_EQ(Sem().Get(c_f32)->Declaration(), c_f32);
    EXPECT_EQ(Sem().Get(c_vi32)->Declaration(), c_vi32);
    EXPECT_EQ(Sem().Get(c_vu32)->Declaration(), c_vu32);
    EXPECT_EQ(Sem().Get(c_vf32)->Declaration(), c_vf32);
    EXPECT_EQ(Sem().Get(c_mf32)->Declaration(), c_mf32);

    ASSERT_TRUE(TypeOf(c_i32)->Is<sem::I32>());
    ASSERT_TRUE(TypeOf(c_u32)->Is<sem::U32>());
    ASSERT_TRUE(TypeOf(c_f32)->Is<sem::F32>());
    ASSERT_TRUE(TypeOf(c_vi32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vu32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vf32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_mf32)->Is<sem::Matrix>());

    EXPECT_TRUE(Sem().Get(c_i32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_u32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_f32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vi32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vu32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_mf32)->ConstantValue()->AllZero());
}

TEST_F(ResolverVariableTest, GlobalConst_ImplicitType_Decls) {
    auto* c_i32 = GlobalConst("a", Expr(0_i));
    auto* c_u32 = GlobalConst("b", Expr(0_u));
    auto* c_f32 = GlobalConst("c", Expr(0_f));
    auto* c_ai = GlobalConst("d", Expr(0_a));
    auto* c_af = GlobalConst("e", Expr(0._a));
    auto* c_vi32 = GlobalConst("f", vec3<i32>());
    auto* c_vu32 = GlobalConst("g", vec3<u32>());
    auto* c_vf32 = GlobalConst("h", vec3<f32>());
    auto* c_vai = GlobalConst("i", Construct(ty.vec(nullptr, 3), Expr(0_a)));
    auto* c_vaf = GlobalConst("j", Construct(ty.vec(nullptr, 3), Expr(0._a)));
    auto* c_mf32 = GlobalConst("k", mat3x3<f32>());
    auto* c_maf32 = GlobalConst("l", Construct(ty.mat(nullptr, 3, 3),  //
                                               Construct(ty.vec(nullptr, 3), Expr(0._a)),
                                               Construct(ty.vec(nullptr, 3), Expr(0._a)),
                                               Construct(ty.vec(nullptr, 3), Expr(0._a))));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(c_i32)->Declaration(), c_i32);
    EXPECT_EQ(Sem().Get(c_u32)->Declaration(), c_u32);
    EXPECT_EQ(Sem().Get(c_f32)->Declaration(), c_f32);
    EXPECT_EQ(Sem().Get(c_ai)->Declaration(), c_ai);
    EXPECT_EQ(Sem().Get(c_af)->Declaration(), c_af);
    EXPECT_EQ(Sem().Get(c_vi32)->Declaration(), c_vi32);
    EXPECT_EQ(Sem().Get(c_vu32)->Declaration(), c_vu32);
    EXPECT_EQ(Sem().Get(c_vf32)->Declaration(), c_vf32);
    EXPECT_EQ(Sem().Get(c_vai)->Declaration(), c_vai);
    EXPECT_EQ(Sem().Get(c_vaf)->Declaration(), c_vaf);
    EXPECT_EQ(Sem().Get(c_mf32)->Declaration(), c_mf32);
    EXPECT_EQ(Sem().Get(c_maf32)->Declaration(), c_maf32);

    ASSERT_TRUE(TypeOf(c_i32)->Is<sem::I32>());
    ASSERT_TRUE(TypeOf(c_u32)->Is<sem::U32>());
    ASSERT_TRUE(TypeOf(c_f32)->Is<sem::F32>());
    ASSERT_TRUE(TypeOf(c_ai)->Is<sem::AbstractInt>());
    ASSERT_TRUE(TypeOf(c_af)->Is<sem::AbstractFloat>());
    ASSERT_TRUE(TypeOf(c_vi32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vu32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vf32)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vai)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_vaf)->Is<sem::Vector>());
    ASSERT_TRUE(TypeOf(c_mf32)->Is<sem::Matrix>());
    ASSERT_TRUE(TypeOf(c_maf32)->Is<sem::Matrix>());

    EXPECT_TRUE(Sem().Get(c_i32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_u32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_f32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_ai)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_af)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vi32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vu32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vai)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_vaf)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_mf32)->ConstantValue()->AllZero());
    EXPECT_TRUE(Sem().Get(c_maf32)->ConstantValue()->AllZero());
}

TEST_F(ResolverVariableTest, GlobalConst_PropagateConstValue) {
    GlobalConst("b", Expr("a"));
    auto* c = GlobalConst("c", Expr("b"));
    GlobalConst("a", Expr(42_i));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(c)->Is<sem::I32>());

    EXPECT_EQ(Sem().Get(c)->ConstantValue()->As<i32>(), 42_i);
}

TEST_F(ResolverVariableTest, GlobalConst_ConstEval) {
    auto* c = GlobalConst("c", Div(Mul(Add(1_i, 2_i), 3_i), 3_i));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(c)->Is<sem::I32>());

    EXPECT_EQ(Sem().Get(c)->ConstantValue()->As<i32>(), 3_i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function parameter
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(ResolverVariableTest, Param_ShadowsFunction) {
    // fn a(a : bool) {
    // }

    auto* p = Param("a", ty.bool_());
    auto* f = Func("a", utils::Vector{p}, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(f);
    auto* param = Sem().Get<sem::Parameter>(p);

    ASSERT_NE(func, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), func);
}

TEST_F(ResolverVariableTest, Param_ShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F(a : bool) {
    // }

    auto* g = GlobalVar("a", ty.i32(), ast::AddressSpace::kPrivate);
    auto* p = Param("a", ty.bool_());
    Func("F", utils::Vector{p}, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* param = Sem().Get<sem::Parameter>(p);

    ASSERT_NE(global, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), global);
}

TEST_F(ResolverVariableTest, Param_ShadowsGlobalConst) {
    // const a : i32 = 1i;
    //
    // fn F(a : bool) {
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* p = Param("a", ty.bool_());
    Func("F", utils::Vector{p}, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* param = Sem().Get<sem::Parameter>(p);

    ASSERT_NE(global, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), global);
}

TEST_F(ResolverVariableTest, Param_ShadowsAlias) {
    // type a = i32;
    //
    // fn F(a : a) {
    // }

    auto* a = Alias("a", ty.i32());
    auto* p = Param("a", ty.type_name("a"));
    Func("F", utils::Vector{p}, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* alias = Sem().Get(a);
    auto* param = Sem().Get<sem::Parameter>(p);

    ASSERT_NE(alias, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), alias);
    EXPECT_EQ(param->Type(), alias);
}

}  // namespace
}  // namespace tint::resolver
