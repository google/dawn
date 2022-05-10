// Copyright 2022 The Tint Authors.
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
#include "src/tint/sem/member_accessor_expression.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverSourceVariableTest : public ResolverTest {};

TEST_F(ResolverSourceVariableTest, GlobalPrivateVar) {
    auto* a = Global("a", ty.f32(), ast::StorageClass::kPrivate);
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, GlobalWorkgroupVar) {
    auto* a = Global("a", ty.f32(), ast::StorageClass::kWorkgroup);
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, GlobalStorageVar) {
    auto* a = Global("a", ty.f32(), ast::StorageClass::kStorage, GroupAndBinding(0, 0));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, GlobalUniformVar) {
    auto* a = Global("a", ty.f32(), ast::StorageClass::kUniform, GroupAndBinding(0, 0));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, GlobalTextureVar) {
    auto* a = Global("a", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
                     ast::StorageClass::kNone, GroupAndBinding(0, 0));
    auto* expr = Expr(a);
    WrapInFunction(Call("textureDimensions", expr));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, GlobalOverride) {
    auto* a = Override("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, GlobalConst) {
    auto* a = GlobalConst("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, FunctionVar) {
    auto* a = Var("a", ty.f32(), ast::StorageClass::kNone);
    auto* expr = Expr(a);
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, FunctionLet) {
    auto* a = Let("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, Parameter) {
    auto* a = Param("a", ty.f32());
    auto* expr = Expr(a);
    Func("foo", {a}, ty.void_(), {WrapInStatement(expr)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, PointerParameter) {
    // fn foo(a : ptr<function, f32>)
    // {
    //   let b = a;
    // }
    auto* param = Param("a", ty.pointer(ty.f32(), ast::StorageClass::kFunction));
    auto* expr_param = Expr(param);
    auto* let = Let("b", nullptr, expr_param);
    auto* expr_let = Expr("b");
    Func("foo", {param}, ty.void_(), {WrapInStatement(let), WrapInStatement(expr_let)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_param = Sem().Get(param);
    EXPECT_EQ(Sem().Get(expr_param)->SourceVariable(), sem_param);
    EXPECT_EQ(Sem().Get(expr_let)->SourceVariable(), sem_param);
}

TEST_F(ResolverSourceVariableTest, VarCopyVar) {
    // {
    //   var a : f32;
    //   var b = a;
    // }
    auto* a = Var("a", ty.f32(), ast::StorageClass::kNone);
    auto* expr_a = Expr(a);
    auto* b = Var("b", ty.f32(), ast::StorageClass::kNone, expr_a);
    auto* expr_b = Expr(b);
    WrapInFunction(a, b, expr_b);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    auto* sem_b = Sem().Get(b);
    EXPECT_EQ(Sem().Get(expr_a)->SourceVariable(), sem_a);
    EXPECT_EQ(Sem().Get(expr_b)->SourceVariable(), sem_b);
}

TEST_F(ResolverSourceVariableTest, LetCopyVar) {
    // {
    //   var a : f32;
    //   let b = a;
    // }
    auto* a = Var("a", ty.f32(), ast::StorageClass::kNone);
    auto* expr_a = Expr(a);
    auto* b = Let("b", ty.f32(), expr_a);
    auto* expr_b = Expr(b);
    WrapInFunction(a, b, expr_b);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    auto* sem_b = Sem().Get(b);
    EXPECT_EQ(Sem().Get(expr_a)->SourceVariable(), sem_a);
    EXPECT_EQ(Sem().Get(expr_b)->SourceVariable(), sem_b);
}

TEST_F(ResolverSourceVariableTest, ThroughIndexAccessor) {
    // var<private> a : array<f32, 4u>;
    // {
    //   a[2i]
    // }
    auto* a = Global("a", ty.array(ty.f32(), 4_u), ast::StorageClass::kPrivate);
    auto* expr = IndexAccessor(a, 2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, ThroughMemberAccessor) {
    // struct S { f : f32 }
    // var<private> a : S;
    // {
    //   a.f
    // }
    auto* S = Structure("S", {Member("f", ty.f32())});
    auto* a = Global("a", ty.Of(S), ast::StorageClass::kPrivate);
    auto* expr = MemberAccessor(a, "f");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, ThroughPointers) {
    // var<private> a : f32;
    // {
    //   let a_ptr1 = &*&a;
    //   let a_ptr2 = &*a_ptr1;
    // }
    auto* a = Global("a", ty.f32(), ast::StorageClass::kPrivate);
    auto* address_of_1 = AddressOf(a);
    auto* deref_1 = Deref(address_of_1);
    auto* address_of_2 = AddressOf(deref_1);
    auto* a_ptr1 = Let("a_ptr1", nullptr, address_of_2);
    auto* deref_2 = Deref(a_ptr1);
    auto* address_of_3 = AddressOf(deref_2);
    auto* a_ptr2 = Let("a_ptr2", nullptr, address_of_3);
    WrapInFunction(a_ptr1, a_ptr2);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(address_of_1)->SourceVariable(), sem_a);
    EXPECT_EQ(Sem().Get(address_of_2)->SourceVariable(), sem_a);
    EXPECT_EQ(Sem().Get(address_of_3)->SourceVariable(), sem_a);
    EXPECT_EQ(Sem().Get(deref_1)->SourceVariable(), sem_a);
    EXPECT_EQ(Sem().Get(deref_2)->SourceVariable(), sem_a);
}

TEST_F(ResolverSourceVariableTest, Literal) {
    auto* expr = Expr(1_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), nullptr);
}

TEST_F(ResolverSourceVariableTest, FunctionReturnValue) {
    auto* expr = Call("min", 1_f, 2_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), nullptr);
}

TEST_F(ResolverSourceVariableTest, BinaryExpression) {
    auto* a = Var("a", ty.f32(), ast::StorageClass::kNone);
    auto* expr = Add(a, Expr(1_f));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), nullptr);
}

TEST_F(ResolverSourceVariableTest, UnaryExpression) {
    auto* a = Var("a", ty.f32(), ast::StorageClass::kNone);
    auto* expr = create<ast::UnaryOpExpression>(ast::UnaryOp::kNegation, Expr(a));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->SourceVariable(), nullptr);
}

}  // namespace
}  // namespace tint::resolver
