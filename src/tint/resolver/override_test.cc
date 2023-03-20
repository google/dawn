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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverOverrideTest : public ResolverTest {
  protected:
    /// Verify that the AST node `var` was resolved to an overridable constant
    /// with an ID equal to `id`.
    /// @param var the overridable constant AST node
    /// @param id the expected constant ID
    void ExpectOverrideId(const ast::Variable* var, uint16_t id) {
        auto* sem = Sem().Get<sem::GlobalVariable>(var);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->Declaration(), var);
        EXPECT_TRUE(sem->Declaration()->Is<ast::Override>());
        EXPECT_EQ(sem->OverrideId().value, id);
        EXPECT_FALSE(sem->ConstantValue());
    }
};

TEST_F(ResolverOverrideTest, NonOverridable) {
    auto* a = GlobalConst("a", ty.f32(), Expr(1_f));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get<sem::GlobalVariable>(a);
    ASSERT_NE(sem_a, nullptr);
    EXPECT_EQ(sem_a->Declaration(), a);
    EXPECT_FALSE(sem_a->Declaration()->Is<ast::Override>());
    EXPECT_TRUE(sem_a->ConstantValue());
}

TEST_F(ResolverOverrideTest, WithId) {
    auto* a = Override("a", ty.f32(), Expr(1_f), Id(7_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ExpectOverrideId(a, 7u);
}

TEST_F(ResolverOverrideTest, WithoutId) {
    auto* a = Override("a", ty.f32(), Expr(1_f));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ExpectOverrideId(a, 0u);
}

TEST_F(ResolverOverrideTest, WithAndWithoutIds) {
    Enable(builtin::Extension::kF16);

    auto* a = Override("a", ty.f32(), Expr(1_f));
    auto* b = Override("b", ty.f16(), Expr(1_h));
    auto* c = Override("c", ty.i32(), Expr(1_i), Id(2_u));
    auto* d = Override("d", ty.u32(), Expr(1_u), Id(4_u));
    auto* e = Override("e", ty.f32(), Expr(1_f));
    auto* f = Override("f", ty.f32(), Expr(1_f), Id(1_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    // Verify that constant id allocation order is deterministic.
    ExpectOverrideId(a, 0u);
    ExpectOverrideId(b, 3u);
    ExpectOverrideId(c, 2u);
    ExpectOverrideId(d, 4u);
    ExpectOverrideId(e, 5u);
    ExpectOverrideId(f, 1u);
}

TEST_F(ResolverOverrideTest, DuplicateIds) {
    Override("a", ty.f32(), Expr(1_f), Id(Source{{12, 34}}, 7_u));
    Override("b", ty.f32(), Expr(1_f), Id(Source{{56, 78}}, 7_u));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(56:78 error: @id values must be unique
12:34 note: a override with an ID of 7 was previously declared here:)");
}

TEST_F(ResolverOverrideTest, IdTooLarge) {
    Override("a", ty.f32(), Expr(1_f), Id(Source{{12, 34}}, 65536_u));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: @id value must be between 0 and 65535");
}

TEST_F(ResolverOverrideTest, TransitiveReferences_DirectUse) {
    auto* a = Override("a", ty.f32());
    auto* b = Override("b", ty.f32(), Expr(1_f));
    Override("unused", ty.f32(), Expr(1_f));
    auto* func = Func("foo", utils::Empty, ty.void_(),
                      utils::Vector{
                          Assign(Phony(), "a"),
                          Assign(Phony(), "b"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
    ASSERT_EQ(refs.Length(), 2u);
    EXPECT_EQ(refs[0], Sem().Get(a));
    EXPECT_EQ(refs[1], Sem().Get(b));
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaOverrideInit) {
    auto* a = Override("a", ty.f32());
    auto* b = Override("b", ty.f32(), Mul(2_a, "a"));
    Override("unused", ty.f32(), Expr(1_f));
    auto* func = Func("foo", utils::Empty, ty.void_(),
                      utils::Vector{
                          Assign(Phony(), "b"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* r = Sem().TransitivelyReferencedOverrides(Sem().Get(b));
        ASSERT_NE(r, nullptr);
        auto& refs = *r;
        ASSERT_EQ(refs.Length(), 1u);
        EXPECT_EQ(refs[0], Sem().Get(a));
    }

    {
        auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaPrivateInit) {
    auto* a = Override("a", ty.f32());
    auto* b = GlobalVar("b", builtin::AddressSpace::kPrivate, ty.f32(), Mul(2_a, "a"));
    Override("unused", ty.f32(), Expr(1_f));
    auto* func = Func("foo", utils::Empty, ty.void_(),
                      utils::Vector{
                          Assign(Phony(), "b"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* r = Sem().TransitivelyReferencedOverrides(Sem().Get<sem::GlobalVariable>(b));
        ASSERT_NE(r, nullptr);
        auto& refs = *r;
        ASSERT_EQ(refs.Length(), 1u);
        EXPECT_EQ(refs[0], Sem().Get(a));
    }

    {
        auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaAttribute) {
    auto* a = Override("a", ty.i32());
    auto* b = Override("b", ty.i32(), Mul(2_a, "a"));
    Override("unused", ty.i32(), Expr(1_a));
    auto* func = Func("foo", utils::Empty, ty.void_(),
                      utils::Vector{
                          Assign(Phony(), "b"),
                      },
                      utils::Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(Mul(2_a, "b")),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
    ASSERT_EQ(refs.Length(), 2u);
    EXPECT_EQ(refs[0], Sem().Get(b));
    EXPECT_EQ(refs[1], Sem().Get(a));
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaArraySize) {
    auto* a = Override("a", ty.i32());
    auto* b = Override("b", ty.i32(), Mul(2_a, "a"));
    auto* arr =
        GlobalVar("arr", builtin::AddressSpace::kWorkgroup, ty.array(ty.i32(), Mul(2_a, "b")));
    auto arr_ty = arr->type;
    Override("unused", ty.i32(), Expr(1_a));
    auto* func = Func("foo", utils::Empty, ty.void_(),
                      utils::Vector{
                          Assign(IndexAccessor("arr", 0_a), 42_a),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* r = Sem().TransitivelyReferencedOverrides(TypeOf(arr_ty));
        ASSERT_NE(r, nullptr);
        auto& refs = *r;
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }

    {
        auto* r = Sem().TransitivelyReferencedOverrides(Sem().Get<sem::GlobalVariable>(arr));
        ASSERT_NE(r, nullptr);
        auto& refs = *r;
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }

    {
        auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 3u);
        EXPECT_EQ(refs[0], Sem().Get(arr));
        EXPECT_EQ(refs[1], Sem().Get(b));
        EXPECT_EQ(refs[2], Sem().Get(a));
    }
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaArraySize_Alias) {
    auto* a = Override("a", ty.i32());
    auto* b = Override("b", ty.i32(), Mul(2_a, "a"));
    Alias("arr_ty", ty.array(ty.i32(), Mul(2_a, "b")));
    auto* arr = GlobalVar("arr", builtin::AddressSpace::kWorkgroup, ty("arr_ty"));
    auto arr_ty = arr->type;
    Override("unused", ty.i32(), Expr(1_a));
    auto* func = Func("foo", utils::Empty, ty.void_(),
                      utils::Vector{
                          Assign(IndexAccessor("arr", 0_a), 42_a),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* r = Sem().TransitivelyReferencedOverrides(TypeOf(arr_ty));
        ASSERT_NE(r, nullptr);
        auto& refs = *r;
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }

    {
        auto* r = Sem().TransitivelyReferencedOverrides(Sem().Get<sem::GlobalVariable>(arr));
        ASSERT_NE(r, nullptr);
        auto& refs = *r;
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }

    {
        auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 3u);
        EXPECT_EQ(refs[0], Sem().Get(arr));
        EXPECT_EQ(refs[1], Sem().Get(b));
        EXPECT_EQ(refs[2], Sem().Get(a));
    }
}

TEST_F(ResolverOverrideTest, TransitiveReferences_MultipleEntryPoints) {
    auto* a = Override("a", ty.i32());
    auto* b1 = Override("b1", ty.i32(), Mul(2_a, "a"));
    auto* b2 = Override("b2", ty.i32(), Mul(2_a, "a"));
    auto* c1 = Override("c1", ty.i32());
    auto* c2 = Override("c2", ty.i32());
    auto* d = Override("d", ty.i32());
    Alias("arr_ty1", ty.array(ty.i32(), Mul("b1", "c1")));
    Alias("arr_ty2", ty.array(ty.i32(), Mul("b2", "c2")));
    auto* arr1 = GlobalVar("arr1", builtin::AddressSpace::kWorkgroup, ty("arr_ty1"));
    auto* arr2 = GlobalVar("arr2", builtin::AddressSpace::kWorkgroup, ty("arr_ty2"));
    Override("unused", ty.i32(), Expr(1_a));
    auto* func1 = Func("foo1", utils::Empty, ty.void_(),
                       utils::Vector{
                           Assign(IndexAccessor("arr1", 0_a), 42_a),
                       },
                       utils::Vector{
                           Stage(ast::PipelineStage::kCompute),
                           WorkgroupSize(Mul(2_a, "d")),
                       });
    auto* func2 = Func("foo2", utils::Empty, ty.void_(),
                       utils::Vector{
                           Assign(IndexAccessor("arr2", 0_a), 42_a),
                       },
                       utils::Vector{
                           Stage(ast::PipelineStage::kCompute),
                           WorkgroupSize(64_a),
                       });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto& refs = Sem().Get(func1)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 5u);
        EXPECT_EQ(refs[0], Sem().Get(d));
        EXPECT_EQ(refs[1], Sem().Get(arr1));
        EXPECT_EQ(refs[2], Sem().Get(b1));
        EXPECT_EQ(refs[3], Sem().Get(a));
        EXPECT_EQ(refs[4], Sem().Get(c1));
    }

    {
        auto& refs = Sem().Get(func2)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 4u);
        EXPECT_EQ(refs[0], Sem().Get(arr2));
        EXPECT_EQ(refs[1], Sem().Get(b2));
        EXPECT_EQ(refs[2], Sem().Get(a));
        EXPECT_EQ(refs[3], Sem().Get(c2));
    }
}

}  // namespace
}  // namespace tint::resolver
