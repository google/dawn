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
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/type/reference.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverIndexAccessorTest = ResolverTest;

TEST_F(ResolverIndexAccessorTest, Matrix_Dynamic_F32) {
    GlobalVar("my_var", ty.mat2x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", Expr(Source{{12, 34}}, 1_f));
    WrapInFunction(acc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Matrix_Dynamic_Ref) {
    GlobalVar("my_var", ty.mat2x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* idx = Var("idx", ty.i32(), Call<i32>());
    auto* acc = IndexAccessor("my_var", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimensions_Dynamic_Ref) {
    GlobalVar("my_var", ty.mat4x4<f32>(), builtin::AddressSpace::kPrivate);
    auto* idx = Var("idx", ty.u32(), Expr(3_u));
    auto* idy = Var("idy", ty.u32(), Expr(2_u));
    auto* acc = IndexAccessor(IndexAccessor("my_var", idx), idy);
    WrapInFunction(Decl(idx), Decl(idy), acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Matrix_Dynamic) {
    GlobalConst("my_const", ty.mat2x3<f32>(), Call(ty.mat2x3<f32>()));
    auto* idx = Var("idx", ty.i32(), Call<i32>());
    auto* acc = IndexAccessor("my_const", Expr(Source{{12, 34}}, idx));
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Matrix_XDimension_Dynamic) {
    GlobalConst("my_const", ty.mat4x4<f32>(), Call(ty.mat4x4<f32>()));
    auto* idx = Var("idx", ty.u32(), Expr(3_u));
    auto* acc = IndexAccessor("my_const", Expr(Source{{12, 34}}, idx));
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimension_Dynamic) {
    GlobalConst("my_const", ty.mat4x4<f32>(), Call(ty.mat4x4<f32>()));
    auto* idx = Var("idy", ty.u32(), Expr(2_u));
    auto* acc = IndexAccessor(IndexAccessor("my_const", Expr(Source{{12, 34}}, idx)), 1_i);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");
}

TEST_F(ResolverIndexAccessorTest, Matrix) {
    GlobalVar("my_var", ty.mat2x3<f32>(), builtin::AddressSpace::kPrivate);

    auto* acc = IndexAccessor("my_var", 1_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    ASSERT_TRUE(TypeOf(acc)->Is<type::Vector>());
    EXPECT_EQ(TypeOf(acc)->As<type::Vector>()->Width(), 3u);

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Matrix_BothDimensions) {
    GlobalVar("my_var", ty.mat2x3<f32>(), builtin::AddressSpace::kPrivate);

    auto* acc = IndexAccessor(IndexAccessor("my_var", 0_i), 1_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Vector_F32) {
    GlobalVar("my_var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", Expr(Source{{12, 34}}, 2_f));
    WrapInFunction(acc);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Vector_Dynamic_Ref) {
    GlobalVar("my_var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    auto* idx = Var("idx", ty.i32(), Expr(2_i));
    auto* acc = IndexAccessor("my_var", idx);
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Vector_Dynamic) {
    GlobalConst("my_const", ty.vec3<f32>(), Call(ty.vec3<f32>()));
    auto* idx = Var("idx", ty.i32(), Expr(2_i));
    auto* acc = IndexAccessor("my_const", Expr(Source{{12, 34}}, idx));
    WrapInFunction(Decl(idx), acc);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverIndexAccessorTest, Vector) {
    GlobalVar("my_var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* acc = IndexAccessor("my_var", 2_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_i32) {
    GlobalVar("my_var", ty.array<f32, 3>(), builtin::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", 2_i);
    WrapInFunction(acc);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(acc)->Is<type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_u32) {
    GlobalVar("my_var", ty.array<f32, 3>(), builtin::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", 2_u);
    WrapInFunction(acc);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(acc)->Is<type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_AInt) {
    GlobalVar("my_var", ty.array<f32, 3>(), builtin::AddressSpace::kPrivate);
    auto* acc = IndexAccessor("my_var", 2_a);
    WrapInFunction(acc);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(acc)->Is<type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Alias_Array) {
    auto* aary = Alias("myarrty", ty.array<f32, 3>());

    GlobalVar("my_var", ty.Of(aary), builtin::AddressSpace::kPrivate);

    auto* acc = IndexAccessor("my_var", 2_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<type::F32>());

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Constant) {
    GlobalConst("my_const", ty.array<f32, 3>(), array<f32, 3>());

    auto* acc = IndexAccessor("my_const", 2_i);
    WrapInFunction(acc);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(acc), nullptr);
    EXPECT_TRUE(TypeOf(acc)->Is<type::F32>());
}

TEST_F(ResolverIndexAccessorTest, Array_Dynamic_I32) {
    // let a : array<f32, 3> = 0;
    // var idx : i32 = 0;
    // var f : f32 = a[idx];
    auto* a = Let("a", ty.array<f32, 3>(), array<f32, 3>());
    auto* idx = Var("idx", ty.i32(), Call<i32>());
    auto* acc = IndexAccessor("a", Expr(Source{{12, 34}}, idx));
    auto* f = Var("f", ty.f32(), acc);
    Func("my_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(a),
             Decl(idx),
             Decl(f),
         });

    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "");

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_F32) {
    // let a : array<f32, 3>;
    // var f : f32 = a[2.0f];
    auto* a = Let("a", ty.array<f32, 3>(), array<f32, 3>());
    auto* f = Var("a_2", ty.f32(), IndexAccessor("a", Expr(Source{{12, 34}}, 2_f)));
    Func("my_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(a),
             Decl(f),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverIndexAccessorTest, Array_Literal_I32) {
    // let a : array<f32, 3>;
    // var f : f32 = a[2i];
    auto* a = Let("a", ty.array<f32, 3>(), array<f32, 3>());
    auto* acc = IndexAccessor("a", 2_i);
    auto* f = Var("a_2", ty.f32(), acc);
    Func("my_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(a),
             Decl(f),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Expr_Deref_FuncGoodParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let idx: u32 = u32();
    //     let x: f32 = (*p)[idx];
    //     return x;
    // }
    auto* p = Param("p", ty.pointer(ty.vec4<f32>(), builtin::AddressSpace::kFunction));
    auto* idx = Let("idx", ty.u32(), Call<u32>());
    auto* star_p = Deref(p);
    auto* acc = IndexAccessor(Source{{12, 34}}, star_p, idx);
    auto* x = Var("x", ty.f32(), acc);
    Func("func", utils::Vector{p}, ty.f32(), utils::Vector{Decl(idx), Decl(x), Return(x)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto idx_sem = Sem().Get(acc)->UnwrapLoad()->As<sem::IndexAccessorExpression>();
    ASSERT_NE(idx_sem, nullptr);
    EXPECT_EQ(idx_sem->Index()->Declaration(), acc->index);
    EXPECT_EQ(idx_sem->Object()->Declaration(), acc->object);
}

TEST_F(ResolverIndexAccessorTest, Expr_Deref_FuncBadParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let idx: u32 = u32();
    //     let x: f32 = *p[idx];
    //     return x;
    // }
    auto* p = Param("p", ty.pointer(ty.vec4<f32>(), builtin::AddressSpace::kFunction));
    auto* idx = Let("idx", ty.u32(), Call<u32>());
    auto* accessor_expr = IndexAccessor(Source{{12, 34}}, p, idx);
    auto* star_p = Deref(accessor_expr);
    auto* x = Var("x", ty.f32(), star_p);
    Func("func", utils::Vector{p}, ty.f32(), utils::Vector{Decl(idx), Decl(x), Return(x)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot index type 'ptr<function, vec4<f32>, read_write>'");
}

TEST_F(ResolverIndexAccessorTest, Exr_Deref_BadParent) {
    // var param: vec4<f32>
    // let x: f32 = *(&param)[0];
    auto* param = Var("param", ty.vec4<f32>());
    auto* idx = Var("idx", ty.u32(), Call<u32>());
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
