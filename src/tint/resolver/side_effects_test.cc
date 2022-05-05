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

#include "gtest/gtest.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/member_accessor_expression.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

struct SideEffectsTest : ResolverTest {
    template <typename T>
    void MakeSideEffectFunc(const char* name) {
        auto global = Sym();
        Global(global, ty.Of<T>(), ast::StorageClass::kPrivate);
        auto local = Sym();
        Func(name, {}, ty.Of<T>(),
             {
                 Decl(Var(local, ty.Of<T>())),
                 Assign(global, local),
                 Return(global),
             });
    }

    template <typename MAKE_TYPE_FUNC>
    void MakeSideEffectFunc(const char* name, MAKE_TYPE_FUNC make_type) {
        auto global = Sym();
        Global(global, make_type(), ast::StorageClass::kPrivate);
        auto local = Sym();
        Func(name, {}, make_type(),
             {
                 Decl(Var(local, make_type())),
                 Assign(global, local),
                 Return(global),
             });
    }
};

TEST_F(SideEffectsTest, Phony) {
    auto* expr = Phony();
    auto* body = Assign(expr, 1_i);
    WrapInFunction(body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Literal) {
    auto* expr = Expr(1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, VariableUser) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Expr("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::VariableUser>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_NoSE) {
    Global("a", ty.f32(), ast::StorageClass::kPrivate);
    auto* expr = Call("dpdx", "a");
    Func("f", {}, ty.void_(), {Ignore(expr)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_NoSE_WithSEArg) {
    MakeSideEffectFunc<f32>("se");
    auto* expr = Call("dpdx", Call("se"));
    Func("f", {}, ty.void_(), {Ignore(expr)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_SE) {
    Global("a", ty.atomic(ty.i32()), ast::StorageClass::kWorkgroup);
    auto* expr = Call("atomicAdd", AddressOf("a"), 1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Function) {
    Func("f", {}, ty.i32(), {Return(1_i)});
    auto* expr = Call("f");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConversion_NoSE) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Construct(ty.f32(), "a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConversion_SE) {
    MakeSideEffectFunc<i32>("se");
    auto* expr = Construct(ty.f32(), Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConstructor_NoSE) {
    auto* var = Decl(Var("a", ty.f32()));
    auto* expr = Construct(ty.f32(), "a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConstructor_SE) {
    MakeSideEffectFunc<f32>("se");
    auto* expr = Construct(ty.f32(), Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Struct_NoSE) {
    auto* s = Structure("S", {Member("m", ty.i32())});
    auto* var = Decl(Var("a", ty.Of(s)));
    auto* expr = MemberAccessor("a", "m");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Struct_SE) {
    auto* s = Structure("S", {Member("m", ty.i32())});
    MakeSideEffectFunc("se", [&] { return ty.Of(s); });
    auto* expr = MemberAccessor(Call("se"), "m");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Vector) {
    auto* var = Decl(Var("a", ty.vec4<f32>()));
    auto* expr = MemberAccessor("a", "x");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    EXPECT_TRUE(sem->Is<sem::MemberAccessorExpression>());
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_VectorSwizzleNoSE) {
    auto* var = Decl(Var("a", ty.vec4<f32>()));
    auto* expr = MemberAccessor("a", "xzyw");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    EXPECT_TRUE(sem->Is<sem::Swizzle>());
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_VectorSwizzleSE) {
    MakeSideEffectFunc("se", [&] { return ty.vec4<f32>(); });
    auto* expr = MemberAccessor(Call("se"), "xzyw");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    EXPECT_TRUE(sem->Is<sem::Swizzle>());
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_NoSE) {
    auto* a = Decl(Var("a", ty.i32()));
    auto* b = Decl(Var("b", ty.i32()));
    auto* expr = Add("a", "b");
    WrapInFunction(a, b, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_LeftSE) {
    MakeSideEffectFunc<i32>("se");
    auto* b = Decl(Var("b", ty.i32()));
    auto* expr = Add(Call("se"), "b");
    WrapInFunction(b, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_RightSE) {
    MakeSideEffectFunc<i32>("se");
    auto* a = Decl(Var("a", ty.i32()));
    auto* expr = Add("a", Call("se"));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_BothSE) {
    MakeSideEffectFunc<i32>("se1");
    MakeSideEffectFunc<i32>("se2");
    auto* expr = Add(Call("se1"), Call("se2"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Unary_NoSE) {
    auto* var = Decl(Var("a", ty.bool_()));
    auto* expr = Not("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Unary_SE) {
    MakeSideEffectFunc<bool>("se");
    auto* expr = Not(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_NoSE) {
    auto* var = Decl(Var("a", ty.array<i32, 10>()));
    auto* expr = IndexAccessor("a", 0_i);
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_ObjSE) {
    MakeSideEffectFunc("se", [&] { return ty.array<i32, 10>(); });
    auto* expr = IndexAccessor(Call("se"), 0_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_IndexSE) {
    MakeSideEffectFunc<i32>("se");
    auto* var = Decl(Var("a", ty.array<i32, 10>()));
    auto* expr = IndexAccessor("a", Call("se"));
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_BothSE) {
    MakeSideEffectFunc("se1", [&] { return ty.array<i32, 10>(); });
    MakeSideEffectFunc<i32>("se2");
    auto* expr = IndexAccessor(Call("se1"), Call("se2"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Bitcast_NoSE) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Bitcast<f32>("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Bitcast_SE) {
    MakeSideEffectFunc<i32>("se");
    auto* expr = Bitcast<f32>(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

}  // namespace
}  // namespace tint::resolver
