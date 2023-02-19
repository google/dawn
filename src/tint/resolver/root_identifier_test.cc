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
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/type/texture_dimension.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverRootIdentifierTest : public ResolverTest {};

TEST_F(ResolverRootIdentifierTest, GlobalPrivateVar) {
    auto* a = GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalWorkgroupVar) {
    auto* a = GlobalVar("a", ty.f32(), builtin::AddressSpace::kWorkgroup);
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalStorageVar) {
    auto* a = GlobalVar("a", ty.f32(), builtin::AddressSpace::kStorage, Group(0_a), Binding(0_a));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalUniformVar) {
    auto* a = GlobalVar("a", ty.f32(), builtin::AddressSpace::kUniform, Group(0_a), Binding(0_a));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalTextureVar) {
    auto* a = GlobalVar("a", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
                        builtin::AddressSpace::kUndefined, Group(0_a), Binding(0_a));
    auto* expr = Expr(a);
    WrapInFunction(Call("textureDimensions", expr));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalOverride) {
    auto* a = Override("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, GlobalConst) {
    auto* a = GlobalConst("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, FunctionVar) {
    auto* a = Var("a", ty.f32());
    auto* expr = Expr(a);
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, FunctionLet) {
    auto* a = Let("a", ty.f32(), Expr(1_f));
    auto* expr = Expr(a);
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, Parameter) {
    auto* a = Param("a", ty.f32());
    auto* expr = Expr(a);
    Func("foo", utils::Vector{a}, ty.void_(), utils::Vector{WrapInStatement(expr)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().GetVal(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, PointerParameter) {
    // fn foo(a : ptr<function, f32>)
    // {
    //   let b = a;
    // }
    auto* param = Param("a", ty.pointer(ty.f32(), builtin::AddressSpace::kFunction));
    auto* expr_param = Expr(param);
    auto* let = Let("b", expr_param);
    auto* expr_let = Expr("b");
    Func("foo", utils::Vector{param}, ty.void_(),
         utils::Vector{WrapInStatement(let), WrapInStatement(expr_let)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_param = Sem().Get(param);
    EXPECT_EQ(Sem().GetVal(expr_param)->RootIdentifier(), sem_param);
    EXPECT_EQ(Sem().GetVal(expr_let)->RootIdentifier(), sem_param);
}

TEST_F(ResolverRootIdentifierTest, VarCopyVar) {
    // {
    //   var a : f32;
    //   var b = a;
    // }
    auto* a = Var("a", ty.f32());
    auto* expr_a = Expr(a);
    auto* b = Var("b", ty.f32(), expr_a);
    auto* expr_b = Expr(b);
    WrapInFunction(a, b, expr_b);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    auto* sem_b = Sem().Get(b);
    EXPECT_EQ(Sem().GetVal(expr_a)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().GetVal(expr_b)->RootIdentifier(), sem_b);
}

TEST_F(ResolverRootIdentifierTest, LetCopyVar) {
    // {
    //   var a : f32;
    //   let b = a;
    // }
    auto* a = Var("a", ty.f32());
    auto* expr_a = Expr(a);
    auto* b = Let("b", ty.f32(), expr_a);
    auto* expr_b = Expr(b);
    WrapInFunction(a, b, expr_b);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    auto* sem_b = Sem().Get(b);
    EXPECT_EQ(Sem().GetVal(expr_a)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().GetVal(expr_b)->RootIdentifier(), sem_b);
}

TEST_F(ResolverRootIdentifierTest, ThroughIndexAccessor) {
    // var<private> a : array<f32, 4u>;
    // {
    //   a[2i]
    // }
    auto* a = GlobalVar("a", ty.array<f32, 4>(), builtin::AddressSpace::kPrivate);
    auto* expr = IndexAccessor(a, 2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, ThroughMemberAccessor) {
    // struct S { f : f32 }
    // var<private> a : S;
    // {
    //   a.f
    // }
    auto* S = Structure("S", utils::Vector{Member("f", ty.f32())});
    auto* a = GlobalVar("a", ty.Of(S), builtin::AddressSpace::kPrivate);
    auto* expr = MemberAccessor(a, "f");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, ThroughPointers) {
    // var<private> a : f32;
    // {
    //   let a_ptr1 = &*&a;
    //   let a_ptr2 = &*a_ptr1;
    // }
    auto* a = GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);
    auto* address_of_1 = AddressOf(a);
    auto* deref_1 = Deref(address_of_1);
    auto* address_of_2 = AddressOf(deref_1);
    auto* a_ptr1 = Let("a_ptr1", address_of_2);
    auto* deref_2 = Deref(a_ptr1);
    auto* address_of_3 = AddressOf(deref_2);
    auto* a_ptr2 = Let("a_ptr2", address_of_3);
    WrapInFunction(a_ptr1, a_ptr2);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get(a);
    EXPECT_EQ(Sem().Get(address_of_1)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().Get(address_of_2)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().Get(address_of_3)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().Get(deref_1)->RootIdentifier(), sem_a);
    EXPECT_EQ(Sem().Get(deref_2)->RootIdentifier(), sem_a);
}

TEST_F(ResolverRootIdentifierTest, Literal) {
    auto* expr = Expr(1_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), nullptr);
}

TEST_F(ResolverRootIdentifierTest, FunctionReturnValue) {
    auto* expr = Call("min", 1_f, 2_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), nullptr);
}

TEST_F(ResolverRootIdentifierTest, BinaryExpression) {
    auto* a = Var("a", ty.f32());
    auto* expr = Add(a, Expr(1_f));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), nullptr);
}

TEST_F(ResolverRootIdentifierTest, UnaryExpression) {
    auto* a = Var("a", ty.f32());
    auto* expr = create<ast::UnaryOpExpression>(ast::UnaryOp::kNegation, Expr(a));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    EXPECT_EQ(Sem().Get(expr)->RootIdentifier(), nullptr);
}

}  // namespace
}  // namespace tint::resolver
