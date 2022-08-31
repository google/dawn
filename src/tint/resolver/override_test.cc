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
    std::vector<ast::Variable*> variables;
    auto* a = Override("a", ty.f32(), Expr(1_f));
    auto* b = Override("b", ty.f32(), Expr(1_f));
    auto* c = Override("c", ty.f32(), Expr(1_f), Id(2_u));
    auto* d = Override("d", ty.f32(), Expr(1_f), Id(4_u));
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

    EXPECT_EQ(r()->error(), R"(56:78 error: override IDs must be unique
12:34 note: a override with an ID of 7 was previously declared here:)");
}

TEST_F(ResolverOverrideTest, IdTooLarge) {
    Override("a", ty.f32(), Expr(1_f), Id(Source{{12, 34}}, 65536_u));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: override IDs must be between 0 and 65535");
}

TEST_F(ResolverOverrideTest, F16_TemporallyBan) {
    Enable(ast::Extension::kF16);

    Override(Source{{12, 34}}, "a", ty.f16(), Expr(1_h), Id(1_u));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: 'override' of type f16 is not implemented yet");
}

}  // namespace
}  // namespace tint::resolver
