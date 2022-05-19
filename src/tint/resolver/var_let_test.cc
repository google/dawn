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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

struct ResolverVarLetTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverVarLetTest, VarDeclWithoutConstructor) {
    // struct S { i : i32; }
    // alias A = S;
    // fn F(){
    //   var i : i32;
    //   var u : u32;
    //   var f : f32;
    //   var b : bool;
    //   var s : S;
    //   var a : A;
    // }

    auto* S = Structure("S", {Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));

    auto* i = Var("i", ty.i32(), ast::StorageClass::kNone);
    auto* u = Var("u", ty.u32(), ast::StorageClass::kNone);
    auto* f = Var("f", ty.f32(), ast::StorageClass::kNone);
    auto* b = Var("b", ty.bool_(), ast::StorageClass::kNone);
    auto* s = Var("s", ty.Of(S), ast::StorageClass::kNone);
    auto* a = Var("a", ty.Of(A), ast::StorageClass::kNone);

    Func("F", {}, ty.void_(),
         {
             Decl(i),
             Decl(u),
             Decl(f),
             Decl(b),
             Decl(s),
             Decl(a),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    // `var` declarations are always of reference type
    ASSERT_TRUE(TypeOf(i)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(u)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(f)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(b)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(s)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(a)->Is<sem::Reference>());

    EXPECT_TRUE(TypeOf(i)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(u)->As<sem::Reference>()->StoreType()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(f)->As<sem::Reference>()->StoreType()->Is<sem::F32>());
    EXPECT_TRUE(TypeOf(b)->As<sem::Reference>()->StoreType()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(s)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());
    EXPECT_TRUE(TypeOf(a)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());

    EXPECT_EQ(Sem().Get(i)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(u)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(f)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(b)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(s)->Constructor(), nullptr);
    EXPECT_EQ(Sem().Get(a)->Constructor(), nullptr);
}

TEST_F(ResolverVarLetTest, VarDeclWithConstructor) {
    // struct S { i : i32; }
    // alias A = S;
    // fn F(){
    //   var i : i32 = 1i;
    //   var u : u32 = 1u;
    //   var f : f32 = 1.f;
    //   var b : bool = true;
    //   var s : S = S(1);
    //   var a : A = A(1);
    // }

    auto* S = Structure("S", {Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));

    auto* i_c = Expr(1_i);
    auto* u_c = Expr(1_u);
    auto* f_c = Expr(1_f);
    auto* b_c = Expr(true);
    auto* s_c = Construct(ty.Of(S), Expr(1_i));
    auto* a_c = Construct(ty.Of(A), Expr(1_i));

    auto* i = Var("i", ty.i32(), ast::StorageClass::kNone, i_c);
    auto* u = Var("u", ty.u32(), ast::StorageClass::kNone, u_c);
    auto* f = Var("f", ty.f32(), ast::StorageClass::kNone, f_c);
    auto* b = Var("b", ty.bool_(), ast::StorageClass::kNone, b_c);
    auto* s = Var("s", ty.Of(S), ast::StorageClass::kNone, s_c);
    auto* a = Var("a", ty.Of(A), ast::StorageClass::kNone, a_c);

    Func("F", {}, ty.void_(),
         {
             Decl(i),
             Decl(u),
             Decl(f),
             Decl(b),
             Decl(s),
             Decl(a),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    // `var` declarations are always of reference type
    ASSERT_TRUE(TypeOf(i)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(u)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(f)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(b)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(s)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(a)->Is<sem::Reference>());

    EXPECT_TRUE(TypeOf(i)->As<sem::Reference>()->StoreType()->Is<sem::I32>());
    EXPECT_TRUE(TypeOf(u)->As<sem::Reference>()->StoreType()->Is<sem::U32>());
    EXPECT_TRUE(TypeOf(f)->As<sem::Reference>()->StoreType()->Is<sem::F32>());
    EXPECT_TRUE(TypeOf(b)->As<sem::Reference>()->StoreType()->Is<sem::Bool>());
    EXPECT_TRUE(TypeOf(s)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());
    EXPECT_TRUE(TypeOf(a)->As<sem::Reference>()->StoreType()->Is<sem::Struct>());

    EXPECT_EQ(Sem().Get(i)->Constructor()->Declaration(), i_c);
    EXPECT_EQ(Sem().Get(u)->Constructor()->Declaration(), u_c);
    EXPECT_EQ(Sem().Get(f)->Constructor()->Declaration(), f_c);
    EXPECT_EQ(Sem().Get(b)->Constructor()->Declaration(), b_c);
    EXPECT_EQ(Sem().Get(s)->Constructor()->Declaration(), s_c);
    EXPECT_EQ(Sem().Get(a)->Constructor()->Declaration(), a_c);
}

TEST_F(ResolverVarLetTest, LetDecl) {
    // struct S { i : i32; }
    // fn F(){
    //   var v : i32;
    //   let i : i32 = 1i;
    //   let u : u32 = 1u;
    //   let f : f32 = 1.;
    //   let b : bool = true;
    //   let s : S = S(1);
    //   let a : A = A(1);
    //   let p : pointer<function, i32> = &v;
    // }

    auto* S = Structure("S", {Member("i", ty.i32())});
    auto* A = Alias("A", ty.Of(S));
    auto* v = Var("v", ty.i32(), ast::StorageClass::kNone);

    auto* i_c = Expr(1_i);
    auto* u_c = Expr(1_u);
    auto* f_c = Expr(1_f);
    auto* b_c = Expr(true);
    auto* s_c = Construct(ty.Of(S), Expr(1_i));
    auto* a_c = Construct(ty.Of(A), Expr(1_i));
    auto* p_c = AddressOf(v);

    auto* i = Let("i", ty.i32(), i_c);
    auto* u = Let("u", ty.u32(), u_c);
    auto* f = Let("f", ty.f32(), f_c);
    auto* b = Let("b", ty.bool_(), b_c);
    auto* s = Let("s", ty.Of(S), s_c);
    auto* a = Let("a", ty.Of(A), a_c);
    auto* p = Let("p", ty.pointer<i32>(ast::StorageClass::kFunction), p_c);

    Func("F", {}, ty.void_(),
         {
             Decl(v),
             Decl(i),
             Decl(u),
             Decl(f),
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
    ASSERT_TRUE(TypeOf(b)->Is<sem::Bool>());
    ASSERT_TRUE(TypeOf(s)->Is<sem::Struct>());
    ASSERT_TRUE(TypeOf(a)->Is<sem::Struct>());
    ASSERT_TRUE(TypeOf(p)->Is<sem::Pointer>());
    ASSERT_TRUE(TypeOf(p)->As<sem::Pointer>()->StoreType()->Is<sem::I32>());

    EXPECT_EQ(Sem().Get(i)->Constructor()->Declaration(), i_c);
    EXPECT_EQ(Sem().Get(u)->Constructor()->Declaration(), u_c);
    EXPECT_EQ(Sem().Get(f)->Constructor()->Declaration(), f_c);
    EXPECT_EQ(Sem().Get(b)->Constructor()->Declaration(), b_c);
    EXPECT_EQ(Sem().Get(s)->Constructor()->Declaration(), s_c);
    EXPECT_EQ(Sem().Get(a)->Constructor()->Declaration(), a_c);
    EXPECT_EQ(Sem().Get(p)->Constructor()->Declaration(), p_c);
}

TEST_F(ResolverVarLetTest, DefaultVarStorageClass) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

    auto* buf = Structure("S", {Member("m", ty.i32())});
    auto* function = Var("f", ty.i32());
    auto* private_ = Global("p", ty.i32(), ast::StorageClass::kPrivate);
    auto* workgroup = Global("w", ty.i32(), ast::StorageClass::kWorkgroup);
    auto* uniform = Global("ub", ty.Of(buf), ast::StorageClass::kUniform,
                           ast::AttributeList{
                               create<ast::BindingAttribute>(0),
                               create<ast::GroupAttribute>(0),
                           });
    auto* storage = Global("sb", ty.Of(buf), ast::StorageClass::kStorage,
                           ast::AttributeList{
                               create<ast::BindingAttribute>(1),
                               create<ast::GroupAttribute>(0),
                           });
    auto* handle = Global("h", ty.depth_texture(ast::TextureDimension::k2d),
                          ast::AttributeList{
                              create<ast::BindingAttribute>(2),
                              create<ast::GroupAttribute>(0),
                          });

    WrapInFunction(function);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(function)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(private_)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(workgroup)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(uniform)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(storage)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(handle)->Is<sem::Reference>());

    EXPECT_EQ(TypeOf(function)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(private_)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(workgroup)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(uniform)->As<sem::Reference>()->Access(), ast::Access::kRead);
    EXPECT_EQ(TypeOf(storage)->As<sem::Reference>()->Access(), ast::Access::kRead);
    EXPECT_EQ(TypeOf(handle)->As<sem::Reference>()->Access(), ast::Access::kRead);
}

TEST_F(ResolverVarLetTest, ExplicitVarStorageClass) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

    auto* buf = Structure("S", {Member("m", ty.i32())});
    auto* storage = Global("sb", ty.Of(buf), ast::StorageClass::kStorage, ast::Access::kReadWrite,
                           ast::AttributeList{
                               create<ast::BindingAttribute>(1),
                               create<ast::GroupAttribute>(0),
                           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(storage)->Is<sem::Reference>());

    EXPECT_EQ(TypeOf(storage)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
}

TEST_F(ResolverVarLetTest, LetInheritsAccessFromOriginatingVariable) {
    // struct Inner {
    //    arr: array<i32, 4>;
    // }
    // struct S {
    //    inner: Inner;
    // }
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // fn f() {
    //   let p = &s.inner.arr[4];
    // }
    auto* inner = Structure("Inner", {Member("arr", ty.array<i32, 4>())});
    auto* buf = Structure("S", {Member("inner", ty.Of(inner))});
    auto* storage = Global("s", ty.Of(buf), ast::StorageClass::kStorage, ast::Access::kReadWrite,
                           ast::AttributeList{
                               create<ast::BindingAttribute>(0),
                               create<ast::GroupAttribute>(0),
                           });

    auto* expr = IndexAccessor(MemberAccessor(MemberAccessor(storage, "inner"), "arr"), 4_i);
    auto* ptr = Let("p", nullptr, AddressOf(expr));

    WrapInFunction(ptr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(expr)->Is<sem::Reference>());
    ASSERT_TRUE(TypeOf(ptr)->Is<sem::Pointer>());

    EXPECT_EQ(TypeOf(expr)->As<sem::Reference>()->Access(), ast::Access::kReadWrite);
    EXPECT_EQ(TypeOf(ptr)->As<sem::Pointer>()->Access(), ast::Access::kReadWrite);
}

TEST_F(ResolverVarLetTest, LocalShadowsAlias) {
    // type a = i32;
    //
    // fn X() {
    //   var a = false;
    // }
    //
    // fn Y() {
    //   let a = true;
    // }

    auto* t = Alias("a", ty.i32());
    auto* v = Var("a", nullptr, Expr(false));
    auto* l = Let("a", nullptr, Expr(false));
    Func("X", {}, ty.void_(), {Decl(v)});
    Func("Y", {}, ty.void_(), {Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);

    EXPECT_EQ(local_v->Shadows(), type_t);
    EXPECT_EQ(local_l->Shadows(), type_t);
}

TEST_F(ResolverVarLetTest, LocalShadowsStruct) {
    // struct a {
    //   m : i32;
    // };
    //
    // fn X() {
    //   var a = true;
    // }
    //
    // fn Y() {
    //   let a = false;
    // }

    auto* t = Structure("a", {Member("m", ty.i32())});
    auto* v = Var("a", nullptr, Expr(false));
    auto* l = Let("a", nullptr, Expr(false));
    Func("X", {}, ty.void_(), {Decl(v)});
    Func("Y", {}, ty.void_(), {Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* type_t = Sem().Get(t);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);

    EXPECT_EQ(local_v->Shadows(), type_t);
    EXPECT_EQ(local_l->Shadows(), type_t);
}

TEST_F(ResolverVarLetTest, LocalShadowsFunction) {
    // fn a() {
    //   var a = true;
    // }
    //
    // fn b() {
    //   let b = false;
    // }

    auto* v = Var("a", nullptr, Expr(false));
    auto* l = Let("b", nullptr, Expr(false));
    auto* fa = Func("a", {}, ty.void_(), {Decl(v)});
    auto* fb = Func("b", {}, ty.void_(), {Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);
    auto* func_a = Sem().Get(fa);
    auto* func_b = Sem().Get(fb);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);
    ASSERT_NE(func_a, nullptr);
    ASSERT_NE(func_b, nullptr);

    EXPECT_EQ(local_v->Shadows(), func_a);
    EXPECT_EQ(local_l->Shadows(), func_b);
}

TEST_F(ResolverVarLetTest, LocalShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn X() {
    //   var a = a;
    // }
    //
    // fn Y() {
    //   let a = a;
    // }

    auto* g = Global("a", ty.i32(), ast::StorageClass::kPrivate);
    auto* v = Var("a", nullptr, Expr("a"));
    auto* l = Let("a", nullptr, Expr("a"));
    Func("X", {}, ty.void_(), {Decl(v)});
    Func("Y", {}, ty.void_(), {Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);

    EXPECT_EQ(local_v->Shadows(), global);
    EXPECT_EQ(local_l->Shadows(), global);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->constructor);
    auto* user_l = Sem().Get<sem::VariableUser>(local_l->Declaration()->constructor);

    ASSERT_NE(user_v, nullptr);
    ASSERT_NE(user_l, nullptr);

    EXPECT_EQ(user_v->Variable(), global);
    EXPECT_EQ(user_l->Variable(), global);
}

TEST_F(ResolverVarLetTest, LocalShadowsGlobalLet) {
    // let a : i32 = 1;
    //
    // fn X() {
    //   var a = (a == 123);
    // }
    //
    // fn Y() {
    //   let a = (a == 321);
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", nullptr, Expr("a"));
    auto* l = Let("a", nullptr, Expr("a"));
    Func("X", {}, ty.void_(), {Decl(v)});
    Func("Y", {}, ty.void_(), {Decl(l)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);

    EXPECT_EQ(local_v->Shadows(), global);
    EXPECT_EQ(local_l->Shadows(), global);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->constructor);
    auto* user_l = Sem().Get<sem::VariableUser>(local_l->Declaration()->constructor);

    ASSERT_NE(user_v, nullptr);
    ASSERT_NE(user_l, nullptr);

    EXPECT_EQ(user_v->Variable(), global);
    EXPECT_EQ(user_l->Variable(), global);
}

TEST_F(ResolverVarLetTest, LocalShadowsLocalVar) {
    // fn X() {
    //   var a : i32;
    //   {
    //     var a = a;
    //   }
    //   {
    //     let a = a;
    //   }
    // }

    auto* s = Var("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", nullptr, Expr("a"));
    auto* l = Let("a", nullptr, Expr("a"));
    Func("X", {}, ty.void_(), {Decl(s), Block(Decl(v)), Block(Decl(l))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_s = Sem().Get<sem::LocalVariable>(s);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(local_s, nullptr);
    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);

    EXPECT_EQ(local_v->Shadows(), local_s);
    EXPECT_EQ(local_l->Shadows(), local_s);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->constructor);
    auto* user_l = Sem().Get<sem::VariableUser>(local_l->Declaration()->constructor);

    ASSERT_NE(user_v, nullptr);
    ASSERT_NE(user_l, nullptr);

    EXPECT_EQ(user_v->Variable(), local_s);
    EXPECT_EQ(user_l->Variable(), local_s);
}

TEST_F(ResolverVarLetTest, LocalShadowsLocalLet) {
    // fn X() {
    //   let a = 1;
    //   {
    //     var a = (a == 123);
    //   }
    //   {
    //     let a = (a == 321);
    //   }
    // }

    auto* s = Let("a", ty.i32(), Expr(1_i));
    auto* v = Var("a", nullptr, Expr("a"));
    auto* l = Let("a", nullptr, Expr("a"));
    Func("X", {}, ty.void_(), {Decl(s), Block(Decl(v)), Block(Decl(l))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* local_s = Sem().Get<sem::LocalVariable>(s);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(local_s, nullptr);
    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);

    EXPECT_EQ(local_v->Shadows(), local_s);
    EXPECT_EQ(local_l->Shadows(), local_s);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->constructor);
    auto* user_l = Sem().Get<sem::VariableUser>(local_l->Declaration()->constructor);

    ASSERT_NE(user_v, nullptr);
    ASSERT_NE(user_l, nullptr);

    EXPECT_EQ(user_v->Variable(), local_s);
    EXPECT_EQ(user_l->Variable(), local_s);
}

TEST_F(ResolverVarLetTest, LocalShadowsParam) {
    // fn F(a : i32) {
    //   {
    //     var a = a;
    //   }
    //   {
    //     let a = a;
    //   }
    // }

    auto* p = Param("a", ty.i32());
    auto* v = Var("a", nullptr, Expr("a"));
    auto* l = Let("a", nullptr, Expr("a"));
    Func("X", {p}, ty.void_(), {Block(Decl(v)), Block(Decl(l))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* param = Sem().Get<sem::Parameter>(p);
    auto* local_v = Sem().Get<sem::LocalVariable>(v);
    auto* local_l = Sem().Get<sem::LocalVariable>(l);

    ASSERT_NE(param, nullptr);
    ASSERT_NE(local_v, nullptr);
    ASSERT_NE(local_l, nullptr);

    EXPECT_EQ(local_v->Shadows(), param);
    EXPECT_EQ(local_l->Shadows(), param);

    auto* user_v = Sem().Get<sem::VariableUser>(local_v->Declaration()->constructor);
    auto* user_l = Sem().Get<sem::VariableUser>(local_l->Declaration()->constructor);

    ASSERT_NE(user_v, nullptr);
    ASSERT_NE(user_l, nullptr);

    EXPECT_EQ(user_v->Variable(), param);
    EXPECT_EQ(user_l->Variable(), param);
}

TEST_F(ResolverVarLetTest, ParamShadowsFunction) {
    // fn a(a : bool) {
    // }

    auto* p = Param("a", ty.bool_());
    auto* f = Func("a", {p}, ty.void_(), {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* func = Sem().Get(f);
    auto* param = Sem().Get<sem::Parameter>(p);

    ASSERT_NE(func, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), func);
}

TEST_F(ResolverVarLetTest, ParamShadowsGlobalVar) {
    // var<private> a : i32;
    //
    // fn F(a : bool) {
    // }

    auto* g = Global("a", ty.i32(), ast::StorageClass::kPrivate);
    auto* p = Param("a", ty.bool_());
    Func("F", {p}, ty.void_(), {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* param = Sem().Get<sem::Parameter>(p);

    ASSERT_NE(global, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), global);
}

TEST_F(ResolverVarLetTest, ParamShadowsGlobalLet) {
    // let a : i32 = 1;
    //
    // fn F(a : bool) {
    // }

    auto* g = GlobalConst("a", ty.i32(), Expr(1_i));
    auto* p = Param("a", ty.bool_());
    Func("F", {p}, ty.void_(), {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get(g);
    auto* param = Sem().Get<sem::Parameter>(p);

    ASSERT_NE(global, nullptr);
    ASSERT_NE(param, nullptr);

    EXPECT_EQ(param->Shadows(), global);
}

TEST_F(ResolverVarLetTest, ParamShadowsAlias) {
    // type a = i32;
    //
    // fn F(a : a) {
    // }

    auto* a = Alias("a", ty.i32());
    auto* p = Param("a", ty.type_name("a"));
    Func("F", {p}, ty.void_(), {});

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
