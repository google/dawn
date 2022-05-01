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
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/reference.h"

namespace tint::resolver {
namespace {

using ResolverIndexAccessorTest = ResolverTest;

TEST_F(ResolverIndexAccessorTest, Matrix_Dynamic_F32) {
    Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kPrivate);
    auto* acc = IndexAccessor("my_var", Expr(Source{{12, 34}}, 1.0f));
    WrapInFunction(acc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Matrix_Dynamic_Ref) {
    Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kPrivate);
    auto* idx = Var("idx", ty.i32(), Construct(ty.i32()));
    auto* acc = IndexAccessor("my_var", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimensions_Dynamic_Ref) {
    Global("my_var", ty.mat4x4<f32>(), ast::StorageClass::kPrivate);
    auto* idx = Var("idx", ty.u32(), Expr(3u));
    auto* idy = Var("idy", ty.u32(), Expr(2u));
    auto* acc = IndexAccessor(IndexAccessor("my_var", idx), idy);
    WrapInFunction(Decl(idx), Decl(idy), acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIndexAccessorTest, Matrix_Dynamic) {
    GlobalConst("my_const", ty.mat2x3<f32>(), Construct(ty.mat2x3<f32>()));
    auto* idx = Var("idx", ty.i32(), Construct(ty.i32()));
    auto* acc = IndexAccessor("my_const", Expr(Source{{12, 34}}, idx));
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverIndexAccessorTest, Matrix_XDimension_Dynamic) {
    GlobalConst("my_var", ty.mat4x4<f32>(), Construct(ty.mat4x4<f32>()));
    auto* idx = Var("idx", ty.u32(), Expr(3u));
    auto* acc = IndexAccessor("my_var", Expr(Source{{12, 34}}, idx));
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimension_Dynamic) {
    GlobalConst("my_var", ty.mat4x4<f32>(), Construct(ty.mat4x4<f32>()));
    auto* idx = Var("idy", ty.u32(), Expr(2u));
    auto* acc = IndexAccessor(IndexAccessor("my_var", Expr(Source{{12, 34}}, idx)), 1);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverIndexAccessorTest, Matrix) {
    Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kPrivate);

    auto* acc = IndexAccessor("my_var", 2);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

    auto* ref = TypeOf(acc)->As<sem::Reference>();
    ASSERT_TRUE(ref->StoreType()->Is<sem::Vector>());
    EXPECT_EQ(ref->StoreType()->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimensions) {
    Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kPrivate);

    auto* acc = IndexAccessor(IndexAccessor("my_var", 2), 1);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

    auto* ref = TypeOf(acc)->As<sem::Reference>();
    EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
}

TEST_F(ResolverIndexAccessorTest, Vector_F32) {
    Global("my_var", ty.vec3<f32>(), ast::StorageClass::kPrivate);
    auto* acc = IndexAccessor("my_var", Expr(Source{{12, 34}}, 2.0f));
    WrapInFunction(acc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Vector_Dynamic_Ref) {
    Global("my_var", ty.vec3<f32>(), ast::StorageClass::kPrivate);
    auto* idx = Var("idx", ty.i32(), Expr(2));
    auto* acc = IndexAccessor("my_var", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverIndexAccessorTest, Vector_Dynamic) {
    GlobalConst("my_var", ty.vec3<f32>(), Construct(ty.vec3<f32>()));
    auto* idx = Var("idx", ty.i32(), Expr(2));
    auto* acc = IndexAccessor("my_var", Expr(Source{{12, 34}}, idx));
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverIndexAccessorTest, Vector) {
    Global("my_var", ty.vec3<f32>(), ast::StorageClass::kPrivate);

    auto* acc = IndexAccessor("my_var", 2);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

    auto* ref = TypeOf(acc)->As<sem::Reference>();
    EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
}

TEST_F(ResolverIndexAccessorTest, Array) {
    auto* idx = Expr(2);
    Global("my_var", ty.array<f32, 3>(), ast::StorageClass::kPrivate);

    auto* acc = IndexAccessor("my_var", idx);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

    auto* ref = TypeOf(acc)->As<sem::Reference>();
    EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
}

TEST_F(ResolverIndexAccessorTest, Alias_Array) {
    auto* aary = Alias("myarrty", ty.array<f32, 3>());

    Global("my_var", ty.Of(aary), ast::StorageClass::kPrivate);

    auto* acc = IndexAccessor("my_var", 2);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

    auto* ref = TypeOf(acc)->As<sem::Reference>();
    EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
}

TEST_F(ResolverIndexAccessorTest, Array_Constant) {
    GlobalConst("my_var", ty.array<f32, 3>(), array<f32, 3>());

    auto* acc = IndexAccessor("my_var", 2);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<sem::F32>());
}

TEST_F(ResolverIndexAccessorTest, Array_Dynamic_I32) {
    // let a : array<f32, 3> = 0;
    // var idx : i32 = 0;
    // var f : f32 = a[idx];
    auto* a = Let("a", ty.array<f32, 3>(), array<f32, 3>());
    auto* idx = Var("idx", ty.i32(), Construct(ty.i32()));
    auto* f = Var("f", ty.f32(), IndexAccessor("a", Expr(Source{{12, 34}}, idx)));
    Func("my_func", ast::VariableList{}, ty.void_(),
         {
             Decl(a),
             Decl(idx),
             Decl(f),
         },
         ast::AttributeList{});

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_F32) {
    // let a : array<f32, 3>;
    // var f : f32 = a[2.0f];
    auto* a = Let("a", ty.array<f32, 3>(), array<f32, 3>());
    auto* f = Var("a_2", ty.f32(), IndexAccessor("a", Expr(Source{{12, 34}}, 2.0f)));
    Func("my_func", ast::VariableList{}, ty.void_(),
         {
             Decl(a),
             Decl(f),
         },
         ast::AttributeList{});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_I32) {
    // let a : array<f32, 3>;
    // var f : f32 = a[2];
    auto* a = Let("a", ty.array<f32, 3>(), array<f32, 3>());
    auto* f = Var("a_2", ty.f32(), IndexAccessor("a", 2));
    Func("my_func", ast::VariableList{}, ty.void_(),
         {
             Decl(a),
             Decl(f),
         },
         ast::AttributeList{});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIndexAccessorTest, EXpr_Deref_FuncGoodParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let idx: u32 = u32();
    //     let x: f32 = (*p)[idx];
    //     return x;
    // }
    auto* p = Param("p", ty.pointer(ty.vec4<f32>(), ast::StorageClass::kFunction));
    auto* idx = Let("idx", ty.u32(), Construct(ty.u32()));
    auto* star_p = Deref(p);
    auto* accessor_expr = IndexAccessor(Source{{12, 34}}, star_p, idx);
    auto* x = Var("x", ty.f32(), accessor_expr);
    Func("func", {p}, ty.f32(), {Decl(idx), Decl(x), Return(x)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIndexAccessorTest, EXpr_Deref_FuncBadParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let idx: u32 = u32();
    //     let x: f32 = *p[idx];
    //     return x;
    // }
    auto* p = Param("p", ty.pointer(ty.vec4<f32>(), ast::StorageClass::kFunction));
    auto* idx = Let("idx", ty.u32(), Construct(ty.u32()));
    auto* accessor_expr = IndexAccessor(Source{{12, 34}}, p, idx);
    auto* star_p = Deref(accessor_expr);
    auto* x = Var("x", ty.f32(), star_p);
    Func("func", {p}, ty.f32(), {Decl(idx), Decl(x), Return(x)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot index type 'ptr<function, vec4<f32>, read_write>'");
}

TEST_F(ResolverIndexAccessorTest, Exr_Deref_BadParent) {
    // var param: vec4<f32>
    // let x: f32 = *(&param)[0];
    auto* param = Var("param", ty.vec4<f32>());
    auto* idx = Var("idx", ty.u32(), Construct(ty.u32()));
    auto* addressOf_expr = AddressOf(param);
    auto* accessor_expr = IndexAccessor(Source{{12, 34}}, addressOf_expr, idx);
    auto* star_p = Deref(accessor_expr);
    auto* x = Var("x", ty.f32(), star_p);
    WrapInFunction(param, idx, x);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot index type 'ptr<function, vec4<f32>, read_write>'");
}

}  // namespace
}  // namespace tint::resolver
