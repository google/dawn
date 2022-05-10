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

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

struct ResolverVarLetValidationTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverVarLetValidationTest, LetNoInitializer) {
    // let a : i32;
    WrapInFunction(Let(Source{{12, 34}}, "a", ty.i32(), nullptr));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: let declaration must have an initializer");
}

TEST_F(ResolverVarLetValidationTest, GlobalLetNoInitializer) {
    // let a : i32;
    GlobalConst(Source{{12, 34}}, "a", ty.i32(), nullptr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: let declaration must have an initializer");
}

TEST_F(ResolverVarLetValidationTest, VarNoInitializerNoType) {
    // var a;
    WrapInFunction(Var(Source{{12, 34}}, "a", nullptr));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: function scope var declaration requires a type or "
              "initializer");
}

TEST_F(ResolverVarLetValidationTest, GlobalVarNoInitializerNoType) {
    // var a;
    Global(Source{{12, 34}}, "a", nullptr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: module scope var declaration requires a type and "
              "initializer");
}

TEST_F(ResolverVarLetValidationTest, VarTypeNotStorable) {
    // var i : i32;
    // var p : pointer<function, i32> = &v;
    auto* i = Var("i", ty.i32(), ast::StorageClass::kNone);
    auto* p = Var(Source{{56, 78}}, "a", ty.pointer<i32>(ast::StorageClass::kFunction),
                  ast::StorageClass::kNone, AddressOf(Source{{12, 34}}, "i"));
    WrapInFunction(i, p);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: ptr<function, i32, read_write> cannot be used as the "
              "type of a var");
}

TEST_F(ResolverVarLetValidationTest, LetTypeNotConstructible) {
    // @group(0) @binding(0) var t1 : texture_2d<f32>;
    // let t2 : t1;
    auto* t1 = Global("t1", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
                      GroupAndBinding(0, 0));
    auto* t2 = Let(Source{{56, 78}}, "t2", nullptr, Expr(t1));
    WrapInFunction(t2);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: texture_2d<f32> cannot be used as the type of a let");
}

TEST_F(ResolverVarLetValidationTest, LetConstructorWrongType) {
    // var v : i32 = 2u
    WrapInFunction(Let(Source{{3, 3}}, "v", ty.i32(), Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize let of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVarLetValidationTest, VarConstructorWrongType) {
    // var v : i32 = 2u
    WrapInFunction(Var(Source{{3, 3}}, "v", ty.i32(), ast::StorageClass::kNone, Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize var of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVarLetValidationTest, LetConstructorWrongTypeViaAlias) {
    auto* a = Alias("I32", ty.i32());
    WrapInFunction(Let(Source{{3, 3}}, "v", ty.Of(a), Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize let of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVarLetValidationTest, VarConstructorWrongTypeViaAlias) {
    auto* a = Alias("I32", ty.i32());
    WrapInFunction(Var(Source{{3, 3}}, "v", ty.Of(a), ast::StorageClass::kNone, Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize var of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVarLetValidationTest, LetOfPtrConstructedWithRef) {
    // var a : f32;
    // let b : ptr<function,f32> = a;
    const auto priv = ast::StorageClass::kFunction;
    auto* var_a = Var("a", ty.f32(), priv);
    auto* var_b = Let(Source{{12, 34}}, "b", ty.pointer<f32>(priv), Expr("a"), {});
    WrapInFunction(var_a, var_b);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: cannot initialize let of type 'ptr<function, f32, read_write>' with value of type 'f32')");
}

TEST_F(ResolverVarLetValidationTest, LocalLetRedeclared) {
    // let l : f32 = 1.;
    // let l : i32 = 0;
    auto* l1 = Let("l", ty.f32(), Expr(1.f));
    auto* l2 = Let(Source{{12, 34}}, "l", ty.i32(), Expr(0_i));
    WrapInFunction(l1, l2);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: redeclaration of 'l'\nnote: 'l' previously declared here");
}

TEST_F(ResolverVarLetValidationTest, GlobalVarRedeclaredAsLocal) {
    // var v : f32 = 2.1;
    // fn my_func() {
    //   var v : f32 = 2.0;
    //   return 0;
    // }

    Global("v", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

    WrapInFunction(Var(Source{{12, 34}}, "v", ty.f32(), ast::StorageClass::kNone, Expr(2.0f)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverVarLetValidationTest, VarRedeclaredInInnerBlock) {
    // {
    //  var v : f32;
    //  { var v : f32; }
    // }
    auto* var_outer = Var("v", ty.f32(), ast::StorageClass::kNone);
    auto* var_inner = Var(Source{{12, 34}}, "v", ty.f32(), ast::StorageClass::kNone);
    auto* inner = Block(Decl(var_inner));
    auto* outer_body = Block(Decl(var_outer), inner);

    WrapInFunction(outer_body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverVarLetValidationTest, VarRedeclaredInIfBlock) {
    // {
    //   var v : f32 = 3.14;
    //   if (true) { var v : f32 = 2.0; }
    // }
    auto* var_a_float = Var("v", ty.f32(), ast::StorageClass::kNone, Expr(3.1f));

    auto* var = Var(Source{{12, 34}}, "v", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

    auto* cond = Expr(true);
    auto* body = Block(Decl(var));

    auto* outer_body = Block(Decl(var_a_float), If(cond, body));

    WrapInFunction(outer_body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverVarLetValidationTest, InferredPtrStorageAccessMismatch) {
    // struct Inner {
    //    arr: array<i32, 4>;
    // }
    // struct S {
    //    inner: Inner;
    // }
    // @group(0) @binding(0) var<storage> s : S;
    // fn f() {
    //   let p : pointer<storage, i32, read_write> = &s.inner.arr[2i];
    // }
    auto* inner = Structure("Inner", {Member("arr", ty.array<i32, 4>())});
    auto* buf = Structure("S", {Member("inner", ty.Of(inner))});
    auto* storage = Global("s", ty.Of(buf), ast::StorageClass::kStorage,
                           ast::AttributeList{
                               create<ast::BindingAttribute>(0),
                               create<ast::GroupAttribute>(0),
                           });

    auto* expr = IndexAccessor(MemberAccessor(MemberAccessor(storage, "inner"), "arr"), 2_i);
    auto* ptr =
        Let(Source{{12, 34}}, "p",
            ty.pointer<i32>(ast::StorageClass::kStorage, ast::Access::kReadWrite), AddressOf(expr));

    WrapInFunction(ptr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot initialize let of type "
              "'ptr<storage, i32, read_write>' with value of type "
              "'ptr<storage, i32, read>'");
}

TEST_F(ResolverVarLetValidationTest, NonConstructibleType_Atomic) {
    auto* v = Var("v", ty.atomic(Source{{12, 34}}, ty.i32()));
    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function variable must have a constructible type");
}

TEST_F(ResolverVarLetValidationTest, NonConstructibleType_RuntimeArray) {
    auto* s = Structure("S", {Member(Source{{56, 78}}, "m", ty.array(ty.i32()))});
    auto* v = Var(Source{{12, 34}}, "v", ty.Of(s));
    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
56:78 note: while analysing structure member S.m
12:34 note: while instantiating variable v)");
}

TEST_F(ResolverVarLetValidationTest, NonConstructibleType_Struct_WithAtomic) {
    auto* s = Structure("S", {Member("m", ty.atomic(ty.i32()))});
    auto* v = Var("v", ty.Of(s));
    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: function variable must have a constructible type");
}

TEST_F(ResolverVarLetValidationTest, NonConstructibleType_InferredType) {
    // @group(0) @binding(0) var s : sampler;
    // fn foo() {
    //   var v = s;
    // }
    Global("s", ty.sampler(ast::SamplerKind::kSampler), GroupAndBinding(0, 0));
    auto* v = Var(Source{{12, 34}}, "v", nullptr, Expr("s"));
    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function variable must have a constructible type");
}

TEST_F(ResolverVarLetValidationTest, InvalidStorageClassForInitializer) {
    // var<workgroup> v : f32 = 1.23;
    Global(Source{{12, 34}}, "v", ty.f32(), ast::StorageClass::kWorkgroup, Expr(1.23f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: var of storage class 'workgroup' cannot have "
              "an initializer. var initializers are only supported for the "
              "storage classes 'private' and 'function'");
}

TEST_F(ResolverVarLetValidationTest, VectorLetNoType) {
    // let a : mat3x3 = mat3x3<f32>();
    WrapInFunction(Let("a", create<ast::Vector>(Source{{12, 34}}, nullptr, 3), vec3<f32>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing vector element type");
}

TEST_F(ResolverVarLetValidationTest, VectorVarNoType) {
    // var a : mat3x3;
    WrapInFunction(Var("a", create<ast::Vector>(Source{{12, 34}}, nullptr, 3)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing vector element type");
}

TEST_F(ResolverVarLetValidationTest, MatrixLetNoType) {
    // let a : mat3x3 = mat3x3<f32>();
    WrapInFunction(Let("a", create<ast::Matrix>(Source{{12, 34}}, nullptr, 3, 3), mat3x3<f32>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing matrix element type");
}

TEST_F(ResolverVarLetValidationTest, MatrixVarNoType) {
    // var a : mat3x3;
    WrapInFunction(Var("a", create<ast::Matrix>(Source{{12, 34}}, nullptr, 3, 3)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing matrix element type");
}

}  // namespace
}  // namespace tint::resolver
